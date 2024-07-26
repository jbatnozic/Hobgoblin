#pragma once

#include "Chunk_spooler_default.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Utility/Monitor.hpp>

#include <algorithm>

// TODO: SetThreadName

namespace gridworld {
namespace detail {

namespace {
constexpr auto LOG_ID = "gridworld";

std::optional<Chunk> LoadChunk(ChunkId aChunkId, ChunkDiskIoHandlerInterface& aDiskIoHandler) {
    auto rtChunk = aDiskIoHandler.loadChunkFromRuntimeCache(aChunkId);
    if (rtChunk.has_value()) {
        return rtChunk;
    }
    return aDiskIoHandler.loadChunkFromPersistentCache(
        aChunkId); // TODO: store in runtime cache - noooooooo
}

void UnloadChunk(const Chunk& aChunk, ChunkId aChunkId, ChunkDiskIoHandlerInterface& aDiskIoHandler) {
    aDiskIoHandler.storeChunkInRuntimeCache(aChunk, aChunkId);
}
} // namespace

class RequestHandleImpl : public DefaultChunkSpooler::RequestHandleInterface {
public:
    RequestHandleImpl(DefaultChunkSpooler& aSpooler, ChunkId aChunkId)
        : _spooler{aSpooler}
        , _chunkId{aChunkId} {}

    ~RequestHandleImpl() override = default;

    void cancel() override {
        _spooler._cancelLoadRequest(_chunkId);
    }

    ChunkId getChunkId() const override {
        return _chunkId;
    }

    bool isFinished() const override {
        return _result.Do([](const auto& aIt) {
            return aIt.isFinished;
        });
    }

    std::optional<Chunk> takeChunk() override {
        std::optional<Chunk> chunk;
        _result.Do([&](auto& aIt) {
            if (!aIt.isFinished) {
                HG_THROW_TRACED(hg::TracedLogicError,
                                0,
                                "takeChunk() called but request was not finished.");
            }
            chunk = std::move(aIt.chunk);
        });
        return chunk;
    }

    void giveResult(std::optional<Chunk> aChunkOpt) {
        _result.Do([&](auto& aIt) {
            aIt.chunk      = std::move(aChunkOpt);
            aIt.isFinished = true;
        });
    }

private:
    DefaultChunkSpooler& _spooler;
    ChunkId              _chunkId;

    struct Result {
        std::optional<Chunk> chunk;
        bool                 isFinished = false;
    };
    hg::util::Monitor<Result> _result;
};

#define HOLDS_LOAD_REQUEST(_variant_)   std::holds_alternative<LoadRequest>(_variant_)
#define HOLDS_UNLOAD_REQUEST(_variant_) std::holds_alternative<UnloadRequest>(_variant_)

DefaultChunkSpooler::DefaultChunkSpooler(ChunkDiskIoHandlerInterface& aDiskIoHandler)
    : _diskIoHandler{&aDiskIoHandler}
    , _worker{&DefaultChunkSpooler::_workerBody, this} {
    // SetThreadName(_worker, "chunkspool");
    HG_LOG_INFO(LOG_ID, "Spooler started.");
}

DefaultChunkSpooler::~DefaultChunkSpooler() {
    HG_LOG_INFO(LOG_ID, "Stopping Spooler...");

    {
        std::unique_lock<Mutex> lock{_mutex};
        _stopped = true;
    }
    _cv_workerSync.notify_one();
    _worker.join();

    HG_LOG_INFO(LOG_ID, "Spooler stopped.");
}

void DefaultChunkSpooler::pause() {
    std::unique_lock<Mutex> lock{_mutex};
    _paused = true;
}

void DefaultChunkSpooler::unpause() {
    std::unique_lock<Mutex> lock{_mutex};

    _paused = false;
    lock.unlock();
    _cv_workerSync.notify_all();
}

bool DefaultChunkSpooler::isPaused() const {
    std::unique_lock<Mutex> lock{_mutex};

    const auto result = _paused;
    lock.unlock();
    return result;
}

std::vector<std::shared_ptr<DefaultChunkSpooler::RequestHandleInterface>> DefaultChunkSpooler::
    loadChunks(const std::vector<LoadRequest>& aLoadRequests) {
    if (aLoadRequests.empty()) {
        return {};
    }

    std::unique_lock<Mutex> lock{_mutex};

    std::vector<std::shared_ptr<RequestHandleInterface>> handles;
    for (auto request : aLoadRequests) {
        handles.push_back(_onNewLoadRequest(request, lock));
    }

    lock.unlock();
    _cv_workerSync.notify_one();

    return handles;
}

std::optional<Chunk> DefaultChunkSpooler::loadImmediately(ChunkId aChunkId) {
#if 0
    std::unique_lock<Mutex> lock{_mutex};

    HG_VALIDATE_PRECONDITION(_paused && "Spooler must be paused when this method is called");

    // Erase from _loadRequests if it's present there
    std::erase_if(_loadRequests, [aChunkId](const LoadRequest& aLoadRequest) {
        return aLoadRequest.chunkId == aChunkId;
    });

    // Easy out: Try to get from _loadedChunks
    {
        const auto iter = std::find_if(_loadedChunks.begin(),
                                       _loadedChunks.end(),
                                       [aChunkId](LoadedChunk& aLoadedChunk) {
                                           return aLoadedChunk.id == aChunkId;
                                       });

        if (iter != _loadedChunks.end()) {
            std::optional<Chunk> result = std::move(iter->chunk);
            _loadedChunks.erase(iter);
            return result;
        }
    }

    // Easy out: Try to get from _unloadRequests
    {
        const auto iter = _unloadRequests.find(aChunkId);

        if (iter != _unloadRequests.end()) {
            std::optional<Chunk> result = std::move(iter->second);
            _unloadRequests.erase(iter);
            return result;
        }
    }

    // Most likely: load from disk
    return LoadChunk(aChunkId, *_diskIoHandler);
#endif
    return {};
}

hg::PZInteger DefaultChunkSpooler::unloadChunk(ChunkId aChunkId, Chunk&& aChunk) {
    std::unique_lock<Mutex> lock{_mutex};

    HG_VALIDATE_PRECONDITION(_paused && "Spooler must be paused when this method is called");

    const auto iter = _requests.find(aChunkId);
    if (iter != _requests.end()) {
        auto& existingRequest = iter->second.request;
        if (HOLDS_LOAD_REQUEST(existingRequest)) {
            HG_NOT_IMPLEMENTED("~~~~~");
        } else if (HOLDS_UNLOAD_REQUEST(existingRequest)) {
            HG_NOT_IMPLEMENTED("Handling of duplicate load requests not implemented.");
        } else {
            HG_UNREACHABLE("Invalid request found in request map for chunk {}, {}.",
                           aChunkId.x,
                           aChunkId.y);
        }
    }

    auto& cb   = _requests[aChunkId];
    cb.request = UnloadRequest{std::move(aChunk), aChunkId};
    cb.handle  = nullptr;
    _unloadRequestCount += 1;

    const auto result = _unloadRequestCount;

    lock.unlock();
    _cv_workerSync.notify_one();

    return result;
}

void DefaultChunkSpooler::unloadRuntimeCache() {
    std::unique_lock<Mutex> lock{_mutex};

    HG_VALIDATE_PRECONDITION(_paused && "Spooler must be paused when this method is called");

    HG_NOT_IMPLEMENTED(); // TODO
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS                                                       //
///////////////////////////////////////////////////////////////////////////

void DefaultChunkSpooler::_workerBody() {
    while (true) {
        RequestControlBlock cb;

        /* SYNCHRONIZATION */
        {
            std::unique_lock<Mutex> lock{_mutex};
            while (true) {
                if (_stopped) {
                    return; // Spooler is being destroyed
                }
                if (!_paused && _countAvailableRequests(lock) > 0) {
                    break; // Ready to work
                }
                _cv_workerSync.wait(lock);
            }

            const auto requestIter = _findRequestWithBestPriority(lock);
            cb = _eraseRequest(requestIter, lock);
            _adjustUnloadPriority(cb.request, lock);
        }

        /* WORK */
        auto& requestVariant = cb.request;
        HG_ASSERT(!std::holds_alternative<std::monostate>(requestVariant));

        if (HOLDS_LOAD_REQUEST(requestVariant)) {
            const auto& loadRequest = std::get<LoadRequest>(requestVariant);
            auto chunk = LoadChunk(loadRequest.chunkId, *_diskIoHandler);
            if (cb.handle) {
                cb.handle->giveResult(std::move(chunk));
            }
        } else /*if (HOLDS_UNLOAD_REQUEST(requestVariant))*/ {
            const auto& unloadRequest = std::get<UnloadRequest>(requestVariant);
            UnloadChunk(unloadRequest.chunk, unloadRequest.id, *_diskIoHandler);
        }
    }
}

std::shared_ptr<DefaultChunkSpooler::RequestHandleInterface> DefaultChunkSpooler::_onNewLoadRequest(
    LoadRequest aLoadRequest,
    const std::unique_lock<Mutex>&) {
    auto handle = std::make_shared<RequestHandleImpl>(*this, aLoadRequest.chunkId);

    const auto iter = _requests.find(aLoadRequest.chunkId);
    if (iter != _requests.end()) {
        auto& existingRequest = iter->second.request;
        if (HOLDS_LOAD_REQUEST(existingRequest)) {
            HG_NOT_IMPLEMENTED("Handling of duplicate load requests not implemented.");
        } else if (HOLDS_UNLOAD_REQUEST(existingRequest)) {
            handle->giveResult(std::move(std::get<UnloadRequest>(existingRequest).chunk));
            _requests.erase(iter);
            _unloadRequestCount -= 1;
        } else {
            HG_UNREACHABLE("Invalid request found in request map for chunk {}, {}.",
                           aLoadRequest.chunkId.x,
                           aLoadRequest.chunkId.y);
        }
        return handle;
    }

    auto& cb   = _requests[aLoadRequest.chunkId];
    cb.request = aLoadRequest;
    cb.handle  = handle;
    _loadRequestCount += 1;

    return handle;
}

hg::PZInteger DefaultChunkSpooler::_countAvailableRequests(const std::unique_lock<Mutex>&) const {
    HG_ASSERT(hg::stopz(_requests.size()) == _loadRequestCount + _unloadRequestCount);
    return hg::stopz(_requests.size());
}

DefaultChunkSpooler::RequestIter DefaultChunkSpooler::_findLoadRequestWithBestPriority(
    const std::unique_lock<Mutex>&) {
    auto iterToLoadRequestWithBestPriority = _requests.end();
    for (auto iter = _requests.begin(); iter != _requests.end(); iter = std::next(iter)) {
        if (HOLDS_LOAD_REQUEST(iter->second.request)) {
            if (iterToLoadRequestWithBestPriority == _requests.end()) {
                iterToLoadRequestWithBestPriority = iter;
            } else {
                const LoadRequest curr = std::get<LoadRequest>(iter->second.request);
                const LoadRequest best =
                    std::get<LoadRequest>(iterToLoadRequestWithBestPriority->second.request);
                if (curr.priority < best.priority) {
                    iterToLoadRequestWithBestPriority = iter;
                }
            }
        }
    }
    HG_ASSERT(iterToLoadRequestWithBestPriority != _requests.end());
    return iterToLoadRequestWithBestPriority;
}

DefaultChunkSpooler::RequestIter DefaultChunkSpooler::_findAnyUnloadRequest(
    const std::unique_lock<Mutex>&) {
    for (auto iter = _requests.begin(); iter != _requests.end(); iter = std::next(iter)) {
        if (HOLDS_UNLOAD_REQUEST(iter->second.request)) {
            return iter;
        }
    }
    HG_UNREACHABLE("No unload request could be found.");
}

DefaultChunkSpooler::RequestIter DefaultChunkSpooler::_findRequestWithBestPriority(
    const std::unique_lock<Mutex>& aLock) {
    HG_ASSERT(!_requests.empty());

    if (_unloadRequestCount == 0) {
        return _findLoadRequestWithBestPriority(aLock);
    }

    if (_loadRequestCount == 0) {
        return _findAnyUnloadRequest(aLock);
    }

    const auto iterToLoadRequestWithBestPriority = _findLoadRequestWithBestPriority(aLock);
    const auto loadRequestWithBestPriority =
        std::get<LoadRequest>(iterToLoadRequestWithBestPriority->second.request);

    if (loadRequestWithBestPriority.priority <= _unloadPriority) {
        return iterToLoadRequestWithBestPriority;
    } else {
        return _findAnyUnloadRequest(aLock);
    }
}

DefaultChunkSpooler::RequestControlBlock DefaultChunkSpooler::_eraseRequest(
    RequestIter aRequestIter,
    const std::unique_lock<Mutex>&) {
    RequestControlBlock cb = std::move(aRequestIter->second);
    _requests.erase(aRequestIter);
    if (HOLDS_LOAD_REQUEST(cb.request)) {
        _loadRequestCount -= 1;
    } else if (HOLDS_UNLOAD_REQUEST(cb.request)) {
        _unloadRequestCount -= 1;
    } else {
        HG_UNREACHABLE("Invalid request found in request map.");
    }
    return cb;
}

void DefaultChunkSpooler::_cancelLoadRequest(ChunkId aChunkId) {
    std::unique_lock<Mutex> lock{_mutex};

    const auto iter = _requests.find(aChunkId);
    if (iter != _requests.end()) {
        _eraseRequest(iter, lock);
    }
}

void DefaultChunkSpooler::_adjustUnloadPriority(const RequestVariant& aRequestVariant,
                                                const std::unique_lock<Mutex>&) {
    HG_ASSERT(!std::holds_alternative<std::monostate>(aRequestVariant));

    // Simple case: when there are no unload requests, return the priority to default
    if (_unloadRequestCount == 0) {
        _unloadPriority = UNLOAD_REQUEST_DEFAULT_PRIORITY;
        return;
    }

    // There are too many unload requests piled up: assuming there are no negative
    // priorities for load requests (the expected case), this will shed at least
    // [UNLOAD_REQUEST_THRESHOLD / LOADS_PER_UNLOAD] unload requests before
    // continuing with any loads.
    if (_unloadRequestCount >= UNLOAD_REQUEST_THRESHOLD) {
        _unloadPriority = -UNLOAD_REQUEST_THRESHOLD;
        HG_LOG_WARN(
            LOG_ID,
            "There is an overwhelming amount of unload requests ({}), unload priority set to {}.",
            _unloadRequestCount,
            _unloadPriority);
        return;
    }

    if (HOLDS_LOAD_REQUEST(aRequestVariant)) {
        _unloadPriority -= 1;
    } else {
        _unloadPriority += LOADS_PER_UNLOAD;
    }
}

} // namespace detail
} // namespace gridworld
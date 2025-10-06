// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common.hpp>

#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_id.hpp>
#include <GridGoblin/Private/Chunk_impl.hpp>
#include <GridGoblin/Private/Chunk_spooler_interface.hpp>
#include <GridGoblin/World/Active_area.hpp>
#include <GridGoblin/World/Binder.hpp>
#include <GridGoblin/World/World_config.hpp>

#include <Hobgoblin/Utility/Grids.hpp>
#include <Hobgoblin/Utility/Value_sorted_map.hpp>

#include <chrono>
#include <memory>
#include <unordered_map>

namespace jbatnozic {
namespace gridgoblin {

class Binder;

namespace detail {

namespace hg = jbatnozic::hobgoblin;

HG_DECLARE_TAG_TYPE(LOAD_IF_MISSING);

//! This class handles the storage of chunks both in RAM and in on-disk caches.
class ChunkHolder {
private:
    using Self = ChunkHolder;

public:
    ChunkHolder(const WorldConfig& aConfig);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: DEPENDENCIES                                                    //
    ///////////////////////////////////////////////////////////////////////////

    void setChunkSpooler(ChunkSpoolerInterface* aChunkSpooler);

    void setBinder(Binder* aBinder);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: ACTIVE AREAS                                                    //
    ///////////////////////////////////////////////////////////////////////////

    ActiveArea createNewActiveArea();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CYCLE                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! Collects all chunks that were loaded since the last call to `update()`
    //! and makes them available.
    void update();

    void prune();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CHUNK GETTERS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! Returns the number of chunks along the X axis.
    hg::PZInteger getChunkCountX() const {
        return _chunks.getWidth();
    }

    //! Returns the number of chunks along the Y axis.
    hg::PZInteger getChunkCountY() const {
        return _chunks.getHeight();
    }

    const auto& GetChunkMemoryLayoutInfo() const {
        return _chunkMemoryLayoutInfo;
    }

    //! Returns a mutable reference to the chunk with the given ID WITHOUT CHECKING BOUNDS.
    //! The chunk will be loaded if it isn't already, and will remain loaded until the next
    //! call to `prune()`.
    Chunk& getChunkAtIdUnchecked(ChunkId aChunkId, LOAD_IF_MISSING_Tag) {
        auto& chunk =
            _chunks[static_cast<hg::PZInteger>(aChunkId.y)][static_cast<hg::PZInteger>(aChunkId.x)];
        if (HG_LIKELY_CONDITION(!chunk.isEmpty())) {
            HG_LIKELY_BRANCH;
            return chunk;
        } else {
            HG_UNLIKELY_BRANCH;
            _loadChunkImmediately(aChunkId);
            return chunk;
        }
    }

    //! Returns a const reference to the chunk with the given ID WITHOUT CHECKING BOUNDS.
    //! The chunk will be loaded if it isn't already, and will remain loaded until the next
    //! call to `prune()`.
    const Chunk& getChunkAtIdUnchecked(ChunkId aChunkId, LOAD_IF_MISSING_Tag) const {
        return const_cast<Self*>(this)->getChunkAtIdUnchecked(aChunkId, LOAD_IF_MISSING);
    }

    //! Returns a mutable pointer to the chunk with the given ID WITHOUT CHECKING BOUNDS.
    //! If the chunk is already loaded, it will remain loaded until the next call
    //! to `prune()`. Otherwise, `nullptr` will be returned.
    Chunk* getChunkAtIdUnchecked(ChunkId aChunkId) {
        auto& chunk =
            _chunks[static_cast<hg::PZInteger>(aChunkId.y)][static_cast<hg::PZInteger>(aChunkId.x)];
        if (HG_LIKELY_CONDITION(!chunk.isEmpty())) {
            HG_LIKELY_BRANCH;
            return &chunk;
        } else {
            HG_UNLIKELY_BRANCH;
            return nullptr;
        }
    }

    //! Returns a const pointer to the chunk with the given ID WITHOUT CHECKING BOUNDS.
    //! If the chunk is already loaded, it will remain loaded until the next call
    //! to `prune()`. Otherwise, `nullptr` will be returned.
    const Chunk* getChunkAtIdUnchecked(ChunkId aChunkId) const {
        return const_cast<Self*>(this)->getChunkAtIdUnchecked(aChunkId);
    }

private:
    friend class ::jbatnozic::gridgoblin::ActiveArea;

    // ===== Binder

    Binder* _binder = nullptr;

    // ===== Chunk Grid

    detail::ChunkImpl::MemoryLayoutInfo _chunkMemoryLayoutInfo;

    mutable hg::util::RowMajorGrid<Chunk> _chunks;

    hg::PZInteger _chunksInGridCount = 0;

    // ===== Active Chunks

    struct ChunkControlBlock {
        std::shared_ptr<ChunkSpoolerInterface::RequestHandleInterface> requestHandle = nullptr;
        hg::PZInteger                                                  usageCount    = 0;

        bool isChunkLoaded() const {
            return requestHandle == nullptr;
        }
    };

    mutable std::unordered_map<ChunkId, ChunkControlBlock> _chunkControlBlocks;

    // ===== Free Chunks

    using Timestamp = std::chrono::steady_clock::time_point;

    //! This collection maps ChunkId -> Timestamp of the chunk becoming free.
    //! It is by default sorted in ascending order, meaning that the lowest (least recent)
    //! timestamps will be in the front, thus the chunks in the front are the best ones to evict.
    mutable hg::util::ValueSortedMap<ChunkId, Timestamp> _freeChunks;

    // ===== Members

    ChunkSpoolerInterface* _chunkSpooler = nullptr;

    hg::PZInteger _chunkWidth;
    hg::PZInteger _chunkHeight;

    hg::PZInteger _freeChunkLimit;

    // ===== Methods

    void _loadChunkImmediately(ChunkId aChunkId);

    void _onChunkLoaded(ChunkId aChunkId, Chunk&& aChunk);
    void _createDefaultChunk(ChunkId aChunkId);

    void _updateChunkUsage(const std::vector<detail::ChunkUsageChange>& aChunkUsageChanges);

public:
    ///////////////////////////////////////////////////////////////////////////
    // MARK: ITERATOR                                                        //
    ///////////////////////////////////////////////////////////////////////////

    class AvailableChunkIterator {
    public:
        void advance();

        ChunkId dereference() const;

        bool equals(const AvailableChunkIterator& aOther) const;

    private:
        friend class ChunkHolder;

        AvailableChunkIterator(const decltype(_chunkControlBlocks)& aCbMap,
                               const decltype(_freeChunks)&         aFcMap,
                               bool                                 aIsEndIter);

        const decltype(_chunkControlBlocks)&          _cbMap;
        decltype(_chunkControlBlocks)::const_iterator _cbIter;

        const decltype(_freeChunks)&          _fcMap;
        decltype(_freeChunks)::const_iterator _fcIter;

        static constexpr char SELECTOR_CB = 'c';
        static constexpr char SELECTOR_FC = 'f';

        char _selector;
        bool _isEndIter;
    };

    AvailableChunkIterator availableChunksBegin() const {
        return {_chunkControlBlocks, _freeChunks, false};
    }

    AvailableChunkIterator availableChunksEnd() const {
        return {_chunkControlBlocks, _freeChunks, true};
    }
};

} // namespace detail

} // namespace gridgoblin
} // namespace jbatnozic

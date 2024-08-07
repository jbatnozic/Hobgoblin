// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Private/Flow_field_spooler_impl.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/Utility/Flow_field_calculator.hpp>
#include <Hobgoblin/Utility/Latch.hpp>
#include <Hobgoblin/Utility/Monitor.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {
namespace detail {

namespace {

constexpr auto LOG_ID = "Hobgoblin.Utility";

class OffsetCostProvider {
public:
    OffsetCostProvider(WorldCostFunctionWithArg aWcfWithArg, math::Vector2pz aOffset)
        : _wcfWithArg{aWcfWithArg}
        , _offset{aOffset} {}

    std::uint8_t getCostAt(math::Vector2pz aPosition) const {
        return _wcfWithArg.func(aPosition + _offset, _wcfWithArg.arg);
    }

private:
    WorldCostFunctionWithArg _wcfWithArg;
    math::Vector2pz          _offset = {0, 0};
};

// clang-format off
struct Station {
    Station()
        : costProvider{{nullptr, nullptr}, {0, 0}} {}

    // Prevent copying and moving because `Job` has to be
    // able to hold a pointer to an instance of `Station.
    Station(const Station&) = delete;
    Station& operator=(const Station&) = delete;
    Station(Station&&) = delete;
    Station& operator=(Station&&) = delete;

    ///////////////////////////////////////

    OffsetCostProvider costProvider;
    FlowFieldCalculator<OffsetCostProvider> flowFieldCalculator;
    std::atomic_int remainingJobCount{0};
    bool isOccupied = false;
};
// clang-format on

struct Request {
    Request(std::uint64_t aRequestId, std::int32_t aCostProviderId)
        : id{aRequestId}
        , costProviderId{aCostProviderId} {}

    ~Request() {
        HG_LOG_DEBUG(LOG_ID, "Flow Field Request (id: {}) destroyed.", id);
    }

    // Prevent copying and moving because `Job` has to be
    // able to hold a pointer to an instance of `Request.
    Request(const Request&)            = delete;
    Request& operator=(const Request&) = delete;
    Request(Request&&)                 = delete;
    Request& operator=(Request&&)      = delete;

    ///////////////////////////////////////

    std::uint64_t id;
    std::int32_t  costProviderId;

    math::Vector2pz fieldTopLeft{};
    int             remainingIterations{0};

    std::optional<FlowField> result{};

    Latch latch{Latch::CLOSED};

    std::atomic_bool cancelled{false};
};

struct Job {
    Job() {}

    enum Kind {
        CALCULATE_INTEGRATION_FIELD,
        CALCULATE_FLOW_FIELD_PART
    };

    Kind kind;

    struct CalculateIntegrationFieldData {
        math::Vector2pz fieldTopLeft;
        math::Vector2pz fieldDimensions;
        math::Vector2pz target;
    };

    struct CalculateFlowFieldPartData {
        NeverNull<Station*> station;
        PZInteger           preferredWorkerId;
        PZInteger           startingRow;
        PZInteger           rowCount;
    };

    union {
        //! Used only when kind==CALCULATE_INTEGRATION_FIELD.
        CalculateIntegrationFieldData calcIntegrationFieldData;

        //! Used only when kind==CALCULATE_FLOW_FIELD_PART.
        CalculateFlowFieldPartData calcFlowFieldPartData;
    };

    //! Original request that this job relates to.
    std::shared_ptr<Request> request = nullptr;
};

Job CreateCalcIntegrationFieldJob(std::shared_ptr<Request> aRequest,
                                  math::Vector2pz          aFieldTopLeft,
                                  math::Vector2pz          aFieldDimensions,
                                  math::Vector2pz          aTarget) {
    Job job;
    job.kind    = Job::CALCULATE_INTEGRATION_FIELD;
    job.request = std::move(aRequest);

    job.calcIntegrationFieldData.fieldTopLeft    = aFieldTopLeft;
    job.calcIntegrationFieldData.fieldDimensions = aFieldDimensions;
    job.calcIntegrationFieldData.target          = aTarget;

    return job;
}

Job CreateCalcFlowFieldPartJob(std::shared_ptr<Request> aRequest,
                               NeverNull<Station*>      aStation,
                               PZInteger                aPreferredWorkerId,
                               PZInteger                aStartingRow,
                               PZInteger                aRowCount) {
    Job job;
    job.kind    = Job::CALCULATE_FLOW_FIELD_PART;
    job.request = std::move(aRequest);

    job.calcFlowFieldPartData.station           = aStation;
    job.calcFlowFieldPartData.preferredWorkerId = aPreferredWorkerId;
    job.calcFlowFieldPartData.startingRow       = aStartingRow;
    job.calcFlowFieldPartData.rowCount          = aRowCount;

    return job;
}

class FlowFieldSpoolerImpl : public FlowFieldSpoolerImplInterface {
public:
    FlowFieldSpoolerImpl(WCFMap    aWcfMap,
                         PZInteger aConcurrencyLimit,
                         PZInteger aFinishedRequestExpirationLimit)
        : _wcfMap{std::move(aWcfMap)}
        , _concurrencyLimit{aConcurrencyLimit}
        , _finishedRequestExpirationLimit{aFinishedRequestExpirationLimit} {
        HG_VALIDATE_ARGUMENT(!_wcfMap.empty());
        HG_VALIDATE_ARGUMENT(aConcurrencyLimit > 0);

        HG_LOG_INFO(LOG_ID, "Creating Spooler with aConcurrencyLimit={}...", aConcurrencyLimit);

        // Create stations
        _stations.Do([=](StationList& aIt) {
            for (PZInteger i = 0; i < aConcurrencyLimit; i += 1) {
                aIt.emplace_back();
            }
        });

        // Create & start worker threads
        _workerStatuses.resize(pztos(aConcurrencyLimit));
        _workers.reserve(pztos(aConcurrencyLimit));
        for (PZInteger i = 0; i < aConcurrencyLimit; i += 1) {
            _workerStatuses[pztos(i)] = WorkerStatus::PREP_OR_IDLE;
            _workers.emplace_back(&FlowFieldSpoolerImpl::_workerBody, this, i);
        }
    }

    ~FlowFieldSpoolerImpl() override {
        HG_LOG_INFO(LOG_ID, "Stopping Spooler...");

        {
            std::unique_lock<Mutex> lock{_mutex};
            _stopped = true;
        }
        _cv.notify_all();

        for (auto& worker : _workers) {
            worker.join();
        }
    }

    void tick() override;

    void pause() override;

    void unpause() override;

    std::uint64_t addRequest(math::Vector2pz aFieldTopLeft,
                             math::Vector2pz aFieldDimensions,
                             math::Vector2pz aTarget,
                             std::int32_t    aCostProviderId,
                             PZInteger       aMaxIterations) override;

    void cancelRequest(std::uint64_t aRequestId) override;

    std::optional<OffsetFlowField> collectResult(std::uint64_t aRequestId) override;

private:
    std::unordered_map<std::int32_t, WorldCostFunctionWithArg> _wcfMap;
    PZInteger                                                  _concurrencyLimit;

    ///////////////////////////////////////

    using StationList = std::list<Station>;
    using RequestMap  = std::unordered_map<std::uint64_t, std::shared_ptr<Request>>;

    Monitor<StationList> _stations;
    Monitor<RequestMap>  _requests;

    std::atomic_uint64_t _requestIdCounter{0};
    int                  _finishedRequestExpirationLimit;

    ///////////////////////////////////////

    using Mutex = std::mutex;
    Mutex                   _mutex; // Protects: _jobs, _paused, _stopped, _workerStatuses
    std::condition_variable _cv;

    std::vector<Job> _jobs;

    bool _paused{false};
    bool _stopped{false};

    enum class WorkerStatus : std::int8_t {
        PREP_OR_IDLE, //!< Preparing to work or idling
        WORKING       //!< Working
    };

    std::vector<std::thread>  _workers;
    std::vector<WorkerStatus> _workerStatuses;
    std::condition_variable   _cv_workerStatuses;

    ///////////////////////////////////////

    void _setWorkerStatus(PZInteger aWorkerId, WorkerStatus aStatus, const std::unique_lock<Mutex>&) {
        _workerStatuses[pztos(aWorkerId)] = aStatus;
        _cv_workerStatuses.notify_all();
    }

    void _addJob(const Job& aNewJob, const std::unique_lock<Mutex>&) {
        _jobs.emplace_back(aNewJob);
        _cv.notify_all(); // If a worker was blocked waiting for a job, unblock it
    }

    void _addJobs(const std::vector<Job>& aNewJobs, const std::unique_lock<Mutex>&) {
        for (const auto& job : aNewJobs) {
            _jobs.emplace_back(job);
        }
        _cv.notify_all(); // If a worker(s) was blocked waiting for a job, unblock it(them)
    }

    PZInteger _countAvailableJobs(const std::unique_lock<Mutex>&) const {
        return stopz(_jobs.size());
    }

    static std::int32_t _calcJobScore(const Job& aJob, PZInteger aWorkerId) {
        // PRIORITIES (Highest to lowest)
        // 1. CALCULATE_FLOW_FIELD_PART job preferred by this thread
        // 2. CALCULATE_INTEGRATION_FIELD job (any)
        // 3. CALCULATE_FLOW_FIELD_PART job of another thread - when there is nothing better to do
        //
        // A sufficiently urgent (3) can have a higher priority than (2)

        if (aJob.kind == Job::CALCULATE_FLOW_FIELD_PART &&
            aJob.calcFlowFieldPartData.preferredWorkerId == aWorkerId) {
            return std::numeric_limits<std::int32_t>::max();
        }

        std::int32_t score = 0;
        if (aJob.kind == Job::CALCULATE_INTEGRATION_FIELD) {
            score += 1999;
        }
        score -= (aJob.request->remainingIterations * 1000);

        return score;
    }

    Job _takeJob(PZInteger aWorkerId, const std::unique_lock<Mutex>&) {
        HG_ASSERT(!_jobs.empty());

        std::size_t  pickedJobIdx   = 0;
        std::int32_t pickedJobScore = _calcJobScore(_jobs[0], aWorkerId);
        for (std::size_t i = 1; i < _jobs.size(); i += 1) {
            const auto& current = _jobs[i];
            const auto  score   = _calcJobScore(current, aWorkerId);
            if (score > pickedJobScore) {
                pickedJobIdx   = i;
                pickedJobScore = score;
            }
        }

        const Job result = _jobs[pickedJobIdx];
        std::swap(_jobs[pickedJobIdx], _jobs[_jobs.size() - 1]);
        _jobs.pop_back();
        return result;
    }

    //! Assigns an unoccupied Station to a Job.
    NeverNull<Station*> _assignStation(Station& aStation, Job& aJob) {
        HG_HARD_ASSERT(!aStation.isOccupied);

        auto wcfIter = _wcfMap.find(aJob.request->costProviderId);
        HG_HARD_ASSERT(wcfIter != _wcfMap.end());

        const auto& jobData   = aJob.calcIntegrationFieldData;
        aStation.costProvider = OffsetCostProvider{wcfIter->second, jobData.fieldTopLeft};
        aStation.flowFieldCalculator.reset(jobData.fieldDimensions,
                                           jobData.target,
                                           aStation.costProvider);
        aStation.isOccupied = true;
        return &aStation;
    }

    void _workerBody(PZInteger aWorkerId) {
        while (true) {
            Job job;

            /* SYNCHRONIZATION */
            {
                std::unique_lock<Mutex> lock{_mutex};
                _setWorkerStatus(aWorkerId, WorkerStatus::PREP_OR_IDLE, lock);
                while (true) {
                    if (_stopped) {
                        return; // Spooler is being destroyed
                    }
                    if (!_paused && _countAvailableJobs(lock) > 0) {
                        break; // Ready to work
                    }
                    _cv.wait(lock);
                }
                _setWorkerStatus(aWorkerId, WorkerStatus::WORKING, lock);

                job = _takeJob(aWorkerId, lock);
                lock.unlock();
            }

            const auto requestId = job.request->id;

            /* WORK */
            switch (job.kind) {
            case Job::CALCULATE_INTEGRATION_FIELD:
                {
                    HG_LOG_DEBUG(
                        LOG_ID,
                        "Worker {} starting a CALCULATE_INTEGRATION_FIELD job for request {}...",
                        aWorkerId,
                        requestId);
                    Station*   station          = _workCalculateIntegrationFieldJob(job, aWorkerId);
                    const bool requestCancelled = job.request->cancelled.load();
                    HG_LOG_DEBUG(
                        LOG_ID,
                        "Worker {} finished a CALCULATE_INTEGRATION_FIELD job for request {}{}.",
                        aWorkerId,
                        requestId,
                        requestCancelled ? " (cancelled)" : "");
                    if (!requestCancelled) {
                        _finalizeCalculateIntegrationFieldJob(job, aWorkerId, station);
                    } else {
                        station->isOccupied = false;
                    }
                }
                break;

            case Job::CALCULATE_FLOW_FIELD_PART:
                {
                    const auto startRow = job.calcFlowFieldPartData.startingRow;
                    const auto endRow   = startRow + job.calcFlowFieldPartData.rowCount - 1;
                    HG_LOG_DEBUG(
                        LOG_ID,
                        "Worker {} starting a CALCULATE_FLOW_FIELD_PART ({}-{}) job for request {}...",
                        aWorkerId,
                        startRow,
                        endRow,
                        requestId);
                    _workCalculateFlowFieldPartJob(job, aWorkerId);
                    const bool requestCancelled = job.request->cancelled.load();
                    HG_LOG_DEBUG(
                        LOG_ID,
                        "Worker {} finished a CALCULATE_FLOW_FIELD_PART ({}-{}) job for request {}{}.",
                        aWorkerId,
                        startRow,
                        endRow,
                        requestId,
                        requestCancelled ? " (cancelled)" : "");
                    // Finalize even if cancelled
                    _finalizeCalculateFlowFieldPartJob(job, aWorkerId);
                }
                break;

            default:
                HG_UNREACHABLE("Invalid Job::Kind value ({}).", (int)job.kind);
            }
        }
    }

    //! Note: call without holding the mutex.
    //! \returns pointer to station that was assigned to this Job/Request (never NULL).
    NeverNull<Station*> _workCalculateIntegrationFieldJob(Job& aJob, PZInteger aWorkerId) {
        Station* const station = _stations.Do([this, &aJob](StationList& aIt) -> Station* {
            for (auto& station : aIt) {
                if (!station.isOccupied) {
                    return _assignStation(station, aJob);
                }
            }
            return nullptr;
        });
        if (!station) {
            HG_UNREACHABLE("No unoccupied station was found to work on a request.");
        }

        while (true) {
            /* SYNCHRONIZATION */
            {
                std::unique_lock<Mutex> lock{_mutex};
                _setWorkerStatus(aWorkerId, WorkerStatus::PREP_OR_IDLE, lock);
                while (true) {
                    if (_stopped) {
                        return station; // Spooler is being destroyed
                    }
                    if (aJob.request->cancelled.load()) {
                        return station; // Request cancelled
                    }
                    if (!_paused) {
                        break; // Ready to work
                    }
                    _cv.wait(lock);
                }
                _setWorkerStatus(aWorkerId, WorkerStatus::WORKING, lock);
                lock.unlock();
            }

            /* WORK */
            if (station->flowFieldCalculator.calculateIntegrationField(256)) {
                HG_LOG_DEBUG(LOG_ID, "Worker {} finished calculating an integration field.", aWorkerId);
                break;
            }
        }

        return station;
    }

    //! Note: call without holding the mutex.
    void _finalizeCalculateIntegrationFieldJob(Job& aJob, PZInteger aWorkerId, Station* aStation) {
        const auto totalRowCount = aJob.calcIntegrationFieldData.fieldDimensions.y;
        const auto rowsPerJob    = (totalRowCount + (_concurrencyLimit - 1)) / _concurrencyLimit;

        std::vector<Job> newJobs;
        PZInteger        rowsCovered = 0;
        while (rowsCovered < totalRowCount) {
            newJobs.push_back(CreateCalcFlowFieldPartJob(aJob.request,
                                                         aStation,
                                                         aWorkerId,
                                                         rowsCovered,
                                                         (rowsCovered + rowsPerJob < totalRowCount)
                                                             ? rowsPerJob
                                                             : (totalRowCount - rowsCovered)));
            rowsCovered += rowsPerJob;
        }

        aStation->remainingJobCount.store(static_cast<int>(newJobs.size()));

        {
            std::unique_lock<Mutex> lock{_mutex};
            _addJobs(newJobs, lock);
        }
    }

    //! Note: call without holding the mutex.
    void _workCalculateFlowFieldPartJob(Job& aJob, PZInteger aWorkerId) {
        static constexpr PZInteger ROWS_PER_ITER = 4;

        const auto startingRow = aJob.calcFlowFieldPartData.startingRow;
        const auto rowCount    = aJob.calcFlowFieldPartData.rowCount;

        for (PZInteger y = startingRow; y < startingRow + rowCount; y += ROWS_PER_ITER) {
            /* SYNCHRONIZATION */
            {
                std::unique_lock<Mutex> lock{_mutex};
                _setWorkerStatus(aWorkerId, WorkerStatus::PREP_OR_IDLE, lock);
                while (true) {
                    if (_stopped) {
                        return; // Spooler is being destroyed
                    }
                    if (aJob.request->cancelled.load()) {
                        return; // Request cancelled
                    }
                    if (!_paused) {
                        break; // Ready to work
                    }
                    _cv.wait(lock);
                }
                _setWorkerStatus(aWorkerId, WorkerStatus::WORKING, lock);
                lock.unlock();
            }

            /* WORK */
            const auto& jobData = aJob.calcFlowFieldPartData;
            jobData.station->flowFieldCalculator.calculateFlowField(
                y,
                (y + ROWS_PER_ITER <= startingRow + rowCount) ? ROWS_PER_ITER
                                                              : (startingRow + rowCount - y));
        }
    }

    //! Note: call without holding the mutex.
    void _finalizeCalculateFlowFieldPartJob(Job& aJob, PZInteger aWorkerId) {
        HG_ASSERT(aJob.kind == Job::CALCULATE_FLOW_FIELD_PART);

        const auto& jobData = aJob.calcFlowFieldPartData;
        const auto  fsRes   = jobData.station->remainingJobCount.fetch_sub(1);

        HG_LOG_DEBUG(LOG_ID, "Worker {} fsRes = {}.", aWorkerId, fsRes);
        if (fsRes == 1) {
            // There was exactly one job left - this one - so the calculation is finished
            auto flowField                    = jobData.station->flowFieldCalculator.takeFlowField();
            aJob.request->result              = {std::move(flowField)};
            aJob.request->remainingIterations = 0;
            HG_LOG_DEBUG(LOG_ID, "Worker {} signalling latch.", aWorkerId);
            aJob.request->latch.open();
            jobData.station->isOccupied = false;
        }
    }
};

void FlowFieldSpoolerImpl::tick() {
    HG_VALIDATE_PRECONDITION(_paused == false);

    _requests.Do([this](RequestMap& aIt) {
        for (auto iter = aIt.begin(); iter != aIt.end();) {
            auto& request = *(iter->second);

            if (request.remainingIterations > 1) {
                request.remainingIterations -= 1;
            } else if (request.remainingIterations == 1) {
                request.remainingIterations -= 1;
                request.latch.wait(); // Warning: will block all access to _requests until resolved!
            } else {
                request.remainingIterations -= 1;
                if (request.remainingIterations == -(_finishedRequestExpirationLimit + 1)) {
                    HG_LOG_WARN(
                        LOG_ID,
                        "Removing expired request (id: {}) - result calculated but never collected.",
                        request.id);
                    iter = aIt.erase(iter);
                    continue;
                }
            }
            iter = std::next(iter);
        }
    });
}

void FlowFieldSpoolerImpl::pause() {
    using namespace std::chrono;
    const auto start_time = steady_clock::now();
    {
        std::unique_lock<Mutex> lock{_mutex};

        _paused = true;

        _cv_workerStatuses.wait(lock, [this]() -> bool {
            for (const auto status : _workerStatuses) {
                if (status == WorkerStatus::WORKING) {
                    return false;
                }
            }
            return true;
        });
    }
    const auto end_time = steady_clock::now();
    const auto duration = duration_cast<microseconds>(end_time - start_time);
    if (duration > microseconds{500}) {
        HG_LOG_WARN(LOG_ID,
                    "Pausing the spooler took too an unexpectedly long time {}ms.",
                    duration.count() / 1000.0);
    }
}

void FlowFieldSpoolerImpl::unpause() {
    std::unique_lock<Mutex> lock{_mutex};
    _paused = false;
    _cv.notify_all();
}

std::uint64_t FlowFieldSpoolerImpl::addRequest(math::Vector2pz aFieldTopLeft,
                                               math::Vector2pz aFieldDimensions,
                                               math::Vector2pz aTarget,
                                               std::int32_t    aCostProviderId,
                                               PZInteger       aMaxIterations) {
    HG_VALIDATE_ARGUMENT(aTarget.x >= aFieldTopLeft.x && aTarget.y >= aFieldTopLeft.y &&
                         aTarget.x < aFieldTopLeft.x + aFieldDimensions.x &&
                         aTarget.y < aFieldTopLeft.y + aFieldDimensions.y);
    HG_VALIDATE_ARGUMENT(aMaxIterations > 0, "aMaxIterations must be at least 1.");
    HG_VALIDATE_ARGUMENT(_wcfMap.find(aCostProviderId) != _wcfMap.end(),
                         "Invalid cost provider ID provided.");

    const auto id                = _requestIdCounter.fetch_add(1);
    auto       request           = std::make_shared<Request>(id, aCostProviderId);
    request->fieldTopLeft        = aFieldTopLeft;
    request->remainingIterations = aMaxIterations;

    HG_LOG_DEBUG(LOG_ID,
                 "Flow Field Request (id: {}; topLeft: ({},{}); size: ({}x{}); target: ({},{}), "
                 "maxIter: {}) created.",
                 id,
                 aFieldTopLeft.x,
                 aFieldTopLeft.y,
                 aFieldDimensions.x,
                 aFieldDimensions.y,
                 aTarget.x,
                 aTarget.y,
                 aMaxIterations);

    auto job = CreateCalcIntegrationFieldJob(request, aFieldTopLeft, aFieldDimensions, aTarget);

    _requests.Do([request_ = std::move(request), id](RequestMap& aIt) mutable {
        aIt[id] = std::move(request_);
    });

    {
        std::unique_lock<Mutex> lock{_mutex};
        _addJob(job, lock);
    }

    return id;
}

void FlowFieldSpoolerImpl::cancelRequest(std::uint64_t aRequestId) {
    std::shared_ptr<Request> request;
    _requests.Do([aRequestId, &request](RequestMap& aIt) {
        auto iter = aIt.find(aRequestId);
        if (iter != aIt.end()) {
            std::swap(request, iter->second);
            aIt.erase(iter);
        }
    });

    if (request != nullptr) {
        request->cancelled.store(true);
        HG_LOG_INFO(LOG_ID, "Request {} cancelled.", aRequestId);
    }
}

std::optional<OffsetFlowField> FlowFieldSpoolerImpl::collectResult(std::uint64_t aRequestId) {
    return _requests.Do([aRequestId](RequestMap& aIt) -> std::optional<OffsetFlowField> {
        const auto iter = aIt.find(aRequestId);
        HG_HARD_ASSERT(iter != aIt.end()); // TODO: different exception

        auto& request = iter->second;

        if (!request->result.has_value()) {
            return {};
        }

        OffsetFlowField result;
        result.flowField = std::move(*(request->result));
        result.offset    = request->fieldTopLeft;

        aIt.erase(iter);

        return {result};
    });
}

} // namespace

std::unique_ptr<FlowFieldSpoolerImplInterface> CreateDefaultFlowFieldSpoolerImpl(
    WCFMap    aWcfMap,
    PZInteger aConcurrencyLimit,
    PZInteger aFinishedRequestExpirationLimit) {
    return std::make_unique<FlowFieldSpoolerImpl>(std::move(aWcfMap),
                                                  aConcurrencyLimit,
                                                  aFinishedRequestExpirationLimit);
}

} // namespace detail
} // namespace util
HOBGOBLIN_NAMESPACE_END

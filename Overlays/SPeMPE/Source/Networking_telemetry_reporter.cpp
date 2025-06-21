// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off


#include <SPeMPE/Managers/Networking_manager_interface.hpp>
#include <SPeMPE/Utility/Networking_telemetry_reporter.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>

namespace jbatnozic {
namespace spempe {

static constexpr auto LOG_ID = "SPeMPE";

NetworkingTelemetryReporter::NetworkingTelemetryReporter(hobgoblin::QAO_IKey aIKey,
                                                         int aExecutionPriority,
                                                         const Config& aConfig)
    : NonstateObject{aIKey,
                     SPEMPE_TYPEID_SELF,
                     aExecutionPriority,
                     "NetworkingTelemetryReporter"}
    , _cycleLength{aConfig.cycleLength}
    , _counter{0}
{
    HG_VALIDATE_ARGUMENT(aConfig.cycleLength > 0);
}

void NetworkingTelemetryReporter::_eventPostUpdate() {
    _counter += 1;
    if (_counter < _cycleLength) {
        return;
    }

    const auto& netMgr    = ccomp<NetworkingManagerInterface>();
    const auto& telemetry = netMgr.getTelemetry(_cycleLength);
    HG_LOG_INFO(
        LOG_ID,
        "Bandwidth usage in the last {} iteration(s):\n      UPLOADED: {:6.2f}kB\n    DOWNLOADED: {:6.2f}kB",
        _cycleLength,
        static_cast<double>(telemetry.uploadByteCount) / 1024.0,
        static_cast<double>(telemetry.downloadByteCount) / 1024.0
    );
    
    _counter = 0;
}

} // namespace spempe
} // namespace jbatnozic

// clang-format on

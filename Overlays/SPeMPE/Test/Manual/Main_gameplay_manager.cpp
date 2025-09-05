// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Main_gameplay_manager.hpp"

#include <Hobgoblin/Input.hpp>
#include <Hobgoblin/Logging.hpp>

#include "Actors.hpp"
#include "Config.hpp"

static constexpr auto LOG_ID = "SPeMPE.ManualTest";

MainGameplayManagerBase::MainGameplayManagerBase(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard, SPEMPE_TYPEID_SELF, PRIORITY_GAMEPLAYMGR, "MainGameplayManager"} {}

void MainGameplayManagerBase::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);

    const int execPriority = 10;  // not really important for these objects
    const int cycleLength  = 600; // 10 seconds @ 60fps
    {
        const auto config = spe::EventLoopTimingReporter::Config{cycleLength};
        QAO_Create<spe::EventLoopTimingReporter>(ctx().getQAORuntime(), execPriority, config);
    }
    {
        const auto config = spe::NetworkingTelemetryReporter::Config{cycleLength};
        QAO_Create<spe::NetworkingTelemetryReporter>(ctx().getQAORuntime(), execPriority, config);

        ccomp<spe::NetworkingManagerInterface>().setTelemetryCycleLimit(cycleLength);
    }
}

void MainGameplayManagerBase::_eventPreUpdate() {
    if (hg::in::CheckPressedPK(hg::in::PK_RSHIFT)) {
        ctx().stop();
    }
}

namespace singleplayer {

MainGameplayManager::MainGameplayManager(QAO_InstGuard aInstGuard)
    : MainGameplayManagerBase{aInstGuard} {}

void MainGameplayManager::_didAttach(QAO_Runtime& aRuntime) {
    MainGameplayManagerBase::_didAttach(aRuntime);

    auto p = QAO_Create<BasicActor>(aRuntime);
    p->init(32.f, 32.f, hg::uwga::COLOR_PURPLE, 0);
}

} // namespace singleplayer

namespace multiplayer {

MainGameplayManager::MainGameplayManager(QAO_InstGuard aInstGuard)
    : MainGameplayManagerBase{aInstGuard} {}

void MainGameplayManager::_didAttach(QAO_Runtime& aRuntime) {
    MainGameplayManagerBase::_didAttach(aRuntime);

    if (ctx().isPrivileged()) {
#if 1
        auto basicActor = QAO_Create<BasicActor>(aRuntime);
        basicActor->init(32.f, 32.f, hg::uwga::COLOR_PURPLE, 0);
#endif
#if 1
        auto autodiffActor = QAO_Create<AutodiffActor>(aRuntime);
        autodiffActor->init(32.f, 96.f, hg::uwga::COLOR_ORANGE, 0);
#endif
#if 1
        auto alternatingActor = QAO_Create<AlternatingActor>(aRuntime);
        alternatingActor->init(32.f, 160.f, hg::uwga::COLOR_ROYAL_BLUE, 0);
#endif
#if 1
        auto alternatingAutodiffActor = QAO_Create<AlternatingAutodiffActor>(aRuntime);
        alternatingAutodiffActor->init(32.f, 224.f, hg::uwga::COLOR_FOREST_GREEN, 0);
#endif
    }
}

} // namespace multiplayer

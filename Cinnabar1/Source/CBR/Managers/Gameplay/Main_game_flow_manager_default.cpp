// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Main_game_flow_manager_default.hpp"

#include <CBR/Managers/UserInterface/Interactivity_manager_default.hpp>
#include <CBR/Overworld/Asteroid.hpp>
#include <CBR/Ship/Ship_controller.hpp>
#include <CBR/Ship/Ship_starting_block.hpp>

namespace cinnabar {

DefaultMainGameFlowManager::DefaultMainGameFlowManager(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard,
                          QAO_ExeCon::ESSENTIAL,
                          PRIORITY_MAINGAMEFLOWMGR,
                          QAO_STATIC_NAME("cinnabar::DefaultMainGameFlowManager")} {}

void DefaultMainGameFlowManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);

    ccomp<MNetworking>().addEventListener(this);

    // Init scenario (TEMPORARY)

    if (ctx().isPrivileged()) {
        auto core = ShipStartingBlock::createMaster(aRuntime, {150.0, 150.0});
        auto ship = ShipController::createMaster(aRuntime, *core);

        auto asteroid = Asteroid::createMaster(aRuntime, {512.0, 712.0});
    }
}

void DefaultMainGameFlowManager::_willDetach(QAO_Runtime& aRuntime) {
    ccomp<MNetworking>().removeEventListener(this);
    spe::NonstateObject::_willDetach(aRuntime);
}

void DefaultMainGameFlowManager::_eventDisplay() {
    auto& context = ctx();
    auto& gs = context.getGameState();
    
    const auto currentOrdinal = context.getCurrentIterationOrdinal();
    if (_iterOrdinal <= currentOrdinal) {
        _iterOrdinal = currentOrdinal + 1;
    } else {
        return;
    }
    
    const auto& caller = getName();

    // Update
    {
        auto level = gs.getUpdateExeconLevel();
        if (level < QAO_ExeCon::META_EXECUTE_ALL) {
            level = static_cast<QAO_ExeCon>((int)level + 1);
        }
        gs.setUpdateExeconLevel(level, caller);
    }

    // Draw
    if (!context.isHeadless()) {
        auto level = gs.getDrawExeconLevel();
        if (level < QAO_ExeCon::META_EXECUTE_ALL) {
            level = static_cast<QAO_ExeCon>((int)level + 1);
        }
        gs.setDrawExeconLevel(level, caller);
    }

    // Display
    // No need to increase it, only essential managers do stuff in DISPLAY anyway.
}

void DefaultMainGameFlowManager::onNetworkingEvent(const RN_Event& aEvent) {
    // TODO: The main game flow manager is probably not the best place to upload
    //       local lobby info; consider moving this elsewhere (e.g. lobby backend
    //       or a dedicated client connection handler).
    if (ccomp<MNetworking>().isClient()) {
        aEvent.visit([this](const RN_Event::Connected&) {
            HG_LOG_INFO(LOG_ID, "Client lobby uploading local info to server.");
            ccomp<MLobbyBackend>().uploadLocalInfo();
        });
    }
}

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Main_game_flow_manager.hpp>

#include <Asteroid.hpp>
#include <Ship.hpp>

namespace cinnabar {

MainGameFlowManager::MainGameFlowManager(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard,
                          QAO_ExeCon::ESSENTIAL,
                          PRIORITY_MAINGAMEFLOWMGR,
                          QAO_STATIC_NAME("cinnabar::MainGameFlowManager")} {}

void MainGameFlowManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);

    auto ship = QAO_Create<Ship>(aRuntime);
    ship->init(96.0, 96.0);

    auto asteroid = QAO_Create<Asteroid>(aRuntime);
    asteroid->init(256.0, 256.0);
}

void MainGameFlowManager::_eventDisplay() {
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

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Overworld_manager.hpp>

#include <Overworld_collisions.hpp>

namespace cinnabar {

OverworldManager::OverworldManager(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       PRIORITY_ENVIRONMENTMGR,
                       QAO_STATIC_NAME("cinnabar::OverworldManager")} //
{
    InitOverworldCollisions(_mcd, _space);
}

hg::alvin::Space& OverworldManager::getAlvinSpace() {
    return _space;
}

const hg::alvin::Space& OverworldManager::getAlvinSpace() const {
    return _space;
}

void OverworldManager::_eventUpdate1() {
    _space.step(1.0 / 60.0); // TODO: magic number
}

} // namespace cinnabar

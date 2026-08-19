// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Overworld_manager_default.hpp>

#include <Overworld_collisions.hpp>

namespace cinnabar {

DefaultOverworldManager::DefaultOverworldManager(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       PRIORITY_ENVIRONMENTMGR,
                       QAO_STATIC_NAME("cinnabar::DefaultOverworldManager")} //
{
    InitOverworldCollisions(_mcd, _space);
}

hg::alvin::Space& DefaultOverworldManager::getAlvinSpace() {
    return _space;
}

const hg::alvin::Space& DefaultOverworldManager::getAlvinSpace() const {
    return _space;
}

void DefaultOverworldManager::_eventUpdate1() {
    _space.step(1.0 / 60.0); // TODO: magic number
}

} // namespace cinnabar

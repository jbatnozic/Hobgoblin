// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

namespace cinnabar {

struct PlayerControls {
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
};

constexpr auto CTRLNAME_LEFT  = "left";
constexpr auto CTRLNAME_RIGHT = "right";
constexpr auto CTRLNAME_UP    = "up";
constexpr auto CTRLNAME_DOWN  = "down";

inline void SetUpPlayerControlsDefinitions(spe::InputSyncManager& aInputSyncManager) {
    spe::InputSyncManagerWrapper wrapper{aInputSyncManager};
    wrapper.defineSignal<bool>(CTRLNAME_LEFT, false);
    wrapper.defineSignal<bool>(CTRLNAME_RIGHT, false);
    wrapper.defineSignal<bool>(CTRLNAME_UP, false);
    wrapper.defineSignal<bool>(CTRLNAME_DOWN, false);
}

} // namespace cinnabar

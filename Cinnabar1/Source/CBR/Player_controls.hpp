// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <SPeMPE/Managers/Input_sync_manager.hpp>

namespace jbatnozic {
namespace spempe {
class WindowFrameInputView;
} // namespace spempe
} // namespace jbatnozic

namespace cinnabar {

namespace spe = ::jbatnozic::spempe;

struct PlayerControls {
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;

    bool rotateClockwise        = false;
    bool rotateCounterclockwise = false;

    //! \brief Create an instance of `PlayerControls` by reading local input
    //!        as provided by a `WindowManager`.
    //!
    //! \param aInput input view produced by the local `WindowManager` to read the input from.
    static PlayerControls fromLocalInput(const spe::WindowFrameInputView& aInput);

    //! \brief Create an instance of `PlayerControls` by reading remote  input
    //!        as provided by an `InputSyncManager`.
    //!
    //! \param aInputSyncManager reference to the local `InputSyncManager` instance.
    //! \param aClientIndex index of the client for which to read the input
    //!                     (use `spempe::CLIENT_INDEX_LOCAL` to read the input of the local player).
    static PlayerControls fromInputSyncManager(spe::InputSyncManager& aInputSyncManager,
                                               int                    aClientIndex);

    //! \brief Set the signal values events of an `InputSyncManager` (running locally in Client mode)
    //!        according to the values held in `this` instance.
    void toInputSyncManager(spe::InputSyncManager& aInputSyncManager) const;
};

///////////////////////////////////////////////////////////////////////////
// CONTROL DEFINITIONS                                                   //
///////////////////////////////////////////////////////////////////////////

constexpr auto CTRLNAME_LEFT  = "left";
constexpr auto CTRLNAME_RIGHT = "right";
constexpr auto CTRLNAME_UP    = "up";
constexpr auto CTRLNAME_DOWN  = "down";

constexpr auto CTRLNAME_ROTATE_CLOCKWISE        = "rotCw";
constexpr auto CTRLNAME_ROTATE_COUNTERCLOCKWISE = "RotCCw";

inline void SetUpPlayerControlsDefinitions(spe::InputSyncManager& aInputSyncManager) {
    spe::InputSyncManagerWrapper wrapper{aInputSyncManager};
    wrapper.defineSignal<bool>(CTRLNAME_LEFT, false);
    wrapper.defineSignal<bool>(CTRLNAME_RIGHT, false);
    wrapper.defineSignal<bool>(CTRLNAME_UP, false);
    wrapper.defineSignal<bool>(CTRLNAME_DOWN, false);

    wrapper.defineSignal<bool>(CTRLNAME_ROTATE_CLOCKWISE, false);
    wrapper.defineSignal<bool>(CTRLNAME_ROTATE_COUNTERCLOCKWISE, false);
}

///////////////////////////////////////////////////////////////////////////
// METHOD IMPLEMENTATIONS                                                //
///////////////////////////////////////////////////////////////////////////

#ifndef ENABLE_PLAYER_CONTROLS_IMPL
#define ENABLE_PLAYER_CONTROLS_IMPL 0 // The .cpp file defines this to 1 before including
#endif

#if ENABLE_PLAYER_CONTROLS_IMPL

PlayerControls PlayerControls::fromInputSyncManager(spe::InputSyncManager& aInputSyncManager,
                                                    int                    aClientIndex) {
    spe::InputSyncManagerWrapper wrapper{aInputSyncManager};

    const int idx = aClientIndex;

    PlayerControls controls{
        .left                   = wrapper.getSignalValue<bool>(idx, CTRLNAME_LEFT),
        .right                  = wrapper.getSignalValue<bool>(idx, CTRLNAME_RIGHT),
        .up                     = wrapper.getSignalValue<bool>(idx, CTRLNAME_UP),
        .down                   = wrapper.getSignalValue<bool>(idx, CTRLNAME_DOWN),
        .rotateClockwise        = wrapper.getSignalValue<bool>(idx, CTRLNAME_ROTATE_CLOCKWISE),
        .rotateCounterclockwise = wrapper.getSignalValue<bool>(idx, CTRLNAME_ROTATE_COUNTERCLOCKWISE),
    };

    return controls;
}

PlayerControls PlayerControls::fromLocalInput(const spe::WindowFrameInputView& aInput) {
    namespace hg = ::jbatnozic::hobgoblin;
    return {
        .left                   = aInput.checkPressed(hg::in::PK_A),
        .right                  = aInput.checkPressed(hg::in::PK_D),
        .up                     = aInput.checkPressed(hg::in::PK_W),
        .down                   = aInput.checkPressed(hg::in::PK_S),
        .rotateClockwise        = aInput.checkPressed(hg::in::PK_E),
        .rotateCounterclockwise = aInput.checkPressed(hg::in::PK_Q),
    };
}

void PlayerControls::toInputSyncManager(spe::InputSyncManager& aInputSyncManager) const {
    spe::InputSyncManagerWrapper wrapper{aInputSyncManager};

    wrapper.setSignalValue(CTRLNAME_LEFT, left);
    wrapper.setSignalValue(CTRLNAME_RIGHT, right);
    wrapper.setSignalValue(CTRLNAME_UP, up);
    wrapper.setSignalValue(CTRLNAME_DOWN, down);
    wrapper.setSignalValue(CTRLNAME_ROTATE_CLOCKWISE, rotateClockwise);
    wrapper.setSignalValue(CTRLNAME_ROTATE_COUNTERCLOCKWISE, rotateCounterclockwise);
}

#endif

} // namespace cinnabar

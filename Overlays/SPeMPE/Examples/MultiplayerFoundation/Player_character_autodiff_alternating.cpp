// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#include "Player_character_autodiff_alternating.hpp"

#include <Hobgoblin/Logging.hpp>
#include <iostream>

AutodiffAlternatingPlayerCharacter::AutodiffAlternatingPlayerCharacter(QAO_InstGuard aInstGuard,
                                                                       spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, PRIORITY_PLAYERAVATAR,
                   "AutodiffAlternatingPlayerCharacter", aSyncId}
{
    _enableAlternatingUpdates();
}

void AutodiffAlternatingPlayerCharacter::_didAttach(QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);
    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
}

void AutodiffAlternatingPlayerCharacter::init(int aOwningPlayerIndex, float aX, float aY) {
    assert(isMasterObject());

    auto& self = _getCurrentState();
    self.x = aX;
    self.y = aY;
    self.owningPlayerIndex = aOwningPlayerIndex;
}

void AutodiffAlternatingPlayerCharacter::_eventUpdate1(spe::IfMaster) {
    if (ctx().getGameState().isPaused) return;

    auto& self = _getCurrentState();
    assert(self.owningPlayerIndex >= 0);
    if (const auto clientIndex = ccomp<MLobbyBackend>().playerIdxToClientIdx(self.owningPlayerIndex);
        clientIndex != spe::CLIENT_INDEX_UNKNOWN) {

        spe::InputSyncManagerWrapper wrapper{ccomp<MInput>()};

        const bool left = wrapper.getSignalValue<bool>(clientIndex, "left");
        const bool right = wrapper.getSignalValue<bool>(clientIndex, "right");

        self.x += (10.f * ((float)right - (float)left));

        const bool up = wrapper.getSignalValue<bool>(clientIndex, "up");
        const bool down = wrapper.getSignalValue<bool>(clientIndex, "down");

        self.y += (10.f * ((float)down - (float)up));

        wrapper.pollSimpleEvent(clientIndex, "jump",
                                [&]() {
            self.y -= 16.f;
        });
    }
}

void AutodiffAlternatingPlayerCharacter::_eventPostUpdate(spe::IfMaster) {
    if (_didAlternatingUpdatesSync()) {
        _getCurrentState().commit();
    }
}

void AutodiffAlternatingPlayerCharacter::_eventDraw1() {
    if (this->isDeactivated()) return;

    #define NUM_COLORS 12
    static const hg::gr::Color COLORS[NUM_COLORS] = {
        hg::gr::COLOR_RED,
        hg::gr::COLOR_GREEN,
        hg::gr::COLOR_YELLOW,
        hg::gr::COLOR_BLUE,
        hg::gr::COLOR_ORANGE,
        hg::gr::COLOR_PURPLE,
        hg::gr::COLOR_TEAL,
        hg::gr::COLOR_BROWN,
        hg::gr::COLOR_FUCHSIA,
        hg::gr::COLOR_GREY,
        hg::gr::COLOR_WHITE,
        hg::gr::COLOR_AQUA,
    };

    const auto& self_curr = _getCurrentState();
    const auto& self_next = _getFollowingState();

    hg::gr::CircleShape circle{20.f};
    circle.setFillColor(COLORS[self_curr.owningPlayerIndex % NUM_COLORS]);
    circle.setPosition({
        (self_curr.x + self_next.x) / 2.f,
        (self_curr.y + self_next.y) / 2.f
    });
    ccomp<MWindow>().getCanvas().draw(circle);
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AutodiffAlternatingPlayerCharacter, (CREATE, UPDATE, DESTROY));

void AutodiffAlternatingPlayerCharacter::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AutodiffAlternatingPlayerCharacter, aSyncCtrl);
}

void AutodiffAlternatingPlayerCharacter::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AutodiffAlternatingPlayerCharacter, aSyncCtrl);
}

void AutodiffAlternatingPlayerCharacter::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AutodiffAlternatingPlayerCharacter, aSyncCtrl);
}

// clang-format on

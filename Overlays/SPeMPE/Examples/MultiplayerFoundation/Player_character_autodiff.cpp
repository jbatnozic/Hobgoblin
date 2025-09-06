// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Player_character_autodiff.hpp"

#include <Hobgoblin/UWGA.hpp>

AutodiffPlayerCharacter::AutodiffPlayerCharacter(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   SPEMPE_TYPEID_SELF,
                   PRIORITY_PLAYERAVATAR,
                   "AutodiffPlayerCharacter",
                   aSyncId} {}

void AutodiffPlayerCharacter::_didAttach(QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);
    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
}

void AutodiffPlayerCharacter::init(int aOwningPlayerIndex, float aX, float aY) {
    assert(isMasterObject());

    auto& self             = _getCurrentState();
    self.x                 = aX;
    self.y                 = aY;
    self.owningPlayerIndex = aOwningPlayerIndex;
}

void AutodiffPlayerCharacter::_eventUpdate1(spe::IfMaster) {
    if (ctx().getGameState().isPaused)
        return;

    auto& self = _getCurrentState();
    assert(self.owningPlayerIndex >= 0);
    if (const auto clientIndex = ccomp<MLobbyBackend>().playerIdxToClientIdx(self.owningPlayerIndex);
        clientIndex != spe::CLIENT_INDEX_UNKNOWN) {

        spe::InputSyncManagerWrapper wrapper{ccomp<MInput>()};

        const bool left  = wrapper.getSignalValue<bool>(clientIndex, "left");
        const bool right = wrapper.getSignalValue<bool>(clientIndex, "right");

        self.x += (10.f * ((float)right - (float)left));

        const bool up   = wrapper.getSignalValue<bool>(clientIndex, "up");
        const bool down = wrapper.getSignalValue<bool>(clientIndex, "down");

        self.y += (10.f * ((float)down - (float)up));

        wrapper.pollSimpleEvent(clientIndex, "jump", [&]() {
            self.y -= 16.f;
        });
    }
}

void AutodiffPlayerCharacter::_eventPostUpdate(spe::IfMaster) {
    _getCurrentState().commit();
}

void AutodiffPlayerCharacter::_eventDraw1() {
    if (this->isDeactivated())
        return;

#define NUM_COLORS 12
    static const hg::uwga::Color COLORS[NUM_COLORS] = {
        hg::uwga::COLOR_RED,
        hg::uwga::COLOR_GREEN,
        hg::uwga::COLOR_YELLOW,
        hg::uwga::COLOR_BLUE,
        hg::uwga::COLOR_ORANGE,
        hg::uwga::COLOR_PURPLE,
        hg::uwga::COLOR_TEAL,
        hg::uwga::COLOR_BROWN,
        hg::uwga::COLOR_FUCHSIA,
        hg::uwga::COLOR_GREY,
        hg::uwga::COLOR_WHITE,
        hg::uwga::COLOR_AQUA,
    };

    const auto& self = _getCurrentState();

    auto& canvas = ccomp<MWindow>().getActiveCanvas();

    hg::uwga::CircleShape circle{canvas.getSystem(), 20.f};
    circle.setFillColor(COLORS[self.owningPlayerIndex % NUM_COLORS]);
    circle.setPosition({self.x, self.y});
    canvas.draw(circle);
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AutodiffPlayerCharacter, (CREATE, UPDATE, DESTROY));

void AutodiffPlayerCharacter::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AutodiffPlayerCharacter, aSyncCtrl);
}

void AutodiffPlayerCharacter::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AutodiffPlayerCharacter, aSyncCtrl);
}

void AutodiffPlayerCharacter::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AutodiffPlayerCharacter, aSyncCtrl);
}

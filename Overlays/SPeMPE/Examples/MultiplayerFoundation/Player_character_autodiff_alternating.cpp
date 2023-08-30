
#include "Player_character_autodiff_alternating.hpp"

#include <Hobgoblin/Logging.hpp>
#include <iostream>

AutodiffAlternatingPlayerCharacter::AutodiffAlternatingPlayerCharacter(QAO_RuntimeRef aRuntimeRef,
                                                                       spe::RegistryId aRegId,
                                                                       spe::SyncId aSyncId)
    : SyncObjSuper{aRuntimeRef, SPEMPE_TYPEID_SELF, PRIORITY_PLAYERAVATAR,
                   "AutodiffAlternatingPlayerCharacter", aRegId, aSyncId}
{
    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
    _enableAlternatingUpdates();
}

AutodiffAlternatingPlayerCharacter::~AutodiffAlternatingPlayerCharacter() {
    if (isMasterObject()) {
        doSyncDestroy();
    }
}

void AutodiffAlternatingPlayerCharacter::init(int aOwningPlayerIndex, float aX, float aY) {
    assert(isMasterObject());

    auto& self = _getCurrentState();
    self.x = aX;
    self.y = aY;
    self.owningPlayerIndex = aOwningPlayerIndex;
}

void AutodiffAlternatingPlayerCharacter::_eventUpdate(spe::IfMaster) {
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

void AutodiffAlternatingPlayerCharacter::_eventDraw1() {
    if (this->isDeactivated()) return;

    #define NUM_COLORS 12
    static const hg::gr::Color COLORS[NUM_COLORS] = {
        hg::gr::Color::Red,
        hg::gr::Color::Green,
        hg::gr::Color::Yellow,
        hg::gr::Color::Blue,
        hg::gr::Color::Orange,
        hg::gr::Color::Purple,
        hg::gr::Color::Teal,
        hg::gr::Color::Brown,
        hg::gr::Color::Fuchsia,
        hg::gr::Color::Grey,
        hg::gr::Color::White,
        hg::gr::Color::Aqua,
    };

    const auto& self_curr = _getCurrentState();
    const auto& self_next = _getFollowingState();

    sf::CircleShape circle{20.f};
    circle.setFillColor(COLORS[self_curr.owningPlayerIndex % NUM_COLORS]);
    circle.setPosition({
        (self_curr.x + self_next.x) / 2.f,
        (self_curr.y + self_next.y) / 2.f
    });
    ccomp<MWindow>().getCanvas().draw(circle);
}

void AutodiffAlternatingPlayerCharacter::_eventFinalizeFrame(spe::IfMaster) {
    if (_didAlternatingUpdatesSync()) {
        _getCurrentState().commit();
    }
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AutodiffAlternatingPlayerCharacter, (CREATE, UPDATE, DESTROY));

void AutodiffAlternatingPlayerCharacter::_syncCreateImpl(spe::SyncDetails& aSyncDetails) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AutodiffAlternatingPlayerCharacter, aSyncDetails);
}

void AutodiffAlternatingPlayerCharacter::_syncUpdateImpl(spe::SyncDetails& aSyncDetails) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AutodiffAlternatingPlayerCharacter, aSyncDetails);
}

void AutodiffAlternatingPlayerCharacter::_syncDestroyImpl(spe::SyncDetails& aSyncDetails) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AutodiffAlternatingPlayerCharacter, aSyncDetails);
}
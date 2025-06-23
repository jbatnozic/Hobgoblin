// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Actors.hpp"
#include "Config.hpp"

#include <Hobgoblin/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
// MARK: BASIC ACTOR                                                     //
///////////////////////////////////////////////////////////////////////////

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(BasicActor, (CREATE, UPDATE, DESTROY));

BasicActor::BasicActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, PRIORITY_ACTOR, "BasicActor", aSyncId} {}

void BasicActor::init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex) {
    HG_HARD_ASSERT(isMasterObject());
    auto& self = _getCurrentState();
    self.x     = aX;
    self.y     = aY;
    self.color = aColor.toInt();
    self.index = aIndex;
}

void BasicActor::_eventUpdate1(spe::IfMaster) {
    auto& self          = _getCurrentState();
    int   stateDuration = 0;
    switch (_state) {
    case State::WAIT_LEFT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_RIGHT:
        self.x += SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    case State::WAIT_RIGHT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_LEFT:
        self.x -= SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    default:
        HG_UNREACHABLE("Invalid value for State ({}).", (int)_state);
    }

    _durationCounter += 1;
    if (_durationCounter >= stateDuration) {
        _state           = static_cast<State>(((int)_state + 1) % (int)State::STATE_COUNT);
        _durationCounter = 0;
    }
}

void BasicActor::_eventDraw1() {
    if (this->isDeactivated()) {
        return;
    }

    auto& winMgr = ccomp<spe::WindowManagerInterface>();
    auto& canvas = winMgr.getCanvas();

    const auto& self = _getCurrentState();

    hg::gr::CircleShape circle{32.f};
    circle.setOrigin(32.f, 32.f);
    circle.setFillColor(hg::gr::Color{self.color});
    circle.setPosition(self.x, self.y);
    canvas.draw(circle);
}

void BasicActor::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(BasicActor, aSyncCtrl);
}

void BasicActor::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(BasicActor, aSyncCtrl);
}

void BasicActor::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(BasicActor, aSyncCtrl);
}

///////////////////////////////////////////////////////////////////////////
// MARK: AUTODIFF ACTOR                                                  //
///////////////////////////////////////////////////////////////////////////

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AutodiffActor, (CREATE, UPDATE, DESTROY));

AutodiffActor::AutodiffActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, PRIORITY_ACTOR, "AutodiffActor", aSyncId} {}

void AutodiffActor::_didAttach(hg::QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);

    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
}

void AutodiffActor::init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex) {
    HG_HARD_ASSERT(isMasterObject());
    auto& self = _getCurrentState();
    self.x     = aX;
    self.y     = aY;
    self.color = aColor.toInt();
    self.index = aIndex;
}

void AutodiffActor::_eventUpdate1(spe::IfMaster) {
    auto& self          = _getCurrentState();
    int   stateDuration = 0;
    switch (_state) {
    case State::WAIT_LEFT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_RIGHT:
        self.x += SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    case State::WAIT_RIGHT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_LEFT:
        self.x -= SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    default:
        HG_UNREACHABLE("Invalid value for State ({}).", (int)_state);
    }

    _durationCounter += 1;
    if (_durationCounter >= stateDuration) {
        _state           = static_cast<State>(((int)_state + 1) % (int)State::STATE_COUNT);
        _durationCounter = 0;
    }
}

void AutodiffActor::_eventPostUpdate(spe::IfMaster) {
    _getCurrentState().commit();
}

void AutodiffActor::_eventDraw1() {
    if (this->isDeactivated()) {
        return;
    }

    auto& winMgr = ccomp<spe::WindowManagerInterface>();
    auto& canvas = winMgr.getCanvas();

    const auto& self = _getCurrentState();

    hg::gr::CircleShape circle{32.f};
    circle.setOrigin(32.f, 32.f);
    circle.setFillColor(hg::gr::Color{self.color});
    circle.setPosition(self.x, self.y);
    canvas.draw(circle);
}

void AutodiffActor::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AutodiffActor, aSyncCtrl);
}

void AutodiffActor::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AutodiffActor, aSyncCtrl);
}

void AutodiffActor::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AutodiffActor, aSyncCtrl);
}

///////////////////////////////////////////////////////////////////////////
// MARK: ALTERNATING ACTOR                                               //
///////////////////////////////////////////////////////////////////////////

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AlternatingActor, (CREATE, UPDATE, DESTROY));

AlternatingActor::AlternatingActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, PRIORITY_ACTOR, "AlternatingActor", aSyncId} {}

void AlternatingActor::_didAttach(hg::QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);
    _enableAlternatingUpdates();
}

void AlternatingActor::init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex) {
    HG_HARD_ASSERT(isMasterObject());
    auto& self = _getCurrentState();
    self.x     = aX;
    self.y     = aY;
    self.color = aColor.toInt();
    self.index = aIndex;
}

void AlternatingActor::_eventUpdate1(spe::IfMaster) {
    auto& self          = _getCurrentState();
    int   stateDuration = 0;
    switch (_state) {
    case State::WAIT_LEFT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_RIGHT:
        self.x += SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    case State::WAIT_RIGHT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_LEFT:
        self.x -= SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    default:
        HG_UNREACHABLE("Invalid value for State ({}).", (int)_state);
    }

    _durationCounter += 1;
    if (_durationCounter >= stateDuration) {
        _state           = static_cast<State>(((int)_state + 1) % (int)State::STATE_COUNT);
        _durationCounter = 0;
    }
}

void AlternatingActor::_eventDraw1() {
    if (this->isDeactivated()) {
        return;
    }

    auto& winMgr = ccomp<spe::WindowManagerInterface>();
    auto& canvas = winMgr.getCanvas();

    const auto& self_curr = _getCurrentState();
    const auto& self_next = _getFollowingState();

    hg::gr::CircleShape circle{32.f};
    circle.setOrigin(32.f, 32.f);
    circle.setFillColor(hg::gr::Color{self_curr.color});
    circle.setPosition((self_curr.x + self_next.x) / 2.0, (self_curr.y + self_next.y) / 2.0);
    canvas.draw(circle);
}

void AlternatingActor::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AlternatingActor, aSyncCtrl);
}

void AlternatingActor::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AlternatingActor, aSyncCtrl);
}

void AlternatingActor::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AlternatingActor, aSyncCtrl);
}

///////////////////////////////////////////////////////////////////////////
// MARK: ALTERNATING AUTODIFF ACTOR                                      //
///////////////////////////////////////////////////////////////////////////

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AlternatingAutodiffActor, (CREATE, UPDATE, DESTROY));

AlternatingAutodiffActor::AlternatingAutodiffActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, PRIORITY_ACTOR, "AlternatingAutodiffActor", aSyncId} {
    _enableAlternatingUpdates();
}

void AlternatingAutodiffActor::_didAttach(hg::QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);

    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
}

void AlternatingAutodiffActor::init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex) {
    HG_HARD_ASSERT(isMasterObject());
    auto& self = _getCurrentState();
    self.x     = aX;
    self.y     = aY;
    self.color = aColor.toInt();
    self.index = aIndex;
}

void AlternatingAutodiffActor::_eventUpdate1(spe::IfMaster) {
    auto& self          = _getCurrentState();
    int   stateDuration = 0;
    switch (_state) {
    case State::WAIT_LEFT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_RIGHT:
        self.x += SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    case State::WAIT_RIGHT:
        stateDuration = STATE_DURATION_WAIT;
        break;

    case State::GO_LEFT:
        self.x -= SPEED;
        stateDuration = STATE_DURATION_GO;
        break;

    default:
        HG_UNREACHABLE("Invalid value for State ({}).", (int)_state);
    }

    _durationCounter += 1;
    if (_durationCounter >= stateDuration) {
        _state           = static_cast<State>(((int)_state + 1) % (int)State::STATE_COUNT);
        _durationCounter = 0;
    }
}

void AlternatingAutodiffActor::_eventPostUpdate(spe::IfMaster) {
    if (_didAlternatingUpdatesSync()) {
        _getCurrentState().commit();
    }
}

void AlternatingAutodiffActor::_eventDraw1() {
    if (this->isDeactivated()) {
        return;
    }

    auto& winMgr = ccomp<spe::WindowManagerInterface>();
    auto& canvas = winMgr.getCanvas();

    const auto& self_curr = _getCurrentState();
    const auto& self_next = _getFollowingState();

    hg::gr::CircleShape circle{32.f};
    circle.setOrigin(32.f, 32.f);
    circle.setFillColor(hg::gr::Color{self_curr.color});
    circle.setPosition((self_curr.x + self_next.x) / 2.0, (self_curr.y + self_next.y) / 2.0);
    canvas.draw(circle);
}

void AlternatingAutodiffActor::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AlternatingAutodiffActor, aSyncCtrl);
}

void AlternatingAutodiffActor::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AlternatingAutodiffActor, aSyncCtrl);
}

void AlternatingAutodiffActor::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AlternatingAutodiffActor, aSyncCtrl);
}

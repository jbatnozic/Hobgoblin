// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Graphics.hpp>
#include <Hobgoblin/Utility/Autopack.hpp>

#include "Engine.hpp"

#include <cstdint>
#include <iostream>
#include <ostream>

namespace spe = jbatnozic::spempe;
namespace hg  = jbatnozic::hobgoblin;

struct VisibleState {
    float         x     = 0.f;
    float         y     = 0.f;
    std::uint32_t color = 0xFF00FFFF;
    std::int8_t   index = -1;

    HG_ENABLE_AUTOPACK(VisibleState, x, y, color, index);
};

inline std::ostream& operator<<(std::ostream& aOS, const VisibleState& aVS) {
    return (aOS << (int)aVS.x);
}

// clang-format off
SPEMPE_DEFINE_AUTODIFF_STATE(AutodiffVisibleState,
    SPEMPE_MEMBER(float, x, 0.f),
    SPEMPE_MEMBER(float, y, 0.f),
    SPEMPE_MEMBER(std::uint32_t, color, 0xFF00FFFF),
    SPEMPE_MEMBER(std::int8_t, index, -1)
) {};
// clang-format on

inline std::ostream& operator<<(std::ostream& aOS, const AutodiffVisibleState& aVS) {
    return (aOS << (int)aVS.x);
}

///////////////////////////////////////////////////////////////////////////
// MARK: BASIC ACTOR                                                     //
///////////////////////////////////////////////////////////////////////////

class BasicActor : public spe::SynchronizedObject<VisibleState> {
public:
    BasicActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    void init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex);

private:
    enum class State {
        WAIT_LEFT,
        GO_RIGHT,
        WAIT_RIGHT,
        GO_LEFT,

        STATE_COUNT
    };

    static constexpr auto STATE_DURATION_WAIT = 30;
    static constexpr auto STATE_DURATION_GO   = 180;
    static constexpr auto SPEED               = 4.f;

    State _state           = State::WAIT_LEFT;
    int   _durationCounter = 0;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

///////////////////////////////////////////////////////////////////////////
// MARK: AUTODIFF ACTOR                                                  //
///////////////////////////////////////////////////////////////////////////

class AutodiffActor : public spe::SynchronizedObject<AutodiffVisibleState> {
public:
    AutodiffActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    void init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex);

private:
    enum class State {
        WAIT_LEFT,
        GO_RIGHT,
        WAIT_RIGHT,
        GO_LEFT,

        STATE_COUNT
    };

    static constexpr auto STATE_DURATION_WAIT = 30;
    static constexpr auto STATE_DURATION_GO   = 180;
    static constexpr auto SPEED               = 4.f;

    State _state           = State::WAIT_LEFT;
    int   _durationCounter = 0;

    void _didAttach(hg::QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventPostUpdate(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

///////////////////////////////////////////////////////////////////////////
// MARK: ALTERNATING ACTOR                                               //
///////////////////////////////////////////////////////////////////////////

class AlternatingActor : public spe::SynchronizedObject<VisibleState> {
public:
    AlternatingActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    void init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex);

private:
    enum class State {
        WAIT_LEFT,
        GO_RIGHT,
        WAIT_RIGHT,
        GO_LEFT,

        STATE_COUNT
    };

    static constexpr auto STATE_DURATION_WAIT = 30;
    static constexpr auto STATE_DURATION_GO   = 180;
    static constexpr auto SPEED               = 4.f;

    State _state           = State::WAIT_LEFT;
    int   _durationCounter = 0;

    void _didAttach(hg::QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

///////////////////////////////////////////////////////////////////////////
// MARK: ALTERNATING AUTODIFF ACTOR                                      //
///////////////////////////////////////////////////////////////////////////

class AlternatingAutodiffActor : public spe::SynchronizedObject<AutodiffVisibleState> {
public:
    AlternatingAutodiffActor(hg::QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    void init(float aX, float aY, hg::gr::Color aColor, std::int8_t aIndex);

private:
    enum class State {
        WAIT_LEFT,
        GO_RIGHT,
        WAIT_RIGHT,
        GO_LEFT,

        STATE_COUNT
    };

    static constexpr auto STATE_DURATION_WAIT = 30;
    static constexpr auto STATE_DURATION_GO   = 180;
    static constexpr auto SPEED               = 4.f;

    State _state           = State::WAIT_LEFT;
    int   _durationCounter = 0;

    void _didAttach(hg::QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventPostUpdate(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

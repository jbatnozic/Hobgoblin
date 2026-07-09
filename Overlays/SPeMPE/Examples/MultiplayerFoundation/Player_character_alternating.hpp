// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Engine.h"
#include "SPeMPE/GameObjectFramework/Sync_id.hpp"

#include <Hobgoblin/Utility/Autopack.hpp>

#include <cstdint>

struct AlternatingPlayerCharacter_VisibleState {
    float x = 0.f, y = 0.f;
    std::int32_t owningPlayerIndex = spe::PLAYER_INDEX_UNKNOWN;

    HG_ENABLE_AUTOPACK(AlternatingPlayerCharacter_VisibleState, x, y, owningPlayerIndex);
};

/**
 * Implementation of a synchronized object with alternating updates optimization enabled.
 * (the updates are sent every other frame, and then the clients interpolates)
 */
class AlternatingPlayerCharacter
    : public spe::SynchronizedObject<AlternatingPlayerCharacter_VisibleState>
{
public:
    AlternatingPlayerCharacter(QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    void init(int aOwningPlayerIndex, float aX, float aY);

private:
    void _eventUpdate1(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

// IMPORTANT: Since `spempe::SynchronizedObject` is a template and not a concrete class,
//            the declared superclass for directly-derived synchronized objects must be
//            `spempe::SynchronizedObjectBase`.
QAO_REGISTER_CLASS(AlternatingPlayerCharacter, Example_AlternatingPlayerCharacter) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setSuperclass<spe::SynchronizedObjectBase>();
}

// clang-format on

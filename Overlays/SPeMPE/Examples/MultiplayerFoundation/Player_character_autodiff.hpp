// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Engine.h"

#include <cstdint>

SPEMPE_DEFINE_AUTODIFF_STATE(AutodiffPlayerCharacter_VisibleState,
    SPEMPE_MEMBER(float, x, 0.f),
    SPEMPE_MEMBER(float, y, 0.f),
    SPEMPE_MEMBER(std::int32_t, owningPlayerIndex, spe::PLAYER_INDEX_UNKNOWN)
) {};

/**
 * Implementation of a synchronized object with autodiff state optimization enabled.
 * (members that didn't change are automatically detected and are not sent to clients).
 */
class AutodiffPlayerCharacter
    : public spe::SynchronizedObject<AutodiffPlayerCharacter_VisibleState>
{
public:
    AutodiffPlayerCharacter(QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    void init(int aOwningPlayerIndex, float aX, float aY);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventPostUpdate(spe::IfMaster) override;
    void _eventDraw1() override; 

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

// IMPORTANT: Since `spempe::SynchronizedObject` is a template and not a concrete class,
//            the declared superclass for directly-derived synchronized objects must be
//            `spempe::SynchronizedObjectBase`.
QAO_REGISTER_CLASS(AutodiffPlayerCharacter, Example_AutodiffPlayerCharacter) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setSuperclass<spe::SynchronizedObjectBase>();
}

// clang-format on

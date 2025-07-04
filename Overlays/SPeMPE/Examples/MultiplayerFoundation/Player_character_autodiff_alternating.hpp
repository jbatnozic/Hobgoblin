// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Engine.h"
#include "SPeMPE/GameObjectFramework/Sync_id.hpp"

#include <cstdint>

SPEMPE_DEFINE_AUTODIFF_STATE(AutodiffAlternatingPlayerCharacter_VisibleState,
    SPEMPE_MEMBER(float, x, 0.f),
    SPEMPE_MEMBER(float, y, 0.f),
    SPEMPE_MEMBER(std::int32_t, owningPlayerIndex, spe::PLAYER_INDEX_UNKNOWN)
) {};

class AutodiffAlternatingPlayerCharacter
    : public spe::SynchronizedObject<AutodiffAlternatingPlayerCharacter_VisibleState>
{
public:
    AutodiffAlternatingPlayerCharacter(QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

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

// clang-format on

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

namespace cinnabar {

/*
SPEMPE_DEFINE_AUTODIFF_STATE(ShipPiece_VisibleState,
    SPEMPE_MEMBER(?, spriteId, SPRITEID_NONE),
    SPEMPE_MEMBER(?, parentSyncId, ?),
    SPEMPE_MEMBER(?, parentXOffset, 0),
    SPEMPE_MEMBER(?, parentYOffset, 0),
) {}
*/

struct Ship_VisibleState {};

class Ship /* : public spe::SynchronizedObject<Ship_VisibleState> */
    : public spe::StateObject {
public:
    Ship(QAO_InstGuard aInstGuard);

    void init(double aX, double aY);

private:
    void _eventUpdate1() override;
    void _eventDraw1() override;
};

} // namespace cinnabar

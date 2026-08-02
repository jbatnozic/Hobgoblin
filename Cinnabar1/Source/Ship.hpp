// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>

#include <memory>
#include <span>

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

    //! \param aShapeCenter center of the shape (in absolute world coordinates)
    //! \param aShapeVertices positions of shape vertices (in absolute world coordinates)
    void drawGridOverShape(hg::math::Vector2d            aShapeCenter,
                           std::span<hg::math::Vector2d> aShapeVertices,
                           uwga::Canvas&                 aCanvas);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1() override;
    void _eventDraw1() override;

    hg::math::Vector2d _position              = {};
    hg::math::Vector2f _mousePosInLocalCoords = {};
    hg::math::AngleF   _rotation              = hg::math::AngleF::zero();
    bool               _drawGrid              = false;

    std::unique_ptr<uwga::Transform> _transform;
    std::unique_ptr<uwga::Transform> _transformInverse;
};

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <InteriorWorld/Interior_world.hpp>
#include <Graph_of_attachables.hpp>
#include <Ship_attachable.hpp>
#include <Poly_shape.hpp>
#include <Projected_cell_positions.hpp>
#include <QAOMessages/Downcast_to_ship_controller.hpp>

#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>

#include <memory>
#include <span>

namespace cinnabar {

SPEMPE_DEFINE_AUTODIFF_STATE(ShipController_VisibleState,
    SPEMPE_MEMBER(double, positionX, 0.0),
    SPEMPE_MEMBER(double, positionY, 0.0),
    SPEMPE_MEMBER(float, rotation, 0.0)
    // Below are ideas for members of a "ship section" object
    // SPEMPE_MEMBER(?, spriteId, SPRITEID_NONE),
    // SPEMPE_MEMBER(?, parentSyncId, ?),
    // SPEMPE_MEMBER(?, parentXOffset, 0),
    // SPEMPE_MEMBER(?, parentYOffset, 0),
) {};

class AttachableGhost;
struct ShipController_MasterData;

//! \brief The main controller of a complex, modular ship or station with a player-interactible
//!        exterior and interior.
class ShipController
    : public spe::SynchronizedObject<ShipController_VisibleState,
                                     SPEMPE_RSDATA_HEAP(ShipController_MasterData, void)> {
public:
    ShipController(QAO_InstGuard aInstGuard, spe::SyncId aSyncId);

    void init(ShipAttachable& aInitialShipAttachable);

    //! \param aAnchorOffset X/Y offset of the attachable's anchor IN THE SHIP CONTROLLER'S COORDINATE SYSTEM
    //! \param aRotationOffset angle difference
    void attach(ShipAttachable&    aShipAttachable,
                hg::math::Vector2f aAnchorOffset,
                hg::math::AngleF   aRotationOffset);

    void attach(AttachableGhost& aAttachableGhost);

    

    void drawGridOverShape(const PolyShape& aShape, uwga::Canvas& aCanvas);

    //! calculates projected cell positions of a poly shape in the ship's interior world
    //! \param aShape[in] 
    //! \param aProjectedCellPositions[out] 
    void projectCellPositions(const PolyShape& aShape, ProjectedCellPositions& aProjectedCellPositions);

    // QAO Message Handlers

    void msgDowncastToShipController(DowncastToShipController::PayloadPtr aPtr, bool /* aConst */);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;

    hg::math::Vector2d _position              = {};
    hg::math::Vector2f _mousePosInLocalCoords = {};
    hg::math::AngleF   _rotation              = hg::math::AngleF::zero();
    bool               _drawGrid              = false;
};

// MARK: MasterData

struct ShipController_MasterData {
    GraphOfAttachables graphOfAttachables;

    InteriorWorld interiorWorld;

    std::unique_ptr<uwga::Transform> transform;
    std::unique_ptr<uwga::Transform> transformInverse;

    ShipController_MasterData();
};

// MARK: Register class

QAO_REGISTER_CLASS(ShipController, cinnabar_ShipController) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::SynchronizedObjectBase>();
    klass.setMessageHandler<C, DowncastToShipController, &C::msgDowncastToShipController>();
}

} // namespace cinnabar

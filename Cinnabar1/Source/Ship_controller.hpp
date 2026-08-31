// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Attachment_evaluation.hpp>
#include <Graph_of_attachables.hpp>
#include <InteriorWorld/Interior_world.hpp>
#include <Poly_shape.hpp>
#include <Projected_cell_positions.hpp>
#include <QAOMessages/Downcast_to_ship_controller.hpp>
#include <Ship_attachable.hpp>

#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>

#include <memory>
#include <variant>

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
){};

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

    //! \brief evaluate a potential attachment of a ghost of an attachable carrying an IW slice
    AttachmentEvaluation evalAttachment(const AttachableGhost& aGhost);

    //! \brief evaluate a potential attachment of a ghost of an attachable carrying no IW slice
    AttachmentEvaluation evalAttachment(const AttachableGhost&        aGhost,
                                        const ProjectedCellPositions& aProjectedCellPositions);

    //! ignore the attachable's actual position and attach it as if it were at (aAnchorOffset,
    //! aRotationOffset)
    //! - though it will get moved there!
    //! \param aAnchorOffset X/Y offset of the attachable's anchor IN THE SHIP CONTROLLER'S COORDINATE
    //! SYSTEM
    //! \param aRotationOffset angle difference
    void attach(ShipAttachable&    aShipAttachable,
                hg::math::Vector2f aAnchorOffset,
                hg::math::AngleF   aRotationOffset);

    void attach(AttachableGhost& aAttachableGhost);

    void drawGridOverShape(const PolyShape& aShape, uwga::Canvas& aCanvas) const;

    void drawGridOverProjection(const ProjectedCellPositions& aProjectedCellPositions,
                                uwga::Canvas&                 aCanvas) const;

    void drawGridOverGhost(const AttachableGhost& aAttachableGhost, uwga::Canvas& aCanvas) const;

    //! calculates projected cell positions of a poly shape in the ship's interior world
    //! \param aShape[in]
    //! \param aProjectedCellPositions[out]
    void projectCellPositions(const PolyShape& aShape, ProjectedCellPositions& aProjectedCellPositions);

    const ShipAttachable* getAttachableWithIndex(std::int16_t aIndex) const;

    // QAO Message Handlers

    void msgDowncastToShipController(DowncastToShipController::PayloadPtr aPtr, bool /* aConst */);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;

    //! Returns a non-INVALID rotation, if able, and a rotation hint otherwise.
    static std::variant<RelativeIWSliceOrientation, hg::math::AngleF> _checkIWSliceOrientation(
        const ShipAttachable::InteriorWorldSliceData& aSlice,
        hg::math::AngleF                              aRelativeRotation);

    static std::variant<hg::math::Vector2i, hg::math::Vector2f> _checkIWTopLeftCellMapping(
        const ShipAttachable::InteriorWorldSliceData& aSlice,
        hg::math::Vector2f                            aAnchorDiff,
        RelativeIWSliceOrientation                    aOrientation);

    char _checkSliceDataToIWIntegration_rot000(
        const ShipAttachable::InteriorWorldSliceData&    aSlice,
        hg::math::Vector2pz                              aStartingCorner,
        std::vector<AttachmentEvaluation::BondStrength>& aOutBonds);
    char _checkSliceDataToIWIntegration_rot090(
        const ShipAttachable::InteriorWorldSliceData&    aSlice,
        hg::math::Vector2pz                              aStartingCorner,
        std::vector<AttachmentEvaluation::BondStrength>& aOutBonds);
    char _checkSliceDataToIWIntegration_rot180(
        const ShipAttachable::InteriorWorldSliceData&    aSlice,
        hg::math::Vector2pz                              aStartingCorner,
        std::vector<AttachmentEvaluation::BondStrength>& aOutBonds);
    char _checkSliceDataToIWIntegration_rot270(
        const ShipAttachable::InteriorWorldSliceData&    aSlice,
        hg::math::Vector2pz                              aStartingCorner,
        std::vector<AttachmentEvaluation::BondStrength>& aOutBonds);

    void _copySliceDataToInteriorWorld_rot000(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner);
    void _copySliceDataToInteriorWorld_rot090(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner);
    void _copySliceDataToInteriorWorld_rot180(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner);
    void _copySliceDataToInteriorWorld_rot270(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner);

    hg::math::Vector2d _position              = {};
    hg::math::Vector2f _mousePosInLocalCoords = {};
    hg::math::AngleF   _rotation              = hg::math::AngleF::zero();
    bool               _drawGrid              = false;
};

// MARK: MasterData

struct ShipController_MasterData {
    GraphOfAttachables graphOfAttachables;

    InteriorWorld interiorWorld;

    std::unique_ptr<uwga::Transform> transformGlobalToShip;
    std::unique_ptr<uwga::Transform> transformShipToGlobal;

    ShipController_MasterData();
};

// MARK: Register class

QAO_REGISTER_CLASS(ShipController, cinnabar_ShipController) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::SynchronizedObjectBase>();
    klass.setMessageHandler<C, DowncastToShipController, &C::msgDowncastToShipController>();
}

} // namespace cinnabar

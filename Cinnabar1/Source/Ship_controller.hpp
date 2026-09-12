// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Attachment_evaluation.hpp>
#include <Cell_footprint.hpp>
#include <Graph_of_attachables.hpp>
#include <InteriorWorld/Interior_world.hpp>
#include <Poly_shape.hpp>
#include <QAOMessages/Downcast_to_ship_controller.hpp>
#include <Ship_attachable.hpp>

#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>

#include <memory>
#include <variant>

namespace cinnabar {

// clang-format off
SPEMPE_DEFINE_AUTODIFF_STATE(ShipController_VisibleState,
    SPEMPE_MEMBER(double, x, 0.0),
    SPEMPE_MEMBER(double, y, 0.0),
    SPEMPE_MEMBER(hg::math::AngleF, rotation, hg::math::AngleF::zero())
    // Below are ideas for members of a "ship section" object
    // SPEMPE_MEMBER(?, spriteId, SPRITEID_NONE),
    // SPEMPE_MEMBER(?, parentSyncId, ?),
    // SPEMPE_MEMBER(?, parentXOffset, 0),
    // SPEMPE_MEMBER(?, parentYOffset, 0),
) {
    hg::math::Vector2d getPosition() const {
        return {x, y};
    }

    void setPosition(hg::math::Vector2d aPosition) {
        x = aPosition.x, y = aPosition.y;
    }
};
// clang-format on

class AttachableGhost;
struct ShipController_MasterData;

//! \brief The main controller of a complex, modular ship or station with a player-interactible
//!        exterior and interior.
class ShipController
    : public spe::SynchronizedObject<ShipController_VisibleState,
                                     SPEMPE_RSDATA_HEAP(ShipController_MasterData, void)> {
public:
    //! Factory method to create a master object.
    static QAO_Handle<ShipController> createMaster(QAO_RuntimeRef  aRuntime,
                                                   ShipAttachable& aInitialShipAttachable);

    //! Factory method to create a dummy object.
    static QAO_Handle<ShipController> createDummy(QAO_RuntimeRef aRuntime, spe::SyncId aSyncId);

    void init(ShipAttachable& aInitialShipAttachable);

    ///////////////////////////////////////////////////////////////////////////
    // ATTACHING                                                             //
    ///////////////////////////////////////////////////////////////////////////

    //! calculates projected cell positions of a poly shape in the ship's interior world
    //! \param aShape[in]
    //! \param aCellFootprint[out]
    void calcFootprint(const PolyShape& aShape, CellFootprint& aCellFootprint);

    //! \brief evaluate a potential attachment of a ghost of an attachable carrying an IW slice
    AttachmentEvaluation evalAttachment(const AttachableGhost& aGhost);

    //! \brief evaluate a potential attachment of a ghost of an attachable carrying no IW slice
    AttachmentEvaluation evalAttachment(const AttachableGhost& aGhost,
                                        const CellFootprint&   aCellFootprint);

    //! TODO(add desc.)
    void attach(AttachableGhost& aGhost, const AttachmentEvaluation& aAttachmentEval);

    //! TODO(add desc.)
    void attach(AttachableGhost&            aGhost,
                const CellFootprint&        aCellFootprint,
                const AttachmentEvaluation& aAttachmentEval);

    ///////////////////////////////////////////////////////////////////////////
    // DETACHING                                                             //
    ///////////////////////////////////////////////////////////////////////////

    void detach(ShipAttachable& aAttachable);

    ///////////////////////////////////////////////////////////////////////////
    // UTILITY                                                               //
    ///////////////////////////////////////////////////////////////////////////

    hg::math::Vector2d getAnchor() const;

    hg::math::AngleF getRotation() const;

    const ShipAttachable* getAttachableWithIndex(std::int16_t aIndex) const;

    void drawGridOverShape(const PolyShape& aShape, uwga::Canvas& aCanvas) const;

    void drawGridOverProjection(const CellFootprint& aCellFootprint, uwga::Canvas& aCanvas) const;

    void drawGridOverGhost(const AttachableGhost& aAttachableGhost, uwga::Canvas& aCanvas) const;

    ///////////////////////////////////////////////////////////////////////////
    // QAO Message Handlers                                                  //
    ///////////////////////////////////////////////////////////////////////////

    void msgDowncastToShipController(DowncastToShipController::PayloadPtr aPtr, bool /* aConst */);

private:
    BEFRIEND_QAO_CREATE;

    //! Private constructor for the master object.
    ShipController(QAO_InstGuard aInstGuard);

    //! Private constructor for the dummy object.
    ShipController(QAO_InstGuard aInstGuard, spe::SyncId aSyncId);

    void _didAttach(QAO_Runtime& aRuntime) override;
    void _willDetach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventPostUpdate(spe::IfMaster) override;
    void _eventDraw1() override;

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
                                              hg::math::Vector2pz aStartingCorner,
                                              std::int16_t        aAttachableId);
    void _copySliceDataToInteriorWorld_rot090(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner,
                                              std::int16_t        aAttachableId);
    void _copySliceDataToInteriorWorld_rot180(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner,
                                              std::int16_t        aAttachableId);
    void _copySliceDataToInteriorWorld_rot270(const ShipAttachable::InteriorWorldSliceData& aSlice,
                                              hg::math::Vector2pz aStartingCorner,
                                              std::int16_t        aAttachableId);

    void _createConstraintsUponAttach(AttachableGhost& aGhost,
                                      std::int16_t     aAttachableId,
                                      const std::vector<AttachmentEvaluation::BondStrength>& aBonds);

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;

    std::unique_ptr<uwga::Transform> _transformGlobalToShip;
    std::unique_ptr<uwga::Transform> _transformShipToGlobal;

    // Temporarily here
    hg::math::Vector2f _mousePosInLocalCoords = {};
    bool               _drawGrid              = false;
};

// MARK: MasterData

struct ShipController_MasterData {
    GraphOfAttachables graphOfAttachables;

    InteriorWorld interiorWorld;
};

// MARK: Register class

QAO_REGISTER_CLASS(ShipController, cinnabar_ShipController) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::SynchronizedObjectBase>();
    klass.setMessageHandler<C, DowncastToShipController, &C::msgDowncastToShipController>();
}

} // namespace cinnabar

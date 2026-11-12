// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/QAOMessages/Handle_pncs_event.hpp>
#include <CBR/Ship/Ship_attachable.hpp>
#include <CBR/Utility/Cell_footprint.hpp>
#include <CBR/Utility/Poly_shape.hpp>
#include <Engine.hpp>

#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/Utility/Compact_int.hpp>
#include <Hobgoblin/Utility/Grids.hpp>

#include <cstdint>

namespace cinnabar {

class ShipController;

// clang-format off
SPEMPE_DEFINE_AUTODIFF_STATE(AttachableGhost_VisibleState,
    SPEMPE_MEMBER(float, xOffsetRelativeToShipController, 0.0),
    SPEMPE_MEMBER(float, yOffsetRelativeToShipController, 0.0),
    SPEMPE_MEMBER(hg::math::AngleF, angleOffsetRelativeToShipController, hg::math::AngleF::zero()),
    SPEMPE_MEMBER(std::int8_t, holdingPlayerIndex, spe::PLAYER_INDEX_UNKNOWN)
) {
    hg::math::Vector2d getXYOffset() const {
        return {xOffsetRelativeToShipController, yOffsetRelativeToShipController};
    }

    void setXYOffset(hg::math::Vector2d aOffset) {
        xOffsetRelativeToShipController = aOffset.x, yOffsetRelativeToShipController = aOffset.y;
    }
};
// clang-format on

class AttachableGhost : public spe::SynchronizedObject<AttachableGhost_VisibleState> {
public:
    //! Factory method to create a master object.
    static QAO_Handle<AttachableGhost> createMaster(QAO_RuntimeRef aRuntime,
                                                    QAO_GenericId  aAssociatedShipController,
                                                    QAO_GenericId  aAssociatedAttachableId,
                                                    int            aOnBehalfOfPlayerIndex);

    //! Factory method to create a dummy object.
    static QAO_Handle<AttachableGhost> createDummy(QAO_RuntimeRef aRuntime,
                                                   spe::SyncId    aSyncId,
                                                   spe::SyncId    aAssociatedShipControllerSyncId,
                                                   spe::SyncId    aAssociatedShipAttachableSyncId,
                                                   int            aOnBehalfOfPlayerIndex);

    void _initMaster(QAO_GenericId aAssociatedShipController,
                     QAO_GenericId aAssociatedAttachableId,
                     int           aOnBehalfOfPlayerIndex);

    const ShipAttachable& getAssociatedAttachable() const;

    ShipAttachable& getAssociatedAttachable();

    const CellFootprint& getCellFootprint() const;

    const PolyShape& getPolyShape() const {
        return _shape;
    }

    void tryHold(std::int8_t aHoldingPlayerIndex) {
        HG_ASSERT(isMasterObject());
        auto& self = _getCurrentState();
        if (self.holdingPlayerIndex == spe::PLAYER_INDEX_UNKNOWN) {
            self.holdingPlayerIndex = aHoldingPlayerIndex;
        }
    }

    void tryDrop(std::int8_t aHoldingPlayerIndex) {
        HG_ASSERT(isMasterObject());
        auto& self = _getCurrentState();
        if (self.holdingPlayerIndex == aHoldingPlayerIndex) {
            self.holdingPlayerIndex = spe::PLAYER_INDEX_UNKNOWN;
        }
    }

    void tryAdjustPos(std::int8_t aHoldingPlayerIndex, float aX, float aY, hg::math::AngleF aAngle) {
        HG_ASSERT(isMasterObject());
        auto& self = _getCurrentState();
        if (self.holdingPlayerIndex == aHoldingPlayerIndex) {
            self.xOffsetRelativeToShipController     = aX;
            self.yOffsetRelativeToShipController     = aY;
            self.angleOffsetRelativeToShipController = aAngle;
        }
    }

    // QAO Message Handlers

    void msgHandlePNCSEvent(HandlePNCSEvent::PayloadPtr aPayload, bool /* aConst */);

private:
    BEFRIEND_QAO_CREATE_FOR_FACTORIES;

    AttachableGhost(QAO_InstGuard aInstGuard, spe::SyncId aSyncId = spe::SYNC_ID_NEW);

    // Once-init members (never reassigned after initialization)

    QAO_GenericId   _controllerId  = nullptr;
    ShipController* _controllerPtr = nullptr;

    QAO_GenericId   _attachableId  = nullptr;
    ShipAttachable* _attachablePtr = nullptr;

    bool _inHeadlessContext = false;

    // Malleable members

    PolyShape     _shape;
    CellFootprint _cellFootprint;

    hg::math::Vector2d _cursorOffset = {};

    enum class DummyState : std::uint8_t {
        PICKING_UP,          // HOLDING_AND_SENDING (provisional)
        HOLDING_AND_SENDING, // Sending local position to the server
        DROPPING,            // RECEIVING (provisional)
        RECEIVING            // Getting the actual position streamed from the server
    };

    DummyState _dummyState = DummyState::RECEIVING;

    bool        _leftClicked          = false;
    std::int8_t _leftClickPlayerIndex = spe::PLAYER_INDEX_UNKNOWN;

    // bool _held = false;

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventBeginUpdate() override;
    void _eventUpdate1(spe::IfDummy) override;
    void _eventUpdate1(spe::IfMaster) override;
    void _eventPostUpdate(spe::IfMaster) override;
    void _eventDraw1() override;

    ShipAttachable* _findAttachableById(QAO_GenericId aAttachableId) const;
    ShipController* _findControllerById(QAO_GenericId aAttachableId) const;

    void _initAssociatedObjectRefs(QAO_GenericId aAssociatedShipController,
                                   QAO_GenericId aAssociatedAttachableId);

    bool _isBeingHeld() const {
        return isMasterObject() ? (_getCurrentState().holdingPlayerIndex != spe::PLAYER_INDEX_UNKNOWN)
                                : (_dummyState == DummyState::HOLDING_AND_SENDING ||
                                   _dummyState == DummyState::PICKING_UP);
    }

    bool _attachIfPositionIsRight();

    void _dummyState_changeTo_PICKING_UP(bool);
    void _dummyState_changeTo_HOLDING_AND_SENDING(bool);
    void _dummyState_changeTo_DROPPING(bool);
    void _dummyState_changeTo_RECEIVING(bool);

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

QAO_REGISTER_CLASS(AttachableGhost, cinnabar_AttachableGhost) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
    klass.setMessageHandler<C, HandlePNCSEvent, &C::msgHandlePNCSEvent>();
}

void AskServerToCreateAttachableGhost(spe::NetworkingManager& aNetworkingManager,
                                      spe::SyncId             aAssociatedShipControllerSyncId,
                                      spe::SyncId             aAssociatedShipAttachableSyncId,
                                      int                     aOnBehalfOfPlayerIndex);

} // namespace cinnabar

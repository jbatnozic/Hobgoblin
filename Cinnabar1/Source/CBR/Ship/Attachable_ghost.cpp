// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Attachable_ghost.hpp"

#include "Ship_controller.hpp"
#include <CBR/Managers/UserInterface/Interactivity_manager.hpp>
#include <CBR/QAOMessages/Downcast_to_ship_attachable.hpp>

#include <Hobgoblin/ChipmunkPhysics.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <span>

namespace cinnabar {

namespace {
// clang-format off
RN_DEFINE_RPC(AskServerToHoldAttachableGhost,
    RN_ARGS(
        spe::SyncId ,aGhostSyncId,
        std::int8_t ,aPlayerIndex
    )
) {
    RN_NODE_IN_HANDLER().illegalIfClient(__func__);

    RN_NODE_IN_HANDLER().callIfServer([=](hg::RN_ServerInterface& aServer) {
        auto  rc         = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aServer);

        auto* ghostPtr = rc.netwMgr.mapSyncIdToObject(aGhostSyncId);
        if (ghostPtr == nullptr) {
            return;
        }

        static_cast<AttachableGhost*>(ghostPtr)->tryHold(aPlayerIndex);
    });
}

RN_DEFINE_RPC(AskServerToDropAttachableGhost,
    RN_ARGS(
        spe::SyncId ,aGhostSyncId,
        std::int8_t ,aPlayerIndex
    )
) {
    RN_NODE_IN_HANDLER().illegalIfClient(__func__);

    RN_NODE_IN_HANDLER().callIfServer([=](hg::RN_ServerInterface& aServer) {
        auto rc = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aServer);

        auto* ghostPtr = rc.netwMgr.mapSyncIdToObject(aGhostSyncId);
        if (ghostPtr == nullptr) {
            return;
        }

        static_cast<AttachableGhost*>(ghostPtr)->tryDrop(aPlayerIndex);
    });
}

RN_DEFINE_RPC(AskServerToAdjustAttachableGhostPosition,
    RN_ARGS(
        spe::SyncId      ,aGhostSyncId,
        std::int8_t      ,aPlayerIndex,
        float            ,aXOffset,
        float            ,aYOffset,
        hg::math::AngleF ,aAngleOffset
    )
) {
    RN_NODE_IN_HANDLER().illegalIfClient(__func__);

    RN_NODE_IN_HANDLER().callIfServer([=](hg::RN_ServerInterface& aServer) {
        auto  rc = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aServer);

        auto* ghostPtr = rc.netwMgr.mapSyncIdToObject(aGhostSyncId);
        if (ghostPtr == nullptr) {
            return;
        }

        static_cast<AttachableGhost*>(ghostPtr)->tryAdjustPos(
            aPlayerIndex, aXOffset, aYOffset, aAngleOffset);
    });
}
// clang-format on
} // namespace

AttachableGhost::AttachableGhost(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES - 1, // TODO
                   QAO_STATIC_NAME("cinnabar::AttachableGhost"),
                   aSyncId} //
{
    HG_LOG_WARN(LOG_ID, "Created AttachableGhost with SyncId {}", aSyncId);
}

QAO_Handle<AttachableGhost> AttachableGhost::createMaster(QAO_RuntimeRef aRuntime,
                                                          QAO_GenericId  aAssociatedShipController,
                                                          QAO_GenericId  aAssociatedAttachableId,
                                                          int            aOnBehalfOfPlayerIndex) //
{
    auto handle = QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY<AttachableGhost>(aRuntime, spe::SYNC_ID_NEW);
    handle->_initMaster(aAssociatedShipController, aAssociatedAttachableId, aOnBehalfOfPlayerIndex);
    return handle;
}

QAO_Handle<AttachableGhost> AttachableGhost::createDummy(QAO_RuntimeRef aRuntime,
                                                         spe::SyncId    aSyncId,
                                                         spe::SyncId    aAssociatedShipControllerSyncId,
                                                         spe::SyncId    aAssociatedShipAttachableSyncId,
                                                         int            aOnBehalfOfPlayerIndex) //
{
    auto handle = QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY<AttachableGhost>(aRuntime, aSyncId);

    const auto& netMgr = handle->ccomp<MNetworking>();

    auto* shipController = netMgr.mapSyncIdToObject(aAssociatedShipControllerSyncId);
    HG_HARD_ASSERT(shipController != nullptr);

    auto* shipAttachable = netMgr.mapSyncIdToObject(aAssociatedShipAttachableSyncId);
    HG_HARD_ASSERT(shipAttachable != nullptr);

    handle->_initAssociatedObjectRefs(shipController->getId(), shipAttachable->getId());

    handle->_shape = handle->_attachablePtr->getPolyShape();

    return handle;
}

void AttachableGhost::_initMaster(QAO_GenericId aAssociatedShipController,
                                  QAO_GenericId aAssociatedAttachableId,
                                  int           aOnBehalfOfPlayerIndex) {
    _initAssociatedObjectRefs(aAssociatedShipController, aAssociatedAttachableId);

    _shape = _attachablePtr->getPolyShape();

    // _held = true;

    // const auto input       = ccomp<MWindow>().getInput();
    // const auto mousePos    = input.getViewRelativeMousePos();
    // const auto mousePosRel = mousePos - _shape.getAnchor();

    // _cursorOffset = mousePosRel;
    _getCurrentState().angleOffsetRelativeToShipController =
        _controllerPtr->getCurrentVisibleState().rotation.shortestDistanceTo(
            _attachablePtr->getPolyShape().getRotation());

    tryHold(aOnBehalfOfPlayerIndex);
}

const ShipAttachable& AttachableGhost::getAssociatedAttachable() const {
    return *_attachablePtr;
}

ShipAttachable& AttachableGhost::getAssociatedAttachable() {
    return *_attachablePtr;
}

const CellFootprint& AttachableGhost::getCellFootprint() const {
    return _cellFootprint;
}

void AttachableGhost::msgHandlePNCSEvent(HandlePNCSEvent::PayloadPtr aPayload, bool /* aConst */) {
    HG_ASSERT(aPayload != nullptr);
    if (aPayload->mbRightDown && isMasterObject()) {
        QAO_Destroy(*this);
        return;
    }
    if (aPayload->mbLeftDown) {
        _leftClicked          = true;
        _leftClickPlayerIndex = static_cast<std::uint8_t>(aPayload->localPlayerIndex);
    }
}

void AttachableGhost::_didAttach(QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);
    _inHeadlessContext = ctx().isHeadless();

    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
}

void AttachableGhost::_eventBeginUpdate() {
    if (!_inHeadlessContext) {
        _leftClicked = false;

        ccomp<MInteractivity>().pushClickableObject(
            this->getId(),
            _isBeingHeld() ? -9999 : 0,
            0,
            /* quick check */
            [this](hg::math::Vector2d aMouseWorldPos) -> bool {
                return _isBeingHeld() || ((aMouseWorldPos - _shape.getAnchor()).lengthSquared() <=
                                          _shape.getDistanceToFarthestRawVertexSquared());
            },
            /* full check */
            [this](hg::math::Vector2d aMouseWorldPos) -> bool {
                return _isBeingHeld() ||
                       _shape.intersectsWithPointRel(aMouseWorldPos - _shape.getAnchor());
            });
    }
}

// MARK: QAO Events

void AttachableGhost::_eventUpdate1(spe::IfDummy) {
    if (isDeactivated()) {
        return;
    }

    auto& self = _getCurrentState();

    const auto localPlayerIndex = ccomp<MLobbyBackend>().getLocalPlayerIndex();

    // Check if a state change is needed
    switch (_dummyState) {
    case DummyState::PICKING_UP:
        if (_leftClicked) {
            _dummyState_changeTo_DROPPING(true);
        } else if (localPlayerIndex < 0) {
            _dummyState_changeTo_RECEIVING(true);
        } else if (self.holdingPlayerIndex == localPlayerIndex) {
            _dummyState_changeTo_HOLDING_AND_SENDING(false);
        } else if (self.holdingPlayerIndex != -1 && self.holdingPlayerIndex != localPlayerIndex) {
            _dummyState_changeTo_RECEIVING(false);
        } else if (self.holdingPlayerIndex == -1) {
            // Do nothing
        } else {
            // TODO: provisional state timeout
        }
        break;

    case DummyState::HOLDING_AND_SENDING:
        if (_leftClicked) {
            _dummyState_changeTo_DROPPING(true);
        } else if (localPlayerIndex < 0) {
            _dummyState_changeTo_RECEIVING(true);
        } else if (self.holdingPlayerIndex == localPlayerIndex) {
            // Do nothing
        } else if (self.holdingPlayerIndex != -1 && self.holdingPlayerIndex != localPlayerIndex) {
            _dummyState_changeTo_RECEIVING(false);
        } else if (self.holdingPlayerIndex == -1) {
            _dummyState_changeTo_RECEIVING(false);
        }
        break;

    case DummyState::DROPPING:
        if (_leftClicked && localPlayerIndex != -1 && self.holdingPlayerIndex == -1) {
            _dummyState_changeTo_PICKING_UP(true);
        } else if (localPlayerIndex < 0) {
            _dummyState_changeTo_RECEIVING(false);
        } else if (self.holdingPlayerIndex == localPlayerIndex) {
            // Do nothing
        } else if (self.holdingPlayerIndex != -1 && self.holdingPlayerIndex != localPlayerIndex) {
            _dummyState_changeTo_RECEIVING(false);
        } else if (self.holdingPlayerIndex == -1) {
            _dummyState_changeTo_RECEIVING(false);
        } else {
            // TODO: provisional state timeout
        }
        break;

    case DummyState::RECEIVING:
        if (_leftClicked && localPlayerIndex != -1 && self.holdingPlayerIndex == -1) {
            _dummyState_changeTo_PICKING_UP(true);
        } else if (localPlayerIndex < 0) {
            // Do nothing
        } else if (self.holdingPlayerIndex == localPlayerIndex) {
            _dummyState_changeTo_HOLDING_AND_SENDING(false);
        } /* else if (self.holdingPlayerIndex != -1 && self.holdingPlayerIndex != localPlayerIndex) {
            // Do nothing
        } else if (self.holdingPlayerIndex == -1) {
            // Do nothing
        }
        */
        break;

    default:
        HG_UNREACHABLE("Invalid value for DummyState ({}).", (int)_dummyState);
    }

    // Execute current state
    switch (_dummyState) {
    case DummyState::PICKING_UP:
    case DummyState::HOLDING_AND_SENDING:
        {
            const auto& winMgr   = ccomp<MWindow>();
            const auto  input    = winMgr.getInput();
            const auto  mousePos = input.getViewRelativeMousePos();

            // Adjust _shape

            _shape.setAnchor(mousePos - _cursorOffset);
            _shape.setRotation(_shape.getRotation() +
                               hg::math::AngleF::fromDegrees(input.getVerticalMouseWheelScroll()));

            if (!_controllerPtr->isDeactivated()) {
                // Adjust current state

                const auto& controllerVS = _controllerPtr->getCurrentVisibleState();
                const auto  controllerAnchorOffset =
                    hg::math::RotateVector(_shape.getAnchor() - controllerVS.getPosition(),
                                           -controllerVS.rotation);

                self.xOffsetRelativeToShipController = controllerAnchorOffset.x;
                self.yOffsetRelativeToShipController = controllerAnchorOffset.y;

                self.angleOffsetRelativeToShipController =
                    controllerVS.rotation.shortestDistanceTo(_shape.getRotation());

                // Send to server

                Compose_AskServerToAdjustAttachableGhostPosition(
                    ccomp<MNetworking>().getNode(),
                    RN_COMPOSE_FOR_ALL,
                    this->getSyncId(),
                    localPlayerIndex,
                    self.xOffsetRelativeToShipController,
                    self.yOffsetRelativeToShipController,
                    self.angleOffsetRelativeToShipController);
            }
        }
        break;

    case DummyState::DROPPING:
    case DummyState::RECEIVING:
        {
            if (!_controllerPtr->isDeactivated()) {
                const auto& controllerVS = _controllerPtr->getCurrentVisibleState();
                const auto  rotatedOffset =
                    hg::math::RotateVector(hg::math::Vector2d{self.xOffsetRelativeToShipController,
                                                              self.yOffsetRelativeToShipController},
                                           controllerVS.rotation);

                _shape.setAnchor(controllerVS.getPosition() + rotatedOffset);
                _shape.setRotation(controllerVS.rotation + self.angleOffsetRelativeToShipController);
            } else {
                _shape.setAnchor({});
                _shape.setRotation(hg::math::AngleF::zero());
            }
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for DummyState ({}).", (int)_dummyState);
    }

    _shape.recalcRel();
    if (!_controllerPtr->isDeactivated()) {
        _controllerPtr->calcFootprint(_shape, _cellFootprint);
    }
}

void AttachableGhost::_eventUpdate1(spe::IfMaster) {
    auto* runtime = getRuntime();
    if (runtime->find(_controllerId).isNull() || runtime->find(_attachableId).isNull()) {
        auto handle = hg::MoveToUnderlying(runtime->detachObject(getId()));
        HG_ASSERT(handle.isOwning());
        handle.reset(); // Destroy self
        return;
    }

    if (_attachIfPositionIsRight()) {
        QAO_Destroy(*this);
        return;
    }

    if (!_inHeadlessContext) {
        // DEV-mode UI code
        // TODO: implement ghost holding for dev mode
#if 0
    const auto& winMgr      = ccomp<MWindow>();
    const auto  input       = winMgr.getInput();
    const auto  mousePos    = input.getViewRelativeMousePos();
    const auto  mousePosRel = mousePos - _shape.getAnchor();

    if (_leftClicked) {
        if (_shape.intersectsWithPointRel(mousePosRel)) {
            if (_held) {
                _controllerAnchorOffset =
                    hg::math::RotateVector(_shape.getAnchor() - _controllerPtr->getAnchor(),
                                           -_controllerPtr->getRotation());
                _controllerAngleOffset =
                    _controllerPtr->getRotation().shortestDistanceTo(_shape.getRotation());
                _held = false;
            } else {
                _cursorOffset = mousePosRel;
                _held         = true;
            }
        } else {
            _controllerAnchorOffset =
                hg::math::RotateVector(_shape.getAnchor() - _controllerPtr->getAnchor(),
                                       _controllerPtr->getRotation());
            _controllerAngleOffset =
                _controllerPtr->getRotation().shortestDistanceTo(_shape.getRotation());
            _held = false;
        }
        _shift = false;
    }

    if (_held) {
        bool needRecalc = false;

        if (auto wheelScroll = input.getVerticalMouseWheelScroll(); wheelScroll != 0.f) {
            _shape.setRotation(_shape.getRotation() + hg::math::AngleF::fromDeg(wheelScroll));
            needRecalc = true;
        }

        _shape.setAnchor(mousePos - _cursorOffset);

        // if (needRecalc) {
        _shape.recalcRel();
        _controllerPtr->calcFootprint(_shape, _cellFootprint);
        
        const auto eval = _controllerPtr->evalAttachment(*this, _cellFootprint);
        HG_LOG_INFO(LOG_ID, "Eval status = {}", (int)eval.status);
        // }
    } else {
        _shape.setRotation(_controllerPtr->getRotation() + _controllerAngleOffset);

        const auto ctrlAnchor = _controllerPtr->getAnchor();
        const auto ctrlRot    = _controllerPtr->getRotation();
        const auto rotOffset  = hg::math::RotateVector(_controllerAnchorOffset, ctrlRot);
        _shape.setAnchor(ctrlAnchor + rotOffset);

        _shape.recalcRel();
        _controllerPtr->calcFootprint(_shape, _cellFootprint);
    }
#endif
    }

    const auto& self         = _getCurrentState();
    const auto& controllerVS = _controllerPtr->getCurrentVisibleState();
    const auto  rotOffset    = hg::math::RotateVector(self.getXYOffset(), controllerVS.rotation);
    _shape.setAnchor(controllerVS.getPosition() + rotOffset);
    _shape.setRotation(controllerVS.rotation + self.angleOffsetRelativeToShipController);

    _shape.recalcRel();
    _controllerPtr->calcFootprint(_shape, _cellFootprint);
}

void AttachableGhost::_eventPostUpdate(spe::IfMaster) {
    _getCurrentState().commit();
}

void AttachableGhost::_eventDraw1() {
    if (isDeactivated()) {
        return;
    }

    const auto vertCount = _shape.getVertexCount();

    // Draw the AttachableGhost itself
    uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLE_FAN, vertCount + 2, _shape.getAnchor()};

    vArr.vertices[0].position = {};
    vArr.vertices[0].color    = uwga::COLOR_GREY;

    const auto outputVerts = _shape.getOutputVertices();

    for (std::size_t i = 0; i < hg::pztos(vertCount); ++i) {
        vArr.vertices[i + 1].position = outputVerts[i].cast<float>();
        vArr.vertices[i + 1].color    = uwga::COLOR_GREY.withAlpha(125);
    }
    vArr.vertices[vertCount + 1] = vArr.vertices[1];

    auto& canvas = ccomp<MInteractivity>().getActiveCanvas<DrawOverworld>();
    canvas.draw(vArr);

    _shape.debugDraw(hg::uwga::COLOR_AQUA, canvas);

    // Draw the construction grid on top
    if (_isBeingHeld()) {
        _controllerPtr->drawGridOverGhost(*this, canvas);
    }
}

// MARK: Private impl.

ShipAttachable* AttachableGhost::_findAttachableById(QAO_GenericId aAttachableId) const {
    HG_ASSERT(getRuntime() != nullptr);

    auto handle = getRuntime()->find(aAttachableId);
    HG_ASSERT(!handle.isNull());

    ShipAttachable* ptr = nullptr;
    if (!QAO_SendMessage<DowncastToShipAttachable>(*handle, &ptr)) {
        HG_THROW_TRACED(hg::TracedLogicError,
                        0,
                        "Object {} did not answer to message 'DowncastToShipAttachable'.",
                        handle->getDebugDescription());
    }

    HG_ASSERT(ptr != nullptr);

    return ptr;
}

ShipController* AttachableGhost::_findControllerById(QAO_GenericId aAttachableId) const {
    HG_ASSERT(getRuntime() != nullptr);

    auto handle = getRuntime()->find(aAttachableId);
    HG_ASSERT(!handle.isNull());

    ShipController* ptr = nullptr;
    if (!QAO_SendMessage<DowncastToShipController>(*handle, &ptr)) {
        HG_THROW_TRACED(hg::TracedLogicError,
                        0,
                        "Object {} did not answer to message 'DowncastToShipController'.",
                        handle->getDebugDescription());
    }

    HG_ASSERT(ptr != nullptr);

    return ptr;
}

void AttachableGhost::_initAssociatedObjectRefs(QAO_GenericId aAssociatedShipController,
                                                QAO_GenericId aAssociatedAttachableId) {
    _controllerPtr = _findControllerById(aAssociatedShipController);
    _controllerId  = aAssociatedShipController;

    _attachablePtr = _findAttachableById(aAssociatedAttachableId);
    _attachableId  = aAssociatedAttachableId;
}

bool AttachableGhost::_attachIfPositionIsRight() {
    HG_ASSERT(isMasterObject());

    if (_isBeingHeld()) {
        return false;
    }

    const auto centerDistance =
        hg::math::EuclideanDist(_shape.getAnchor(), _attachablePtr->getPolyShape().getAnchor());
    if (centerDistance > 32.0 /* TODO: magic number */) {
        return false;
    }

    const auto rotationDistance =
        _shape.getRotation().shortestDistanceTo(_attachablePtr->getPolyShape().getRotation());
    if (std::abs(rotationDistance.asRad()) > hg::math::DegToRad(10.f) /* TODO: magic number */) {
        return false;
    }

    const auto* iwSliceData = _attachablePtr->getInteriorWorldSliceData();
    if (iwSliceData == nullptr) {
        _controllerPtr->calcFootprint(_shape, _cellFootprint);
        const auto attachmentEval = _controllerPtr->evalAttachment(*this, _cellFootprint);
        if (attachmentEval.status == AttachmentEvaluation::ALL_VALID) {
            _controllerPtr->attach(*this, _cellFootprint, attachmentEval);
            return true;
        } else {
            // TODO
        }
    } else {
        const auto attachmentEval = _controllerPtr->evalAttachment(*this);
        if (attachmentEval.status == AttachmentEvaluation::ALL_VALID) {
            HG_NOT_IMPLEMENTED("TODO");
        } else {
            // TODO
        }
    }

    return false;
}

void AttachableGhost::_dummyState_changeTo_PICKING_UP(bool aAsk) {
    HG_ASSERT(!isMasterObject());
    if (_dummyState == DummyState::PICKING_UP) {
        return;
    }

    if (aAsk) {
        Compose_AskServerToHoldAttachableGhost(ccomp<MNetworking>().getNode(),
                                               RN_COMPOSE_FOR_ALL,
                                               this->getSyncId(),
                                               _leftClickPlayerIndex);
    }

    if (_dummyState != DummyState::HOLDING_AND_SENDING) {
        _cursorOffset = (ccomp<MWindow>().getInput().getViewRelativeMousePos() - _shape.getAnchor());
    }

    _dummyState = DummyState::PICKING_UP;
}

void AttachableGhost::_dummyState_changeTo_HOLDING_AND_SENDING(bool aAsk) {
    HG_ASSERT(!isMasterObject());
    if (_dummyState == DummyState::HOLDING_AND_SENDING) {
        return;
    }

    if (aAsk) {
        Compose_AskServerToHoldAttachableGhost(ccomp<MNetworking>().getNode(),
                                               RN_COMPOSE_FOR_ALL,
                                               this->getSyncId(),
                                               _leftClickPlayerIndex);
    }

    if (_dummyState != DummyState::PICKING_UP) {
        _cursorOffset = (ccomp<MWindow>().getInput().getViewRelativeMousePos() - _shape.getAnchor());
    }

    _dummyState = DummyState::HOLDING_AND_SENDING;
}

void AttachableGhost::_dummyState_changeTo_DROPPING(bool aAsk) {
    HG_ASSERT(!isMasterObject());
    if (_dummyState == DummyState::DROPPING) {
        return;
    }

    if (aAsk) {
        Compose_AskServerToDropAttachableGhost(ccomp<MNetworking>().getNode(),
                                               RN_COMPOSE_FOR_ALL,
                                               this->getSyncId(),
                                               _leftClickPlayerIndex);
    }

    _dummyState = DummyState::DROPPING;
}

void AttachableGhost::_dummyState_changeTo_RECEIVING(bool aAsk) {
    HG_ASSERT(!isMasterObject());
    if (_dummyState == DummyState::RECEIVING) {
        return;
    }

    if (aAsk) {
        Compose_AskServerToDropAttachableGhost(ccomp<MNetworking>().getNode(),
                                               RN_COMPOSE_FOR_ALL,
                                               this->getSyncId(),
                                               _leftClickPlayerIndex);
    }

    _dummyState = DummyState::RECEIVING;
}

// MARK: Sync functions

namespace {
// clang-format off
RN_DEFINE_RPC(AttachableGhost_SyncCreate, 
    RN_ARGS(
        spe::SyncId ,aSyncId, 
        spe::SyncId ,aAssociatedShipControllerSyncId,
        spe::SyncId ,aAssociatedShipAttachableSyncId,
        std::int8_t ,aOnBehalfOfPlayerIndex
    )
) {
    RN_NODE_IN_HANDLER().illegalIfServer(__func__);

    RN_NODE_IN_HANDLER().callIfClient([&](hg::RN_ClientInterface& aClient) {
        auto  rc         = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aClient);
        auto& qaoRuntime = rc.gameContext.getQAORuntime();

        if (spe::MapSyncIdToObject(rc, aSyncId) == nullptr) {
            AttachableGhost::createDummy(&qaoRuntime, 
                                         aSyncId,
                                         aAssociatedShipControllerSyncId,
                                         aAssociatedShipAttachableSyncId,
                                         aOnBehalfOfPlayerIndex);
        }
    });
}
// clang-format on
} // namespace

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AttachableGhost, (UPDATE, DESTROY));

void AttachableGhost::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    const auto& rt = getRuntime();
    Compose_AttachableGhost_SyncCreate(
        aSyncCtrl.getLocalNode(),
        aSyncCtrl.getFilteredRecepients(),
        this->getSyncId(),
        rt->find<spe::SynchronizedObjectBase>(this->_controllerId)->getSyncId(),
        rt->find<spe::SynchronizedObjectBase>(this->_attachableId)->getSyncId(),
        -1);
}

void AttachableGhost::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    aSyncCtrl.filter([](hg::PZInteger aClientIndex) -> spe::SyncFilterStatus {
        return spe::SyncFilterStatus::REGULAR_SYNC; // TODO: implement proper filtering
    });
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AttachableGhost, aSyncCtrl);
}

void AttachableGhost::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AttachableGhost, aSyncCtrl);
}

// MARK: Free functions

namespace {
// clang-format off
RN_DEFINE_RPC(TryCreateAttachableGhost,
    RN_ARGS(
        spe::SyncId ,aAssociatedShipControllerSyncId,
        spe::SyncId ,aAssociatedShipAttachableSyncId,
        std::int8_t ,aOnBehalfOfPlayerIndex
    )
) {
    RN_NODE_IN_HANDLER().callIfClient([](hg::RN_ClientInterface&) {
        throw RN_IllegalMessage("TryCreateAttachableGhost is only for the server!");
    });

    RN_NODE_IN_HANDLER().callIfServer([=](hg::RN_ServerInterface& aServer) {
        auto  rc             = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aServer);
        auto& netMgr         = rc.netwMgr;
        auto* shipController = netMgr.mapSyncIdToObject(aAssociatedShipControllerSyncId);
        auto* shipAttachable = netMgr.mapSyncIdToObject(aAssociatedShipAttachableSyncId);
        if (shipController && shipAttachable) {
            AttachableGhost::createMaster(rc.gameContext.getQAORuntime(),
                                          shipController->getId(),
                                          shipAttachable->getId(), 
                                          aOnBehalfOfPlayerIndex);
        }
    });
}
// clang-format on
} // namespace

void AskServerToCreateAttachableGhost(spe::NetworkingManager& aNetworkingManager,
                                      spe::SyncId             aAssociatedShipControllerSyncId,
                                      spe::SyncId             aAssociatedShipAttachableSyncId,
                                      int                     aOnBehalfOfPlayerIndex) {
    Compose_TryCreateAttachableGhost(aNetworkingManager.getNode(),
                                     RN_COMPOSE_FOR_ALL,
                                     aAssociatedShipControllerSyncId,
                                     aAssociatedShipAttachableSyncId,
                                     static_cast<std::int8_t>(aOnBehalfOfPlayerIndex));
}

} // namespace cinnabar

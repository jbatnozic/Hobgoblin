// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Asteroid.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <Attachable_ghost.hpp>
#include <Interactivity_manager.hpp>
#include <Overworld_manager.hpp>

namespace cinnabar {

namespace {
// clang-format off
RN_DEFINE_RPC(Asteroid_SyncCreate, 
    RN_ARGS(
        spe::SyncId,       aSyncId, 
        hg::util::Packet&, aPolyShapePacket
    )
) {
    auto& node = RN_NODE_IN_HANDLER();

    node.callIfClient([&](hg::RN_ClientInterface& aClient) {
        auto  rc         = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aClient);
        auto& qaoRuntime = rc.gameContext.getQAORuntime();

        if (spe::MapSyncIdToObject(rc, aSyncId) == nullptr) {
            Asteroid::createDummy(&qaoRuntime, aSyncId, aPolyShapePacket);
        }
    });

    node.callIfServer([](hg::RN_ServerInterface&) {
        throw hg::RN_IllegalMessage("Server received a sync message");
    });
}
// clang-format on
} // namespace

#define SIZE 32.f

QAO_Handle<Asteroid> Asteroid::createMaster(QAO_RuntimeRef aRuntime, hg::math::Vector2d aPosition) {
    auto handle = QAO_Create<Asteroid>(aRuntime);

    cpBodySetPosition(handle->_unibody, cpv(aPosition.x, aPosition.y));
    cpBodySetAngle(handle->_unibody, hg::math::PI * hg::util::GetRandomNumber(0.0, 359.9) / 180.0);

    return handle;
}

Asteroid::Asteroid(QAO_InstGuard aInstGuard)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES,
                   QAO_STATIC_NAME("cinnabar::Asteroid"),
                   spe::SYNC_ID_NEW}
    , UnibodyShipAttachable{std::bind(&Asteroid::_initPolyShape, this),
                            std::bind(&Asteroid::_initPhysicalProperties, this),
                            std::bind(&Asteroid::_initColDelegate, this),
                            std::bind(&Asteroid::_alvinBodyFromPhysicalPropertiesAndPolyShape, this),
                            std::bind(&Asteroid::_alvinShapeFromPolyShape, this)} //
{
    _unibody.bindDelegate(*this);
}

QAO_Handle<Asteroid> Asteroid::createDummy(QAO_RuntimeRef    aRuntime,
                                           spe::SyncId       aSyncId,
                                           hg::util::Packet& aPolyShapePacket) {
    return QAO_Create<Asteroid>(aRuntime, aSyncId, aPolyShapePacket);
}

Asteroid::Asteroid(QAO_InstGuard aInstGuard, spe::SyncId aSyncId, hg::util::Packet& aPolyShapePacket)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES,
                   QAO_STATIC_NAME("cinnabar::Asteroid"),
                   aSyncId}
    , UnibodyShipAttachable{} //
{
    _polyShape.readRawVerticesFromStream(aPolyShapePacket);
}

Asteroid::~Asteroid() {
    ShipAttachable::_detach();
}

// MARK: QAO Message Handlers

void Asteroid::msgDowncastToShipAttachable(DowncastToShipAttachable::PayloadPtr aPtr,
                                           bool /* aConst */) {
    (*aPtr) = static_cast<ShipAttachable*>(this);
}

void Asteroid::msgHandlePNCSEvent(HandlePNCSEvent::PayloadPtr aPayload, bool /* aConst */) {
    HG_ASSERT(aPayload != nullptr);
    if (aPayload->mbLeftDown) {
        auto* runtime  = this->getRuntime();
        auto  shipCtrl = runtime->find("cinnabar::ShipController"); // TODO: temporary
        auto  ghost    = QAO_Create<AttachableGhost>(runtime);
        ghost->init(shipCtrl->getId(), this->getId());
    }
}

// MARK: Private

PolyShape Asteroid::_initPolyShape() {
    const auto vertCount = hg::util::GetRandomNumber<hg::PZInteger>(7, 12);

    PolyShape shape{vertCount};

    for (hg::PZInteger i = 0; i < vertCount; ++i) {
        const auto vec =
            (hg::math::AngleF::fullCircle() * (float)i / (float)vertCount).asNormalizedVector() *
            hg::util::GetRandomNumber(100.f, 200.f);
        shape.setRawVertexAt(i, vec);
    }

    const auto baricenterOffset = shape.calculateBaricenterOffset();
    for (hg::PZInteger i = 0; i < vertCount; ++i) {
        shape.setRawVertexAt(i, shape.getRawVertexAt(i) - baricenterOffset);
    }

    shape.recalcRel();

    return shape;
}

Asteroid::PhysicalProperties Asteroid::_initPhysicalProperties() {
    return {.mass = 5.0};
}

hg::alvin::CollisionDelegate Asteroid::_initColDelegate() {
    return hg::alvin::CollisionDelegateBuilder{}
        .setDefaultDecision(hg::alvin::Decision::ACCEPT_COLLISION)
        .finalize();
}

void Asteroid::_didAttach(QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);

    if (isMasterObject()) {
        _getCurrentState().initMirror();
        _unibody.addToSpace(ccomp<MOverworld>().getAlvinSpace()); 
    }
}

void Asteroid::_eventBeginUpdate() {
    _leftClicked = false;

    ccomp<MInteractivity>().pushClickableObject(
        this->getId(),
        0,
        0,
        /* quick check */
        [this](hg::math::Vector2d aMouseWorldPos) -> bool {
            return (aMouseWorldPos - _polyShape.getAnchor()).lengthSquared() <=
                   _polyShape.getDistanceToFarthestRawVertexSquared();
        },
        /* full check */
        [this](hg::math::Vector2d aMouseWorldPos) -> bool {
            return _polyShape.intersectsWithPointRel(aMouseWorldPos - _polyShape.getAnchor());
        });
}

void Asteroid::_eventUpdate2(spe::IfMaster) {
    _syncPolyShapeWithUnibody();

    auto& self = _getCurrentState();
    self.setPosition(_polyShape.getAnchor());
    self.setRotation(_polyShape.getRotation());
}

void Asteroid::_eventUpdate2(spe::IfDummy) {
    if (isDeactivated()) {
        return;
    }

    const auto& self = _getCurrentState();
    _polyShape.setAnchor(self.getPosition());
    _polyShape.setRotation(self.getRotation());
}

void Asteroid::_eventDraw1() {
    auto& canvas = ccomp<MWindow>().getActiveCanvas();

    const auto vertCount = _polyShape.getVertexCount();

    // Draw the asteroid interior
    {
        uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLE_FAN, vertCount + 2, _polyShape.getAnchor()};

        vArr.vertices[0].position = {};
        vArr.vertices[0].color    = uwga::COLOR_GREY;

        const auto outputVerts = _polyShape.getOutputVertices();

        for (std::size_t i = 0; i < hg::pztos(vertCount); ++i) {
            vArr.vertices[i + 1].position = outputVerts[i].cast<float>();
            vArr.vertices[i + 1].color    = uwga::COLOR_GREY;
        }
        vArr.vertices[vertCount + 1] = vArr.vertices[1];

        canvas.draw(vArr);
    }

    _polyShape.debugDraw(hg::uwga::COLOR_AQUA, canvas);
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(Asteroid, (UPDATE, DESTROY));

void Asteroid::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    hg::util::Packet polyShapePacket;
    _polyShape.writeRawVerticesToStream(polyShapePacket);
    Compose_Asteroid_SyncCreate(aSyncCtrl.getLocalNode(),
                                aSyncCtrl.getFilteredRecepients(),
                                this->getSyncId(),
                                polyShapePacket);
}

void Asteroid::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    aSyncCtrl.filter([](hg::PZInteger aClientIndex) -> spe::SyncFilterStatus {
        return spe::SyncFilterStatus::REGULAR_SYNC; // TODO: implement proper filtering
    });
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(Asteroid, aSyncCtrl);
}

void Asteroid::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(Asteroid, aSyncCtrl);
}

} // namespace cinnabar

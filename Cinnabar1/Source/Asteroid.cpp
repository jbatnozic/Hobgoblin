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

#define SIZE 32.f

Asteroid::Asteroid(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       PRIORITY_ENTITIES,
                       QAO_STATIC_NAME("cinnabar::Asteroid")} // clang-format off
    , UnibodyShipAttachable{
        std::bind(&Asteroid::_initPolyShape, this),
        std::bind(&Asteroid::_initPhysicalProperties, this),
        std::bind(&Asteroid::_initColDelegate, this),
        std::bind(&Asteroid::_alvinBodyFromPhysicalPropertiesAndPolyShape, this),
        std::bind(&Asteroid::_alvinShapeFromPolyShape, this)
    } // clang-format on
{
    _unibody.bindDelegate(*this);
}

void Asteroid::init(hg::math::Vector2d aPosition) {
    cpBodySetPosition(_unibody, cpv(aPosition.x, aPosition.y));
    cpBodySetAngle(_unibody, hg::math::PI * hg::util::GetRandomNumber(0.0, 359.9) / 180.0);
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
    spe::StateObject::_didAttach(aRuntime);

    _unibody.addToSpace(ccomp<MOverworld>().getAlvinSpace());
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

void Asteroid::_eventUpdate1() {
    // const auto& winMgr = ccomp<MWindow>();
    // _applyPropulsion(winMgr.getInput());
}

void Asteroid::_eventUpdate2() {
    _syncPolyShapeWithUnibody();
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

} // namespace cinnabar

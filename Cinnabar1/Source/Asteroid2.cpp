// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Asteroid2.hpp>

#include <Hobgoblin/Utility/Randomization.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <Overworld_manager_interface.hpp>

namespace cinnabar {

#define SIZE 32.f

Asteroid2::Asteroid2(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       PRIORITY_ENTITIES,
                       QAO_STATIC_NAME("cinnabar::Asteroid2")}
    // clang-format off
    , UnibodyShipAttachable{
        std::bind(&Asteroid2::_initPolyShape, this),
        std::bind(&Asteroid2::_initPhysicalProperties, this),
        std::bind(&Asteroid2::_initColDelegate, this),
        std::bind(&Asteroid2::_alvinBodyFromPhysicalPropertiesAndPolyShape, this),
        std::bind(&Asteroid2::_alvinShapeFromPolyShape, this)
    } // clang-format on
{
    _unibody.bindDelegate(*this);
}

void Asteroid2::init(hg::math::Vector2d aPosition) {
    cpBodySetPosition(_unibody, cpv(aPosition.x, aPosition.y));
    cpBodySetAngle(_unibody, hg::math::PI * hg::util::GetRandomNumber(0.0, 359.9) / 180.0);
}

// MARK: Private

PolyShape Asteroid2::_initPolyShape() {
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

Asteroid2::PhysicalProperties Asteroid2::_initPhysicalProperties() {
    return {.mass = 5.0};
}

hg::alvin::CollisionDelegate Asteroid2::_initColDelegate() {
    return hg::alvin::CollisionDelegateBuilder{}
        .setDefaultDecision(hg::alvin::Decision::ACCEPT_COLLISION)
        .finalize();
}

void Asteroid2::_didAttach(QAO_Runtime& aRuntime) {
    spe::StateObject::_didAttach(aRuntime);

    _unibody.addToSpace(ccomp<MOverworld>().getAlvinSpace());
}

void Asteroid2::_eventUpdate1() {
    // const auto& winMgr = ccomp<MWindow>();
    // _applyPropulsion(winMgr.getInput());
}

void Asteroid2::_eventUpdate2() {
    _syncPolyShapeWithUnibody();
}

void Asteroid2::_eventDraw1() {
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

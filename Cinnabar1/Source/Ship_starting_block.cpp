// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_starting_block.hpp>

#include <Overworld_manager_interface.hpp>
#include <Ship/Constants.hpp>

namespace cinnabar {

#define SIZE (16.f * OVERWORLD_CELL_SIZE)

ShipStartingBlock::ShipStartingBlock(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       PRIORITY_ENTITIES,
                       QAO_STATIC_NAME("cinnabar::ShipStartingBlock")}
    // clang-format off
    , UnibodyShipAttachable{
        std::bind(&ShipStartingBlock::_initPolyShape, this),
        std::bind(&ShipStartingBlock::_initPhysicalProperties, this),
        std::bind(&ShipStartingBlock::_initColDelegate, this),
        std::bind(&ShipStartingBlock::_alvinBodyFromPhysicalPropertiesAndPolyShape, this),
        std::bind(&ShipStartingBlock::_alvinShapeFromPolyShape, this)
    }
    // clang-format on
{
    _unibody.bindDelegate(*this);

    _iwSliceData = std::make_unique<InteriorWorldSliceData>();
    _iwSliceData->cells.reset(16, 16);
    _iwSliceData->cellGridOffset =
        hg::math::Vector2d{
            -16 * OVERWORLD_CELL_SIZE,
            -16 * OVERWORLD_CELL_SIZE,
        } /
        2.0;
}

void ShipStartingBlock::init(hg::math::Vector2d aPosition) {
    cpBodySetPosition(_unibody, cpv(aPosition.x, aPosition.y));
}

// MARK: Private

PolyShape ShipStartingBlock::_initPolyShape() {
    PolyShape shape{4};

    // Clockwise (I think clockwise is correct)
    shape.setRawVertexAtUnchecked(0, {-SIZE, -SIZE});
    shape.setRawVertexAtUnchecked(1, {+SIZE, -SIZE});
    shape.setRawVertexAtUnchecked(2, {+SIZE, +SIZE});
    shape.setRawVertexAtUnchecked(3, {-SIZE, +SIZE});

    // Note: baricenter offset is guaranteed 0,0 because the shape is completely symmetrical

    shape.recalcRel();

    return shape;
}

ShipStartingBlock::PhysicalProperties ShipStartingBlock::_initPhysicalProperties() {
    return {.mass = 5.0};
}

hg::alvin::CollisionDelegate ShipStartingBlock::_initColDelegate() {
    return hg::alvin::CollisionDelegateBuilder{}
        .setDefaultDecision(hg::alvin::Decision::ACCEPT_COLLISION)
        .finalize();
}

void ShipStartingBlock::_didAttach(QAO_Runtime& aRuntime) {
    spe::StateObject::_didAttach(aRuntime);

    _unibody.addToSpace(ccomp<MOverworld>().getAlvinSpace());
}

void ShipStartingBlock::_eventUpdate1() {
    const auto& winMgr = ccomp<MWindow>();
    _applyPropulsion(winMgr.getInput());
}

void ShipStartingBlock::_eventUpdate2() {
    _syncPolyShapeWithUnibody();
}

void ShipStartingBlock::_eventDraw1() {
    auto& winMgr = ccomp<MWindow>();
    _polyShape.debugDraw(hg::uwga::COLOR_GREY, winMgr.getActiveCanvas());
}

void ShipStartingBlock::_applyPropulsion(const spe::WindowFrameInputView& aInput) {
    const auto left  = aInput.checkPressed(hg::in::PK_A);
    const auto right = aInput.checkPressed(hg::in::PK_D);
    const auto up    = aInput.checkPressed(hg::in::PK_W);
    const auto down  = aInput.checkPressed(hg::in::PK_S);

    // Propulsion
    const cpVect  pos                = cpBodyGetPosition(_unibody);
    const cpFloat propulsionStrength = 4000.0;

    if (up) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, force, pos);
    }

    if (down) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), -0.5 * propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, force, pos);
    }

    if (left) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), 0.5 * propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, cpvrperp(force), pos);
    }

    if (right) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), 0.5 * propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, cpvperp(force), pos);
    }

    // Rotation
    auto rotForce = cpv(0.0, 0.0);

    if (aInput.checkPressed(hg::in::PK_E)) {
        rotForce.y += 100.0;
    }
    if (aInput.checkPressed(hg::in::PK_Q)) {
        rotForce.y -= 100.0;
    }

    cpBodyApplyForceAtLocalPoint(_unibody, rotForce, cpv(16.0, 0.0));
    cpBodyApplyForceAtLocalPoint(_unibody, cpvneg(rotForce), cpvzero);
}

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_starting_block.hpp>

namespace cinnabar {

#define SIZE 32.f

PolyShape ShipStartingBlock::_initPolyShape() {
    PolyShape shape{4};
    // Counter-clockwise for now
    shape.setRawVertexAtUnchecked(0, {-SIZE, -SIZE});
    shape.setRawVertexAtUnchecked(1, {-SIZE, +SIZE});
    shape.setRawVertexAtUnchecked(2, {+SIZE, +SIZE});
    shape.setRawVertexAtUnchecked(3, {+SIZE, -SIZE});
    return shape;
}

ShipStartingBlock::PhysicalProperties ShipStartingBlock::_initPhysicalProperties() {
    return {.mass = 5.0};
}

hg::alvin::CollisionDelegate ShipStartingBlock::_initColDelegate() {}

} // namespace cinnabar

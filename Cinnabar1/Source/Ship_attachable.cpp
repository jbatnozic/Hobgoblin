// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_attachable.hpp>

namespace cinnabar {

#define RADIUS (0.0)

const PolyShape& UnibodyShipAttachable::getPolyShape() const {
    return _polyShape;
}

hg::alvin::Body UnibodyShipAttachable::_alvinBodyFromPhysicalPropertiesAndPolyShape() {
    // assert(_polyShape.calculateBaricenterOffset() == hg::math::Vector2f{0.f, 0.f}); // TODO
    const auto mass = _physicalProperties.mass;
    return hg::alvin::Body::createDynamic(mass,
                                          cpMomentForPoly(mass,
                                                          _polyShape.getVertexCount(),
                                                          _polyShape.getOutputVerticesAsCpVect(),
                                                          cpvzero,
                                                          RADIUS));
}

const ShipAttachable::PhysicalProperties& UnibodyShipAttachable::getPhysicalProperties() const {
    return _physicalProperties;
}

hg::alvin::Shape UnibodyShipAttachable::_alvinShapeFromPolyShape() {
    _polyShape.recalcRel();
    return {cpPolyShapeNew(_unibody.body,
                           _polyShape.getVertexCount(),
                           _polyShape.getOutputVerticesAsCpVect(),
                           cpTransformIdentity,
                           RADIUS)};
}

} // namespace cinnabar

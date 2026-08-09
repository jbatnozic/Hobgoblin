// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_attachable.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>

namespace cinnabar {

#define RADIUS (0.0)

const PolyShape& UnibodyShipAttachable::getPolyShape() const {
    return _polyShape;
}

hg::alvin::Body UnibodyShipAttachable::_alvinBodyFromPhysicalPropertiesAndPolyShape() {
    HG_ASSERT(hg::math::IsNearZero(_polyShape.calculateBaricenterOffset().length(), 0.1f));
    const auto mass = _physicalProperties.mass;
    const auto moment = cpMomentForPoly(mass,
                                        _polyShape.getVertexCount(),
                                        _polyShape.getOutputVerticesAsCpVect(),
                                        cpvzero,
                                        RADIUS);
    return hg::alvin::Body::createDynamic(mass, moment);
}

const ShipAttachable::PhysicalProperties& UnibodyShipAttachable::getPhysicalProperties() const {
    return _physicalProperties;
}

hg::alvin::Shape UnibodyShipAttachable::_alvinShapeFromPolyShape() {
    HG_ASSERT(_polyShape.getState() == PolyShape::READY_RELATIVE);
    return {cpPolyShapeNew(_unibody.body,
                           _polyShape.getVertexCount(),
                           _polyShape.getOutputVerticesAsCpVect(),
                           cpTransformIdentity,
                           RADIUS)};
}

void UnibodyShipAttachable::_syncPolyShapeWithUnibody(double /* aAcceptableDelta */) {
    const auto position = cpBodyGetPosition(_unibody);
    const auto rotation = cpBodyGetRotation(_unibody);

    _polyShape.setAnchor({position.x, position.y});
    _polyShape.setRotation(hg::math::AngleF::fromVector(rotation.x, rotation.y));
    _polyShape.recalcRel();
}

} // namespace cinnabar

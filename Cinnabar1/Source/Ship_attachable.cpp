// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_attachable.hpp>

#include <Ship_controller.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>

#include <cassert>

namespace cinnabar {

// MARK: ShipAttachable

ShipAttachable::~ShipAttachable() {
    _assertDetached("ShipAttachable");
}

void ShipAttachable::_assertDetached(std::string_view aCaller) const {
    if (_assocComps.has_value()) {
        assert(false && "A ShipAttachable must be detached from a ShipController by its derived class!");
        HG_THROW_TRACED(hg::AssertionFailedError,
                        0,
                        "A ShipAttachable wasn't detached from its ShipController by its derived class "
                        "(called by {}).",
                        aCaller);
    }
}

void ShipAttachable::_detach() {
    if (!_assocComps.has_value()) {
        return;
    }
    _assocComps->controller.detach(*this);
    HG_ASSERT(!_assocComps.has_value());
}

// MARK: UnibodyShipAttachable

#define RADIUS (0.0)

UnibodyShipAttachable::~UnibodyShipAttachable() {
    _assertDetached("UnibodyShipAttachable");
}

const PolyShape& UnibodyShipAttachable::getPolyShape() const {
    return _polyShape;
}

hg::NeverNull<cpBody*> UnibodyShipAttachable::getPhysicsBody() {
    return _unibody.body;
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

const ShipAttachable::InteriorWorldSliceData* UnibodyShipAttachable::getInteriorWorldSliceData() const {
    return _iwSliceData.get();
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

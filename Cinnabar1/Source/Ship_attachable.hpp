// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Alvin.hpp>

#include <Poly_shape.hpp>

namespace cinnabar {

class ShipAttachable {
public:
    virtual ~ShipAttachable() = default;

    virtual const PolyShape& getPolyShape() const = 0;

    struct PhysicalProperties {
        double mass;
    };

    virtual const PhysicalProperties& getPhysicalProperties() const = 0;
};

class UnibodyShipAttachable : public ShipAttachable {
public:
    //! \param taPolyShapeFactory create a poly shape (in <TODO> winding and without repeating the 1st vertex)
    template <class taPolyShapeFactory,
              class taPhysicalPropertiesFactory,
              class taUnibodyCollisionDelegateFactory,
              class taUnibodyBodyFactory,
              class taUnibodyShapeFactory>
    UnibodyShipAttachable(taPolyShapeFactory&&                aPolyShapeFactory,
                          taPhysicalPropertiesFactory&&       aPhysicalPropertiesFactory,
                          taUnibodyCollisionDelegateFactory&& aUnibodyCollisionDelegateFactory,
                          taUnibodyBodyFactory&&              aUnibodyBodyFactory,
                          taUnibodyShapeFactory&&             aUnibodyShapeFactory);

    const PolyShape& getPolyShape() const override;

    const PhysicalProperties& getPhysicalProperties() const override;

protected:
    hg::alvin::Body  _alvinBodyFromPhysicalPropertiesAndPolyShape();
    hg::alvin::Shape _alvinShapeFromPolyShape();

    PolyShape          _polyShape;
    PhysicalProperties _physicalProperties;
    hg::alvin::Unibody _unibody;
};

template <class taPolyShapeFactory,
          class taPhysicalPropertiesFactory,
          class taUnibodyCollisionDelegateFactory,
          class taUnibodyBodyFactory,
          class taUnibodyShapeFactory>
UnibodyShipAttachable::UnibodyShipAttachable(
    taPolyShapeFactory&&                aPolyShapeFactory,
    taPhysicalPropertiesFactory&&       aPhysicalPropertiesFactory,
    taUnibodyCollisionDelegateFactory&& aUnibodyCollisionDelegateFactory,
    taUnibodyBodyFactory&&              aUnibodyBodyFactory,
    taUnibodyShapeFactory&&             aUnibodyShapeFactory)
    : _polyShape{aPolyShapeFactory()}
    , _physicalProperties{aPhysicalPropertiesFactory()}
    , _unibody{std::forward<taUnibodyCollisionDelegateFactory>(aUnibodyCollisionDelegateFactory),
               std::forward<taUnibodyBodyFactory>(aUnibodyBodyFactory),
               std::forward<taUnibodyShapeFactory>(aUnibodyShapeFactory)} {}

} // namespace cinnabar

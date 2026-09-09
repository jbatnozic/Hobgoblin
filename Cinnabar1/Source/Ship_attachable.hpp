// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Common/Nullability.hpp>
#include <Hobgoblin/Alvin.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>

#include <Graph_of_attachables.hpp>
#include <InteriorWorld/Cell_props.hpp>
#include <Poly_shape.hpp>

#include <memory>
#include <optional>

namespace cinnabar {

class ShipController;

class ShipAttachable {
public:
    virtual ~ShipAttachable();

    virtual const PolyShape& getPolyShape() const = 0;

    virtual hg::NeverNull<cpBody*> getPhysicsBody() = 0;

    struct PhysicalProperties {
        double mass;
    };

    virtual const PhysicalProperties& getPhysicalProperties() const = 0;

    virtual void drawAsGhost(
        const hg::math::Vector2d& aPosition,
        hg::math::AngleF          aAngle,
        uwga::Color               aColor,
        uwga::Canvas&             aCanvas,
        const uwga::RenderStates& aRenderStates = uwga::RENDER_STATES_DEFAULT) const = 0;

    struct InteriorWorldSliceData {
        //! Grid containing cell data for the attachable's interior.
        interior::CellPropGrid cells;

        //! Negative of the angle of the attachable at which the interior cell grid is considered
        //! axis-aligned and 'upright' in the game world's global coordinate system.
        //! In other words, when ([attachable's rotation] + `rotationOffset` == 0), its interior
        //! cell grid is axis-aligned with the overworld.
        hg::math::AngleF rotationOffset = hg::math::AngleF::zero();

        //! X/Y offset of the center of the cell which sits in the top-left corner of the interior
        //! cell grid (relative to the anchor of the attachable's PolyShape) when 
        //! ([attachable's rotation] + `rotationOffset` == 0).
        hg::math::Vector2f cellGridOffset = {};
    };

    //! \brief return data about a slice of an InteriorWorld contained within this attachable (if any).
    //!
    //! Returns nulloptr if the attachable has no defined interior (note: in this case, it can be freely
    //! attached to an existing ship at any angle).
    virtual const InteriorWorldSliceData* getInteriorWorldSliceData() const = 0;

private:
    friend class GraphOfAttachables;

    struct AssociatedComponents {
        ShipController&           controller;
        GraphOfAttachables::Node& node;

        AssociatedComponents(ShipController& aController, GraphOfAttachables::Node& aNode)
            : controller{aController}
            , node{aNode} {}
    };

    std::optional<AssociatedComponents> _assocComps;

    void _detachFromGraph();
};

class UnibodyShipAttachable : public ShipAttachable {
public:
    //! \param taPolyShapeFactory create a poly shape (in <TODO> winding and without repeating the 1st
    //! vertex)
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

    hg::NeverNull<cpBody*> getPhysicsBody() override final;

    const PhysicalProperties& getPhysicalProperties() const override;

    const InteriorWorldSliceData* getInteriorWorldSliceData() const override;

protected:
    hg::alvin::Body  _alvinBodyFromPhysicalPropertiesAndPolyShape();
    hg::alvin::Shape _alvinShapeFromPolyShape();

    void _syncPolyShapeWithUnibody(double aAcceptableDelta = 0.0);

    PolyShape                               _polyShape;
    PhysicalProperties                      _physicalProperties;
    hg::alvin::Unibody                      _unibody;
    std::unique_ptr<InteriorWorldSliceData> _iwSliceData;
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

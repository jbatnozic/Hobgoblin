// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Layer.hpp>
#include <GridGoblin/Model/Shape.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>

#include <Hobgoblin/Math.hpp>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

//! Describes the bounds of an object in a GridGoblin World, as defined by its position, size and layer.
class BoundsInfo {
public:
    BoundsInfo() = default;

    static BoundsInfo fromCenterAndSize(PositionInWorld aCenter, hg::math::Vector2d aSize, Layer aLayer);

    static BoundsInfo fromTopLeftAndSize(PositionInWorld    aTopLeft,
                                         hg::math::Vector2d aSize,
                                         Layer              aLayer);

    void setCenter(PositionInWorld aPoint);

    void setTopLeft(PositionInWorld aPoint);

    // Maintains the top-left corner, not the center!
    void setSize(hg::math::Vector2d aSize);

    void setSizeMaintainingCenter(hg::math::Vector2d aSize);

    PositionInWorld getCenter() const;

    PositionInWorld getTopLeft() const;

    const hg::math::Rectangle<double>& getBoundingBox() const;

    hg::math::Vector2d getSize() const;

    Layer getLayer() const;
    void  setLayer(Layer aLayer);

private:
    BoundsInfo(Layer aLayer);

    hg::math::Rectangle<double> _bbox;
    PositionInWorld             _center;
    Layer                       _layer = Layer::FLOOR;
};

inline PositionInWorld BoundsInfo::getCenter() const {
    return _center;
}

inline PositionInWorld BoundsInfo::getTopLeft() const {
    return PositionInWorld{
        {_bbox.getLeft(), _bbox.getTop()}
    };
}

inline const hg::math::Rectangle<double>& BoundsInfo::getBoundingBox() const {
    return _bbox;
}

inline hg::math::Vector2d BoundsInfo::getSize() const {
    return {_bbox.w, _bbox.h};
}

inline Layer BoundsInfo::getLayer() const {
    return _layer;
}
inline void BoundsInfo::setLayer(Layer aLayer) {
    _layer = aLayer;
}

} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Positional/Bounds_info.hpp>

namespace jbatnozic {
namespace gridgoblin {

namespace {
constexpr double Half(double aF) {
    return aF * 0.5;
}
} // namespace

BoundsInfo::BoundsInfo(Layer aLayer)
    : _layer{aLayer} {}

BoundsInfo BoundsInfo::fromCenterAndSize(PositionInWorld    aCenter,
                                         hg::math::Vector2d aSize,
                                         Layer              aLayer) {
    BoundsInfo result{aLayer};
    result._center = aCenter;
    result.setSizeMaintainingCenter(aSize);
    return result;
}

BoundsInfo BoundsInfo::fromTopLeftAndSize(PositionInWorld    aTopLeft,
                                          hg::math::Vector2d aSize,
                                          Layer              aLayer) {
    BoundsInfo result{aLayer};
    result._bbox.x = aTopLeft->x;
    result._bbox.y = aTopLeft->y;
    result.setSize(aSize);
    return result;
}

void BoundsInfo::setCenter(PositionInWorld aPoint) {
    _center->x = aPoint->x;
    _center->y = aPoint->y;

    _bbox.x = aPoint->x - Half(_bbox.w);
    _bbox.y = aPoint->y - Half(_bbox.h);
}

void BoundsInfo::setTopLeft(PositionInWorld aPoint) {
    _bbox.x = aPoint->x;
    _bbox.y = aPoint->y;

    _center->x = aPoint->x + Half(_bbox.w);
    _center->y = aPoint->y + Half(_bbox.h);
}

void BoundsInfo::setSize(hg::math::Vector2d aSize) {
    _bbox.w = aSize.x;
    _bbox.h = aSize.y;

    _center->x = _bbox.x + Half(aSize.x);
    _center->y = _bbox.y + Half(aSize.y);
}

void BoundsInfo::setSizeMaintainingCenter(hg::math::Vector2d aSize) {
    _bbox.w = aSize.x;
    _bbox.h = aSize.y;

    _bbox.x = _center->x - Half(aSize.x);
    _bbox.y = _center->y - Half(aSize.y);
}

} // namespace gridgoblin
} // namespace jbatnozic

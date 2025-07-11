// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Math/Vector.hpp>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

class PositionInWorld {
public:
    PositionInWorld()                                  = default;
    PositionInWorld(const PositionInWorld&)            = default;
    PositionInWorld& operator=(const PositionInWorld&) = default;
    PositionInWorld(PositionInWorld&&)                 = default;
    PositionInWorld& operator=(PositionInWorld&&)      = default;

    PositionInWorld(double aX, double aY)
        : pos{aX, aY} {}

    explicit PositionInWorld(hg::math::Vector2d aPos)
        : pos{aPos} {}

    hg::math::Vector2d pos;

    hg::math::Vector2d& operator*() {
        return pos;
    }

    const hg::math::Vector2d& operator*() const {
        return pos;
    }

    hg::math::Vector2d* operator->() {
        return &pos;
    }

    const hg::math::Vector2d* operator->() const {
        return &pos;
    }
};

static_assert(sizeof(PositionInWorld) <= 2 * sizeof(void*));

} // namespace gridgoblin
} // namespace jbatnozic

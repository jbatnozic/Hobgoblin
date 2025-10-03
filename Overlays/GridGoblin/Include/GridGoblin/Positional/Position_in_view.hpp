// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Math/Vector.hpp>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

class PositionInView {
public:
    PositionInView()                                 = default;
    PositionInView(const PositionInView&)            = default;
    PositionInView& operator=(const PositionInView&) = default;
    PositionInView(PositionInView&&)                 = default;
    PositionInView& operator=(PositionInView&&)      = default;

    explicit PositionInView(double aX, double aY)
        : pos{aX, aY} {}

    explicit PositionInView(hg::math::Vector2d aPos)
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

static_assert(sizeof(PositionInView) <= 2 * sizeof(void*));

} // namespace gridgoblin
} // namespace jbatnozic

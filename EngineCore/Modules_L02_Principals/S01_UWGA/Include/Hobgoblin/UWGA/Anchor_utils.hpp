// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_ANCHOR_UTILS_HPP
#define UHOBGOBLIN_UWGA_ANCHOR_UTILS_HPP

#include <Hobgoblin/Math/Vector.hpp>

#include <cmath>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

constexpr double ANCHOR_DEFAULT_DELTA = 0.001;

inline bool AreAnchorsApproxEq(math::Vector2d aAnchor1,
                               math::Vector2d aAnchor2,
                               double         aDelta = ANCHOR_DEFAULT_DELTA) {
    return (std::abs(aAnchor1.x - aAnchor2.x) < aDelta && std::abs(aAnchor1.y - aAnchor2.y) < aDelta);
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_ANCHOR_UTILS_HPP

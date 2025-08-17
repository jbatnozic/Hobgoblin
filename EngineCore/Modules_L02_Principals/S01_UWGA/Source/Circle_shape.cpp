// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#include <Hobgoblin/UWGA/Circle_shape.hpp>

#include <Hobgoblin/Math/Core.hpp>
#include <cmath>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

CircleShape::CircleShape(const System& aSystem, float aRadius, PZInteger aPointCount)
    : Shape{aSystem}
    , _radius{aRadius}
    , _pointCount{aPointCount} {}

void CircleShape::setRadius(float aRadius) {
    _radius      = aRadius;
    _pointsDirty = true;
}

float CircleShape::getRadius() const {
    return _radius;
}

void CircleShape::setPointCount(PZInteger aCount) {
    _pointCount  = aCount;
    _pointsDirty = true;
}

PZInteger CircleShape::getPointCount() const {
    return _pointCount;
}

math::Vector2f CircleShape::getPoint(PZInteger aIndex) const {
    float angle = (static_cast<float>(aIndex) * 2.f * math::PI_F / static_cast<float>(_pointCount)) -
                  (math::PI_F / 2.f);
    float x = std::cos(angle) * _radius;
    float y = std::sin(angle) * _radius;

    return {_radius + x, _radius + y};
}

[[nodiscard]] math::Vector2f CircleShape::getGeometricCenter() const {
    return {_radius, _radius};
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
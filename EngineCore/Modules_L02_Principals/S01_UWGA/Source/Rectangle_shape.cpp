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

#include <Hobgoblin/UWGA/Rectangle_shape.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

RectangleShape::RectangleShape(const System& aSystem, math::Vector2f aSize)
    : Shape{aSystem}
    , _size{aSize} {}

RectangleShape::RectangleShape(const RectangleShape& aOther)
    : Shape{aOther.getSystem()}
    , _size{aOther._size} //
{
    _copyShapeDataFrom(aOther);
}

RectangleShape& RectangleShape::operator=(const RectangleShape& aOther) {
    if (this != &aOther) {
        _copyShapeDataFrom(aOther);

        setSize(aOther.getSize());
    }
    return SELF;
}

void RectangleShape::setSize(math::Vector2f aSize) {
    _size        = aSize;
    _pointsDirty = true;
}

[[nodiscard]] math::Vector2f RectangleShape::getSize() const {
    return _size;
}

PZInteger RectangleShape::getPointCount() const {
    return 4;
}

math::Vector2f RectangleShape::getPoint(PZInteger aIndex) const {
    switch (aIndex) {
    default:
    case 0:
        return {0.f, 0.f};

    case 1:
        return {_size.x, 0.f};

    case 2:
        return {_size.x, _size.y};

    case 3:
        return {0.f, _size.y};
    }
}

[[nodiscard]] math::Vector2f RectangleShape::getGeometricCenter() const {
    return _size / 2.f;
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
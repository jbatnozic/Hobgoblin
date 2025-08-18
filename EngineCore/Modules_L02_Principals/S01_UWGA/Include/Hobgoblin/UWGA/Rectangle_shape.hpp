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

#ifndef UHOBGOBLIN_UWGA_RECTANGLE_SHAPE_HPP
#define UHOBGOBLIN_UWGA_RECTANGLE_SHAPE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Shape.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

//! \brief Specialized shape representing a rectangle.
class RectangleShape : public Shape {
public:
    //! \brief Default constructor.
    explicit RectangleShape(const System& aSystem, math::Vector2f aSize = {});

    //! Copy constructor.
    RectangleShape(const RectangleShape& aOther);

    //! Copy assignment.
    RectangleShape& operator=(const RectangleShape& aOther);

    //! Move constructor.
    RectangleShape(RectangleShape&& aOther) = default;

    //! Move assignment.
    RectangleShape& operator=(RectangleShape&& aOther) = default;

    //! \brief Set the size of the rectangle.
    void setSize(math::Vector2f aSize);

    //! \brief Get the size of the rectangle.
    [[nodiscard]] math::Vector2f getSize() const;

    //! \brief Get the number of points of the circle
    //!
    //! \return Number of points of the circle
    //!
    //! \see setPointCount
    PZInteger getPointCount() const override;

    //! \brief Get a point of the circle
    //!
    //! The returned point is in local coordinates, that is,
    //! the shape's transforms (position, rotation, scale) are
    //! not taken into account.
    //! The result is undefined if \a index is out of the valid range.
    //!
    //! \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    //!
    //! \return index-th point of the shape
    math::Vector2f getPoint(PZInteger aIndex) const override;

    //! \brief Get the geometric center of the shape
    //!
    //! The returned point is in local coordinates, that is,
    //! the shape's transforms (position, rotation, scale) are
    //! not taken into account.
    //!
    //! \return The geometric center of the shape
    [[nodiscard]] math::Vector2f getGeometricCenter() const override;

private:
    math::Vector2f _size;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_RECTANGLE_SHAPE_HPP

////////////////////////////////////////////////////////////
/// \class sf::RectangleShape
/// \ingroup graphics
///
/// This class inherits all the functions of `sf::Transformable`
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of `sf::Shape` (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::RectangleShape rectangle;
/// rectangle.setSize(sf::Vector2f(100, 50));
/// rectangle.setOutlineColor(sf::Color::Red);
/// rectangle.setOutlineThickness(5);
/// rectangle.setPosition({10, 20});
/// ...
/// window.draw(rectangle);
/// \endcode
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::ConvexShape`
///
////////////////////////////////////////////////////////////

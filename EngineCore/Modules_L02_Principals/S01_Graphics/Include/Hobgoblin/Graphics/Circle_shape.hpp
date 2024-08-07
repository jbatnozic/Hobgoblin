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

// clang-format off

#ifndef UHOBGOBLIN_GRAPHICS_CIRCLE_SHAPE_HPP
#define UHOBGOBLIN_GRAPHICS_CIRCLE_SHAPE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Graphics/Shape.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

//! \brief Specialized shape representing a circle.
class CircleShape : public Shape {
public:
    //! \brief Default constructor
    //!
    //! \param radius     Radius of the circle
    //! \param pointCount Number of points composing the circle
    explicit CircleShape(float aRadius = 0, PZInteger aPointCount = 32);

    //! \brief Set the radius of the circle
    //!
    //! \param radius New radius of the circle
    //!
    //! \see getRadius
    void setRadius(float aRadius);

    //! \brief Get the radius of the circle
    //!
    //! \return Radius of the circle
    //!
    //! \see setRadius
    float getRadius() const;

    //! \brief Set the number of points of the circle
    //!
    //! \param count New number of points of the circle
    //!
    //! \see getPointCount
    void setPointCount(PZInteger aCount);

    //! \brief Get the number of points of the circle
    //!
    //! \return Number of points of the circle
    //!
    //! \see setPointCount
    virtual PZInteger getPointCount() const;

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
    virtual math::Vector2f getPoint(PZInteger aIndex) const;

private:
    float     _radius;     //!< Radius of the circle
    PZInteger _pointCount; //!< Number of points composing the circle
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_GRAPHICS_CIRCLE_SHAPE_HPP

////////////////////////////////////////////////////////////
/// \class sf::CircleShape
/// \ingroup graphics
///
/// This class inherits all the functions of sf::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of sf::Shape (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::CircleShape circle;
/// circle.setRadius(150);
/// circle.setOutlineColor(sf::Color::Red);
/// circle.setOutlineThickness(5);
/// circle.setPosition(10, 20);
/// ...
/// window.draw(circle);
/// \endcode
///
/// Since the graphics card can't draw perfect circles, we have to
/// fake them with multiple triangles connected to each other. The
/// "points count" property of sf::CircleShape defines how many of these
/// triangles to use, and therefore defines the quality of the circle.
///
/// The number of points can also be used for another purpose; with
/// small numbers you can create any regular polygon shape:
/// equilateral triangle, square, pentagon, hexagon, ...
///
/// \see sf::Shape, sf::RectangleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////

// clang-format on

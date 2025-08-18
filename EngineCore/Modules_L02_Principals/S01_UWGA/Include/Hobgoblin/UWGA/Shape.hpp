// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2018 Laurent Gomila (laurent@sfml-dev.org)
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

#ifndef UHOBGOBLIN_UWGA_SHAPE_HPP
#define UHOBGOBLIN_UWGA_SHAPE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>
#include <Hobgoblin/UWGA/Transformable.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Texture;
class System;

//! \brief Base class for textured shapes with outline.
//!
//! \warning though it may seem counterintuitive, shapes aren't very memory-efficient. Try to reuse
//!          existing Shape objects as much as possible, though keep in mind that changing the shape's
//!          points (not its transform, but actual points) can also include expensive recalculations
//!          if there are many points to work with.
class Shape
    : virtual public Element
    , public Drawable
    , public Transformable {
public:
    Shape(const System& aSystem);

    const System& getSystem() const override;

    // MARK: Texture

    //! \brief Change the source texture of the shape
    //!
    //! The `texture` argument refers to a texture that must
    //! exist as long as the shape uses it. Indeed, the shape
    //! doesn't store its own copy of the texture, but rather keeps
    //! a pointer to the one that you passed to this function.
    //! If the source texture is destroyed and the shape tries to
    //! use it, the behavior is undefined.
    //! `texture` can be a null pointer to disable texturing.
    //! If `resetRect` is `true`, the `TextureRect` property of
    //! the shape is automatically adjusted to the size of the new
    //! texture. If it is `false`, the texture rect is left unchanged.
    //!
    //! \param aTexture   New texture
    //! \param aResetRect Should the texture rect be reset to the size of the new texture?
    //!
    //! \see `getTexture`, `setTextureRect`
    void setTexture(const Texture* aTexture, bool aResetRect = false);

    //! \brief Get the source texture of the shape
    //!
    //! If the shape has no source texture, a `nullptr` is returned.
    //! The returned pointer is const, which means that you can't
    //! modify the texture when you retrieve it with this function.
    //!
    //! \return Pointer to the shape's texture
    //!
    //! \see `setTexture`
    [[nodiscard]] const Texture* getTexture() const;

    //! \brief Set the sub-rectangle of the texture that the shape will display
    //!
    //! The texture rect is useful when you don't want to display
    //! the whole texture, but rather a part of it.
    //! By default, the texture rect covers the entire texture.
    //!
    //! \param aTextureRect Rectangle defining the region of the texture to display
    //!
    //! \see `getTextureRect`, `setTexture`
    void setTextureRect(TextureRect aTextureRect);

    //! \brief Get the sub-rectangle of the texture displayed by the shape
    //!
    //! \return Texture rectangle of the shape
    //!
    //! \see `setTextureRect`
    [[nodiscard]] TextureRect getTextureRect() const;

    // MARK: Fill

    //! \brief Set the fill color of the shape
    //!
    //! This color is modulated (multiplied) with the shape's
    //! texture if any. It can be used to colorize the shape,
    //! or change its global opacity.
    //! You can use `sf::Color::Transparent` to make the inside of
    //! the shape transparent, and have the outline alone.
    //! By default, the shape's fill color is opaque white.
    //!
    //! \param aColor New color of the shape
    //!
    //! \see `getFillColor`, `setOutlineColor`
    void setFillColor(Color aColor);

    //! \brief Get the fill color of the shape
    //!
    //! \return Fill color of the shape
    //!
    //! \see `setFillColor`
    [[nodiscard]] Color getFillColor() const;

    // MARK: Outline

    //! \brief Set the outline color of the shape
    //!
    //! By default, the shape's outline color is opaque white.
    //!
    //! \param aColor New outline color of the shape
    //!
    //! \see `getOutlineColor`, `setFillColor`
    void setOutlineColor(Color aColor);

    //! \brief Get the outline color of the shape
    //!
    //! \return Outline color of the shape
    //!
    //! \see `setOutlineColor`
    [[nodiscard]] Color getOutlineColor() const;

    //! \brief Set the thickness of the shape's outline
    //!
    //! Note that negative values are allowed (so that the outline
    //! expands towards the center of the shape), and using zero
    //! disables the outline.
    //! By default, the outline thickness is 0.
    //!
    //! \param aThickness New outline thickness
    //!
    //! \see `getOutlineThickness`
    void setOutlineThickness(float aThickness);

    //! \brief Get the outline thickness of the shape
    //!
    //! \return Outline thickness of the shape
    //!
    //! \see `setOutlineThickness`
    [[nodiscard]] float getOutlineThickness() const;

    //! \brief Set the limit on the ratio between miter length and outline thickness
    //!
    //! Outline segments around each shape corner are joined either
    //! with a miter or a bevel join.
    //! - A miter join is formed by extending outline segments until
    //!   they intersect. The distance between the point of
    //!   intersection and the shape's corner is the miter length.
    //! - A bevel join is formed by connecting outline segments with
    //!   a straight line perpendicular to the corner's bissector.
    //!
    //! The miter limit is used to determine whether ouline segments
    //! around a corner are joined with a bevel or a miter.
    //! When the ratio between the miter length and outline thickness
    //! exceeds the miter limit, a bevel is used instead of a miter.
    //!
    //! The miter limit is linked to the maximum inner angle of a
    //! corner below which a bevel is used by the following formula:
    //!
    //! miterLimit = 1 / sin(angle / 2)
    //!
    //! The miter limit must be greater than or equal to 1.
    //! By default, the miter limit is 10.
    //!
    //! \param aMiterLimit New miter limit
    //!
    //! \see getMiterLimit
    void setMiterLimit(float aMiterLimit);

    //! \brief Get the limit on the ratio between miter length and outline thickness
    //!
    //! \return Limit on the ratio between miter length and outline thickness
    //!
    //! \see setMiterLimit
    [[nodiscard]] float getMiterLimit() const;

    // MARK: Points & Bounds

    //! \brief Get the total number of points of the shape
    //!
    //! \return Number of points of the shape
    //!
    //! \see `getPoint`
    [[nodiscard]] virtual PZInteger getPointCount() const = 0;

    //! \brief Get a point of the shape
    //!
    //! The returned point is in local coordinates, that is,
    //! the shape's transforms (position, rotation, scale) are
    //! not taken into account.
    //! The result is undefined if `index` is out of the valid range.
    //!
    //! \param aIndex Index of the point to get, in range [0 .. getPointCount() - 1]
    //!
    //! \return `index`-th point of the shape
    //!
    //! \see `getPointCount`
    [[nodiscard]] virtual math::Vector2f getPoint(PZInteger aIndex) const = 0;

    //! \brief Get the geometric center of the shape
    //!
    //! The returned point is in local coordinates, that is,
    //! the shape's transforms (position, rotation, scale) are
    //! not taken into account.
    //!
    //! \return The geometric center of the shape
    [[nodiscard]] virtual math::Vector2f getGeometricCenter() const;

    //! \brief Get the local bounding rectangle of the entity
    //!
    //! The returned rectangle is in local coordinates, which means
    //! that it ignores the transformations (translation, rotation,
    //! scale, ...) that are applied to the entity.
    //! In other words, this function returns the bounds of the
    //! entity in the entity's coordinate system.
    //!
    //! \return Local bounding rectangle of the entity
    [[nodiscard]] math::Rectangle<float> getLocalBounds() const;

    //! \brief Get the global (non-minimal) bounding rectangle of the entity
    //!
    //! The returned rectangle is in global coordinates, which means
    //! that it takes into account the transformations (translation,
    //! rotation, scale, ...) that are applied to the entity.
    //! In other words, this function returns the bounds of the
    //! shape in the global 2D world's coordinate system.
    //!
    //! This function does not necessarily return the _minimal_
    //! bounding rectangle. It merely ensures that the returned
    //! rectangle covers all the vertices (but possibly more).
    //! This allows for a fast approximation of the bounds as a
    //! first check; you may want to use more precise checks
    //! on top of that.
    //!
    //! \return Global bounding rectangle of the entity
    [[nodiscard]] math::Rectangle<float> getGlobalBounds() const;

    // MARK: Drawable

    void drawOnto(Canvas& aCanvas, const RenderStates& aRenderStates) const override;

protected:
    //! \brief Flag to recompute internal geometry of the shape when needed
    //!
    //! This field must be set to true by the derived class every time
    //! the shape's points change (i.e. the result of either
    //! getPointCount or getPoint is different).
    mutable bool _pointsDirty = true;

    //! Helper function to use in implementations of copy constructors/assignment operators of
    //! derived classes. After calling this function, all you need to do is copy over the data
    //! that's specific to the derived class.
    void _copyShapeDataFrom(const Shape& aOther);

private:
    void _update() const;
    void _updateIfNeeded() const;
    void _updateFillColors() const;
    void _updateTexCoords() const;
    void _updateOutline() const;
    void _updateOutlineColors() const;

    using FloatRect = math::Rectangle<float>;

    // clang-format off
    mutable bool   _outlineDirty{true};
    const Texture* _texture{nullptr};          //!< Texture of the shape
    TextureRect    _textureRect;               //!< Rectangle defining the area of the source texture to display
    Color          _fillColor{COLOR_WHITE};    //!< Fill color
    Color          _outlineColor{COLOR_WHITE}; //!< Outline color
    float          _outlineThickness{0.f};     //!< Thickness of the shape's outline
    float          _miterLimit{10.f};          //!< Limit on the ratio between miter length and outline thickness

    mutable VertexArray _vertices{PrimitiveType::TRIANGLE_FAN};          //!< Vertex array containing the fill geometry
    mutable VertexArray _outlineVertices{PrimitiveType::TRIANGLE_STRIP}; //!< Vertex array containing the outline geometry

    mutable FloatRect _insideBounds;           //!< Bounding rectangle of the inside (fill)
    mutable FloatRect _bounds;                 //!< Bounding rectangle of the whole shape (outline + fill)

    //! Cached Transform object to use to combine the passed transform with this instance's Transform
    //! during drawing in case a non-null Transform is passed as a part of render states
    mutable std::unique_ptr<Transform> _drawTransform;
    // clang-format on
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SHAPE_HPP

////////////////////////////////////////////////////////////
/// \class sf::Shape
/// \ingroup graphics
///
/// sf::Shape is a drawable class that allows to define and
/// display a custom convex shape on a render target.
/// It's only an abstract base, it needs to be specialized for
/// concrete types of shapes (circle, rectangle, convex polygon,
/// star, ...).
///
/// In addition to the attributes provided by the specialized
/// shape classes, a shape always has the following attributes:
/// \li a texture
/// \li a texture rectangle
/// \li a fill color
/// \li an outline color
/// \li an outline thickness
///
/// Each feature is optional, and can be disabled easily:
/// \li the texture can be null
/// \li the fill/outline colors can be sf::Color::Transparent
/// \li the outline thickness can be zero
///
/// You can write your own derived shape class, there are only
/// two virtual functions to override:
/// \li getPointCount must return the number of points of the shape
/// \li getPoint must return the points of the shape
///
/// \see sf::RectangleShape, sf::CircleShape, sf::ConvexShape, sf::Transformable
///
////////////////////////////////////////////////////////////

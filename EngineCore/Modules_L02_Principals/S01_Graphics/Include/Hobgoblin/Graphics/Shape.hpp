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

// clang-format off

#ifndef UHOBGOBLIN_GRAPHICS_SHAPE_HPP
#define UHOBGOBLIN_GRAPHICS_SHAPE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Graphics/Color.hpp>
#include <Hobgoblin/Graphics/Drawable.hpp>
#include <Hobgoblin/Graphics/Transformable.hpp>
#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <cstddef>
#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

namespace detail {
class ShapePolymorphismAdapter {
public:
    virtual ~ShapePolymorphismAdapter() = default;
    virtual PZInteger getPointCount() const = 0;
    virtual math::Vector2f getPoint(PZInteger aIndex) const = 0;
};
} // namespace detail

class Texture;

//! \brief Base class for textured shapes with outline.
class Shape : public Drawable, public Transformable, private detail::ShapePolymorphismAdapter {
public:
    Shape(const Shape& aOther);
    Shape& operator=(const Shape& aOther);

    Shape(Shape&& aOther);
    Shape& operator=(Shape&& aOther);

    //! \brief Virtual destructor
    virtual ~Shape();

    //! \brief Change the source texture of the shape
    //!
    //! The \a texture argument refers to a texture that must
    //! exist as long as the shape uses it. Indeed, the shape
    //! doesn't store its own copy of the texture, but rather keeps
    //! a pointer to the one that you passed to this function.
    //! If the source texture is destroyed and the shape tries to
    //! use it, the behavior is undefined.
    //! \a texture can be NULL to disable texturing.
    //! If \a resetRect is true, the TextureRect property of
    //! the shape is automatically adjusted to the size of the new
    //! texture. If it is false, the texture rect is left unchanged.
    //!
    //! \param texture   New texture
    //! \param resetRect Should the texture rect be reset to the size of the new texture?
    //!
    //! \see getTexture, setTextureRect
    void setTexture(const Texture* aTexture, bool aResetRect = false);

    //! \brief Set the sub-rectangle of the texture that the shape will display
    //!
    //! The texture rect is useful when you don't want to display
    //! the whole texture, but rather a part of it.
    //! By default, the texture rect covers the entire texture.
    //!
    //! \param rect Rectangle defining the region of the texture to display
    //!
    //! \see getTextureRect, setTexture
    void setTextureRect(const math::Rectangle<PZInteger>& aRect);

    //! \brief Set the fill color of the shape
    //!
    //! This color is modulated (multiplied) with the shape's
    //! texture if any. It can be used to colorize the shape,
    //! or change its global opacity.
    //! You can use sf::Color::Transparent to make the inside of
    //! the shape transparent, and have the outline alone.
    //! By default, the shape's fill color is opaque white.
    //!
    //! \param color New color of the shape
    //!
    //! \see getFillColor, setOutlineColor
    void setFillColor(const Color& aColor);

    //! \brief Set the outline color of the shape
    //!
    //! By default, the shape's outline color is opaque white.
    //!
    //! \param color New outline color of the shape
    //!
    //! \see getOutlineColor, setFillColor
    void setOutlineColor(const Color& aColor);

    //! \brief Set the thickness of the shape's outline
    //!
    //! Note that negative values are allowed (so that the outline
    //! expands towards the center of the shape), and using zero
    //! disables the outline.
    //! By default, the outline thickness is 0.
    //!
    //! \param thickness New outline thickness
    //!
    //! \see getOutlineThickness
    void setOutlineThickness(float aThickness);

    //! \brief Get the source texture of the shape
    //!
    //! If the shape has no source texture, a NULL pointer is returned.
    //! The returned pointer is const, which means that you can't
    //! modify the texture when you retrieve it with this function.
    //!
    //! \return Pointer to the shape's texture
    //!
    //! \see setTexture
    const Texture* getTexture() const;

    //! \brief Get the sub-rectangle of the texture displayed by the shape
    //!
    //! \return Texture rectangle of the shape
    //!
    //! \see setTextureRect
    math::Rectangle<PZInteger> getTextureRect() const;

    //! \brief Get the fill color of the shape
    //!
    //! \return Fill color of the shape
    //!
    //! \see setFillColor
    Color getFillColor() const;

    //! \brief Get the outline color of the shape
    //!
    //! \return Outline color of the shape
    //!
    //! \see setOutlineColor
    Color getOutlineColor() const;

    //! \brief Get the outline thickness of the shape
    //!
    //! \return Outline thickness of the shape
    //!
    //! \see setOutlineThickness
    float getOutlineThickness() const;

    //! \brief Get the total number of points of the shape
    //!
    //! \return Number of points of the shape
    //!
    //! \see getPoint
    virtual PZInteger getPointCount() const override = 0;

    //! \brief Get a point of the shape
    //!
    //! The returned point is in local coordinates, that is,
    //! the shape's transforms (position, rotation, scale) are
    //! not taken into account.
    //! The result is undefined if \a index is out of the valid range.
    //!
    //! \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    //!
    //! \return index-th point of the shape
    //!
    //! \see getPointCount
    virtual math::Vector2f getPoint(PZInteger aTndex) const override = 0;

    //! \brief Get the local bounding rectangle of the entity
    //!
    //! The returned rectangle is in local coordinates, which means
    //! that it ignores the transformations (translation, rotation,
    //! scale, ...) that are applied to the entity.
    //! In other words, this function returns the bounds of the
    //! entity in the entity's coordinate system.
    //!
    //! \return Local bounding rectangle of the entity
    math::Rectangle<float> getLocalBounds() const;

    //! \brief Get the global (non-minimal) bounding rectangle of the entity
    //!
    //! The returned rectangle is in global coordinates, which means
    //! that it takes into account the transformations (translation,
    //! rotation, scale, ...) that are applied to the entity.
    //! In other words, this function returns the bounds of the
    //! shape in the global 2D world's coordinate system.
    //!
    //! This function does not necessarily return the \a minimal
    //! bounding rectangle. It merely ensures that the returned
    //! rectangle covers all the vertices (but possibly more).
    //! This allows for a fast approximation of the bounds as a
    //! first check; you may want to use more precise checks
    //! on top of that.
    //!
    //! \return Global bounding rectangle of the entity
    math::Rectangle<float> getGlobalBounds() const;

    ///////////////////////////////////////////////////////////////////////////
    // DRAWABLE                                                              //
    ///////////////////////////////////////////////////////////////////////////

    BatchingType getBatchingType() const override final;

    ///////////////////////////////////////////////////////////////////////////
    // TRANSFORMABLE                                                         //
    ///////////////////////////////////////////////////////////////////////////

    void setPosition(float aX, float aY) override;

    void setPosition(const math::Vector2f& aPosition) override;

    void setRotation(math::AngleF aAngle) override;

    void setScale(float aFactorX, float aFactorY) override;

    void setScale(const math::Vector2f& aFactors) override;

    void setOrigin(float aX, float aY) override;

    void setOrigin(const math::Vector2f& aOrigin) override;

    math::Vector2f getPosition() const override;

    math::AngleF getRotation() const override;

    math::Vector2f getScale() const override;

    math::Vector2f getOrigin() const override;

    void move(float aOffsetX, float aOffsetY) override;

    void move(const math::Vector2f& aOffset) override;

    void rotate(math::AngleF aAngle) override;

    void scale(float aFactorX, float aFactorY) override;

    void scale(const math::Vector2f& aFactor) override;

    Transform getTransform() const override;

    Transform getInverseTransform() const override;

protected:
    //! \brief Default constructor
    Shape();

    //! \brief Recompute the internal geometry of the shape
    //!
    //! This function must be called by the derived class everytime
    //! the shape's points change (i.e. the result of either
    //! getPointCount or getPoint is different).
    void _update();

private:
    void* _getSFMLImpl();
    const void* _getSFMLImpl() const;

#if HG_BUILD_TYPE == HG_DEBUG && defined(_MSC_VER)
    static constexpr std::size_t STORAGE_SIZE = 360;
#else
    static constexpr std::size_t STORAGE_SIZE = 344;
#endif

    static constexpr std::size_t STORAGE_ALIGN = 8;

    const Texture* _texture;
    std::aligned_storage<STORAGE_SIZE, STORAGE_ALIGN>::type _storage;

    void _drawOnto(Canvas& aCanvas, const RenderStates& aStates) const override;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_GRAPHICS_SHAPE_HPP

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

// clang-format on

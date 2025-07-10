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

#ifndef UHOBGOBLIN_UWGA_TRANSFORM_HPP
#define UHOBGOBLIN_UWGA_TRANSFORM_HPP

#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <Hobgoblin/UWGA/Element.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! \brief Define a 3x3 transform matrix.
class Transform : virtual public Element {
public:
    //! \brief Virtual destructor.
    virtual ~Transform() = default;

    //! \brief Make and return a copy of this object.
    virtual std::unique_ptr<Transform> clone() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SETTERS                                                         //
    ///////////////////////////////////////////////////////////////////////////

    // clang-format off
    //! \brief Reset the whole state of the transform from a 3x3 matrix.
    //!
    //! \param a00 Element (0, 0) of the matrix.
    //! \param a01 Element (0, 1) of the matrix.
    //! \param a02 Element (0, 2) of the matrix.
    //! \param a10 Element (1, 0) of the matrix.
    //! \param a11 Element (1, 1) of the matrix.
    //! \param a12 Element (1, 2) of the matrix.
    //! \param a20 Element (2, 0) of the matrix.
    //! \param a21 Element (2, 1) of the matrix.
    //! \param a22 Element (2, 2) of the matrix.
    virtual void setTo3x3Matrix(float a00, float a01, float a02,
                                float a10, float a11, float a12,
                                float a20, float a21, float a22) = 0;
    // clang-format on

    //! \brief Reset the whole state of the transform back into an identity (transform which does
    //!        nothing).
    virtual void setToIdentity() = 0;

    //! \brief Set the whole state of the transform into an inverse of its current self.
    virtual void setToInverse() = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: APPLYING TO OBJECTS                                             //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Transform a 2D point by applying this transformation to it.
    virtual math::Vector2f transformPoint(float aX, float aY) const = 0;

    //! \brief Transform a 2D point by applying this transformation to it.
    virtual math::Vector2f transformPoint(const math::Vector2f& aPoint) const = 0;

    //! \brief Transform a 2D rectangle by applying this transformation to it.
    //!
    //! \warning Since oriented rectangles are NOT supported, the result of this function is
    //!          always an axis-aligned rectangle. That means that the bounding rectangle of the
    //!          transformed rectangle is returned (though they could be equivalent).
    virtual math::Rectangle<float> transformRect(const math::Rectangle<float>& aRectangle) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: MUTATORS                                                        //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Combine the current transform with another one.
    //!
    //! The result is a transform that is equivalent to applying
    //! \a `aTransform` followed by `*this`. Mathematically, it is
    //! equivalent to a matrix multiplication `(*this) * aTransform`.
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& combine(const Transform& aOther) = 0;

    //! \brief Combine the current transform with a translation.
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& translate(float aX, float aY) = 0;

    //! \brief Combine the current transform with a translation.
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& translate(math::Vector2f aOffset) = 0;

    //! \brief Combine the current transform with a rotation.
    //!
    //! \param aAngle angle in degrees.
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& rotate(float aAngle) = 0;

    //! \brief Combine the current transform with a rotation.
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& rotate(math::AngleF aAngle) = 0;

    //! \brief Combine the current transform with a rotation around an arbitrary center of rotation.
    //!
    //! The center of rotation is provided for convenience as a second argument, so that you can 
    //! build rotations around arbitrary points more easily (and efficiently) than the usual
    //! `translate(-center).rotate(angle).translate(center)`.
    //!
    //! \param aAngle Rotation angle, in degrees
    //! \param aCenterX X coordinate of the center of rotation
    //! \param aCenterY Y coordinate of the center of rotation
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& rotate(float aAngle, float aCenterX, float aCenterY) = 0;

    //! \brief Combine the current transform with a rotation around an arbitrary center of rotation.
    //!
    //! The center of rotation is provided for convenience as a second argument, so that you can 
    //! build rotations around arbitrary points more easily (and efficiently) than the usual
    //! `translate(-center).rotate(angle).translate(center)`.
    //!
    //! \param aAngle Rotation angle
    //! \param aCenterX X coordinate of the center of rotation
    //! \param aCenterY Y coordinate of the center of rotation
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& rotate(math::AngleF aAngle, float aCenterX, float aCenterY) = 0;

    //! \brief Combine the current transform with a rotation around an arbitrary center of rotation.
    //!
    //! The center of rotation is provided for convenience as a second argument, so that you can 
    //! build rotations around arbitrary points more easily (and efficiently) than the usual
    //! `translate(-center).rotate(angle).translate(center)`.
    //!
    //! \param aAngle Rotation angle, in degrees
    //! \param aCenter Center of rotation
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& rotate(float aAngle, math::Vector2f aCenter) = 0;

    //! \brief Combine the current transform with a rotation around an arbitrary center of rotation.
    //!
    //! The center of rotation is provided for convenience as a second argument, so that you can 
    //! build rotations around arbitrary points more easily (and efficiently) than the usual
    //! `translate(-center).rotate(angle).translate(center)`.
    //!
    //! \param aAngle Rotation angle
    //! \param aCenter Center of rotation
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& rotate(math::AngleF aAngle, math::Vector2f aCenter) = 0;

    //! \brief Combine the current transform with a scaling.
    //!
    //! \param aScaleX Scaling factor on the X axis
    //! \param aScaleY Scaling factor on the Y axis
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& scale(float aScaleX, float aScaleY) = 0;

    //! \brief Combine the current transform with a scaling around an arbitrary center of scaling.
    //!
    //! The center of scaling is provided for convenience as a second argument, so that you can
    //! build scaling around arbitrary points more easily (and efficiently) than the usual
    //! `translate(-center).scale(factors).translate(center)`.
    //!
    //! \param aScaleX Scaling factor on X axis
    //! \param aScaleY Scaling factor on Y axis
    //! \param aCenterX X coordinate of the center of scaling
    //! \param aCenterY Y coordinate of the center of scaling
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& scale(float aScaleX, float aScaleY, float aCenterX, float aCenterY) = 0;

    //! \brief Combine the current transform with a scaling.
    //!
    //! \param aFactors Scaling factors
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& scale(const math::Vector2f& aFactors) = 0;

    //! \brief Combine the current transform with a scaling around an arbitrary center of scaling.
    //!
    //! The center of scaling is provided for convenience as a second argument, so that you can
    //! build scaling around arbitrary points more easily (and efficiently) than the usual
    //! `translate(-center).scale(factors).translate(center)`.
    //!
    //! \param aFactors Scaling factors
    //! \param aCenter Center of scaling
    //!
    //! \return a reference to `*this`, so that calls can be chained.
    virtual Transform& scale(const math::Vector2f& aFactors, const math::Vector2f& aCenter) = 0;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TRANSFORM_HPP

////////////////////////////////////////////////////////////
/// \class sf::Transform
/// \ingroup graphics
///
/// A sf::Transform specifies how to translate, rotate, scale,
/// shear, project, whatever things. In mathematical terms, it defines
/// how to transform a coordinate system into another.
///
/// For example, if you apply a rotation transform to a sprite, the
/// result will be a rotated sprite. And anything that is transformed
/// by this rotation transform will be rotated the same way, according
/// to its initial position.
///
/// Transforms are typically used for drawing. But they can also be
/// used for any computation that requires to transform points between
/// the local and global coordinate systems of an entity (like collision
/// detection).
///
/// Example:
/// \code
/// // define a translation transform
/// sf::Transform translation;
/// translation.translate(20, 50);
///
/// // define a rotation transform
/// sf::Transform rotation;
/// rotation.rotate(45);
///
/// // combine them
/// sf::Transform transform = translation * rotation;
///
/// // use the result to transform stuff...
/// sf::Vector2f point = transform.transformPoint(10, 20);
/// sf::FloatRect rect = transform.transformRect(sf::FloatRect(0, 0, 10, 100));
/// \endcode
///
/// \see sf::Transformable, sf::RenderStates
///
////////////////////////////////////////////////////////////

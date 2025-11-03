// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2025 Laurent Gomila (laurent@sfml-dev.org)
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

#ifndef UHOBGOBLIN_MATH_VECTOR2_HPP
#define UHOBGOBLIN_MATH_VECTOR2_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Hash.hpp>

#include <cassert>
#include <cmath>
#include <memory> // for std::hash
#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace math {

//! Concept that checks whether types `taFrom` and `taTo` are arithmetic, of compatible representations
//! (either both floating point or both integral), and convertible `taFrom`->`taTo` without loss of data
//! (without narrowing).
template <typename taFrom, typename taTo>
concept is_losslessly_convertible =
    (std::is_arithmetic_v<taFrom> && std::is_arithmetic_v<taTo>) &&
    (std::is_floating_point_v<taFrom> == std::is_floating_point_v<taTo>) &&
    requires(taFrom aFrom) { taTo{aFrom}; };

// MARK: Vector2

template <class T>
class Vector2 {
public:
    //! \brief Default constructor Creates a `Vector2(0, 0)`.
    constexpr Vector2() = default;

    //! \brief Construct the vector from cartesian coordinates
    //!
    //! \param x X coordinate
    //! \param y Y coordinate
    constexpr Vector2(T aX, T aY);

    //! Converts the vector to another type of vector provided that T is implicitly and losslessly
    //! convertible to U.
    template <class U>
        requires is_losslessly_convertible<T, U>
    constexpr operator Vector2<U>() const;

    //! Cast the vector into a different type into which it isn't implicitly convertible.
    //!
    //! Example:
    //!     Vector2<double> vecd = {1.0, 2.0};
    //!     Vector2<float>  vecf = {};
    //!     vecf = vecd; // error - double not convertible to float
    //!     vecf = vecd.cast<float>(); // OK
    template <class U>
    Vector2<U> cast() const {
        return {static_cast<U>(x), static_cast<U>(y)};
    }

    //! \brief Length of the vector <i><b>(floating-point)</b></i>.
    //!
    //! \note If you are not interested in the actual length, but only in comparisons,
    //!       consider using `lengthSquared()` (better for performance).
    [[nodiscard]] T length() const;

    //! \brief Square of vector's length.
    //!
    //! Suitable for comparisons, more efficient than `length()`.
    [[nodiscard]] constexpr T lengthSquared() const;

    //! \brief Vector with same direction but length 1 <i><b>(floating-point)</b></i>.
    //!
    //! \pre `*this` is no zero vector.
    [[nodiscard]] Vector2 normalized() const;

    //! \brief Projection of this vector onto `aAxis` <i><b>(floating-point)</b></i>.
    //!
    //! \param aAxis Vector being projected onto. Need not be normalized.
    //! \pre `aAxis` must not have length zero.
    [[nodiscard]] Vector2 projectedOnto(Vector2 aAxis) const;

    //! \brief Returns a perpendicular vector.
    //!
    //! Returns `*this` rotated by +90 degrees; (x,y) becomes (-y,x).
    //! For example, the vector (1,0) is transformed to (0,1).
    //!
    //! In Hobgoblin's default coordinate system with +X aRhs and +Y down,
    //! this amounts to a clockwise rotation.
    [[nodiscard]] constexpr Vector2 perpendicular() const;

    //! \brief Dot product of two 2D vectors.
    [[nodiscard]] constexpr T dot(Vector2 aRhs) const;

    //! \brief Z component of the cross product of two 2D vectors.
    //!
    //! Treats the operands as 3D vectors, computes their cross product
    //! and returns the result's Z component (X and Y components are always zero).
    [[nodiscard]] constexpr T cross(Vector2 aRhs) const;

    //! \brief Component-wise multiplication of `*this` and `aRhs`.
    //!
    //! Computes `(lhs.x*aRhs.x, lhs.y*aRhs.y)`.
    //!
    //! Scaling is the most common use case for component-wise multiplication/division.
    [[nodiscard]] constexpr Vector2 componentWiseMul(Vector2 aRhs) const;

    //! \brief Component-wise division of `*this` and `aRhs`.
    //!
    //! Computes `(lhs.x/aRhs.x, lhs.y/aRhs.y)`.
    //!
    //! Scaling is the most common use case for component-wise multiplication/division.
    //!
    //! \pre Neither component of `aRhs` is zero.
    [[nodiscard]] constexpr Vector2 componentWiseDiv(Vector2 aRhs) const;

    ///////////////////////////////////////////////////////////////////////////
    // MEMBERS                                                               //
    ///////////////////////////////////////////////////////////////////////////

    T x{}; //!< X coordinate of the vector
    T y{}; //!< Y coordinate of the vector
};

using Vector2pz = Vector2<PZInteger>;
using Vector2i  = Vector2<int>;
using Vector2u  = Vector2<unsigned>;
using Vector2f  = Vector2<float>;
using Vector2d  = Vector2<double>;

// MARK: Free operators

//! Unary `operator+`. Returns the vector as-is.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator+(Vector2<T> aRhs);

//! Unary `operator-`. Returns a member-wise opposite of the vector.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator-(Vector2<T> aRhs);

//! Binary `operator+=`. Performs a member-wise addition and stores the result in `aLhs`.
//! \returns reference to `aLhs`.
template <class T>
constexpr Vector2<T>& operator+=(Vector2<T>& aLhs, Vector2<T> aRhs);

//! Binary `operator-=`. Performs a member-wise subtraction and stores the result in `aLhs`.
//! \returns reference to `aLhs`.
template <class T>
constexpr Vector2<T>& operator-=(Vector2<T>& aLhs, Vector2<T> aRhs);

//! Binary `operator+`. Performs a member-wise addition and returns the result.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator+(Vector2<T> aLhs, Vector2<T> aRhs);

//! Binary `operator-`. Performs a member-wise subtraction and returns the result.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator-(Vector2<T> aLhs, Vector2<T> aRhs);

//! Binary `operator*`. Multiplies the vector `aLhs` by the sclar `aRhs` and returns the result.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator*(Vector2<T> aLhs, T aRhs);

//! Binary `operator*`. Multiplies the vector `aRhs` by the sclar `aLhs` and returns the result.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator*(T aLhs, Vector2<T> aRhs);

//! Binary `operator*`. Multiplies the vector `aLhs` by the sclar `aRhs` and stores the result in `aLhs`.
//! \returns reference to `aLhs`.
template <class T>
constexpr Vector2<T>& operator*=(Vector2<T>& aLhs, T aRhs);

//! Binary `operator/`. Does member-wise division of the vector `aLhs` by
//! the scalar `aRhs` and returns the result.
template <class T>
[[nodiscard]] constexpr Vector2<T> operator/(Vector2<T> aLhs, T aRhs);

//! Binary `operator/`. Does member-wise division of the vector `aLhs` by
//! the sclar `aRhs` and stores the result in `aLhs`.
//! \returns reference to `aLhs`.
template <class T>
constexpr Vector2<T>& operator/=(Vector2<T>& aLhs, T aRhs);

//! Equality operator (checks strict member-wise equality).
template <class T>
[[nodiscard]] constexpr bool operator==(Vector2<T> aLhs, Vector2<T> aRhs);

//! Inequality operator (checks strict member-wise inequality).
template <class T>
[[nodiscard]] constexpr bool operator!=(Vector2<T> aLhs, Vector2<T> aRhs);

// MARK: Vector2/Inline def.

template <class T>
constexpr Vector2<T>::Vector2(T x, T y)
    : x(x)
    , y(y) {}

template <class T>
template <class U>
    requires is_losslessly_convertible<T, U>
constexpr Vector2<T>::operator Vector2<U>() const {
    return Vector2<U>(static_cast<U>(x), static_cast<U>(y));
}

template <class T>
T Vector2<T>::length() const {
    static_assert(std::is_floating_point_v<T>,
                  "Vector2::length() is only supported for floating point types");

    return std::sqrt(lengthSquared());
}

template <class T>
constexpr T Vector2<T>::lengthSquared() const {
    return dot(*this);
}

template <class T>
Vector2<T> Vector2<T>::normalized() const {
    static_assert(std::is_floating_point_v<T>,
                  "Vector2::normalized() is only supported for floating point types");

    assert(*this != Vector2<T>() && "Vector2::normalized() cannot normalize a zero vector");

    return (*this) / length();
}

template <typename T>
Vector2<T> Vector2<T>::projectedOnto(Vector2<T> aAxis) const {
    static_assert(std::is_floating_point_v<T>,
                  "Vector2::projectedOnto() is only supported for floating point types");

    assert(aAxis != Vector2<T>() && "Vector2::projectedOnto() cannot project onto a zero vector");

    return dot(aAxis) / aAxis.lengthSquared() * aAxis;
}

template <class T>
constexpr Vector2<T> Vector2<T>::perpendicular() const {
    return Vector2<T>(-y, x);
}

template <class T>
constexpr T Vector2<T>::dot(Vector2<T> aRhs) const {
    return x * aRhs.x + y * aRhs.y;
}

template <class T>
constexpr T Vector2<T>::cross(Vector2<T> aRhs) const {
    return x * aRhs.y - y * aRhs.x;
}

template <class T>
constexpr Vector2<T> Vector2<T>::componentWiseMul(Vector2<T> aRhs) const {
    return Vector2<T>(x * aRhs.x, y * aRhs.y);
}

template <class T>
constexpr Vector2<T> Vector2<T>::componentWiseDiv(Vector2<T> aRhs) const {
    assert(aRhs.x != 0 && "Vector2::componentWiseDiv() cannot divide by 0");
    assert(aRhs.y != 0 && "Vector2::componentWiseDiv() cannot divide by 0");

    return Vector2<T>(x / aRhs.x, y / aRhs.y);
}

// MARK: Free operators/Inline def.

template <class T>
[[nodiscard]] constexpr Vector2<T> operator+(Vector2<T> aLhs) {
    return aLhs;
}

template <class T>
constexpr Vector2<T> operator-(Vector2<T> aLhs) {
    return Vector2<T>(-aLhs.x, -aLhs.y);
}

template <class T>
constexpr Vector2<T>& operator+=(Vector2<T>& aLhs, Vector2<T> aRhs) {
    aLhs.x += aRhs.x;
    aLhs.y += aRhs.y;

    return aLhs;
}

template <class T>
constexpr Vector2<T>& operator-=(Vector2<T>& aLhs, Vector2<T> aRhs) {
    aLhs.x -= aRhs.x;
    aLhs.y -= aRhs.y;

    return aLhs;
}

template <class T>
constexpr Vector2<T> operator+(Vector2<T> aLhs, Vector2<T> aRhs) {
    return Vector2<T>(aLhs.x + aRhs.x, aLhs.y + aRhs.y);
}

template <class T>
constexpr Vector2<T> operator-(Vector2<T> aLhs, Vector2<T> aRhs) {
    return Vector2<T>(aLhs.x - aRhs.x, aLhs.y - aRhs.y);
}

template <class T>
constexpr Vector2<T> operator*(Vector2<T> aLhs, T aRhs) {
    return Vector2<T>(aLhs.x * aRhs, aLhs.y * aRhs);
}

template <class T>
constexpr Vector2<T> operator*(T aLhs, Vector2<T> aRhs) {
    return Vector2<T>(aRhs.x * aLhs, aRhs.y * aLhs);
}

template <class T>
constexpr Vector2<T>& operator*=(Vector2<T>& aLhs, T aRhs) {
    aLhs.x *= aRhs;
    aLhs.y *= aRhs;

    return aLhs;
}

template <class T>
constexpr Vector2<T> operator/(Vector2<T> aLhs, T aRhs) {
    assert(aRhs != 0 && "Vector2::operator/ cannot divide by 0");

    return Vector2<T>(aLhs.x / aRhs, aLhs.y / aRhs);
}

template <class T>
constexpr Vector2<T>& operator/=(Vector2<T>& aLhs, T aRhs) {
    assert(aRhs != 0 && "Vector2::operator/= cannot divide by 0");

    aLhs.x /= aRhs;
    aLhs.y /= aRhs;

    return aLhs;
}

template <class T>
constexpr bool operator==(Vector2<T> aLhs, Vector2<T> aRhs) {
    return (aLhs.x == aRhs.x) && (aLhs.y == aRhs.y);
}

template <class T>
constexpr bool operator!=(Vector2<T> aLhs, Vector2<T> aRhs) {
    return !(aLhs == aRhs);
}

} // namespace math
HOBGOBLIN_NAMESPACE_END

// MARK: Hash function

template <class T>
struct std::hash<jbatnozic::hobgoblin::math::Vector2<T>> {
    std::size_t operator()(const jbatnozic::hobgoblin::math::Vector2<T>& aVec2) const noexcept {
        std::size_t seed = 0;
        jbatnozic::hobgoblin::math::HashCombine(seed, aVec2.x, aVec2.y);
        return seed;
    }
};

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_MATH_VECTOR2_HPP

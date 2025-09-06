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

#ifndef UHOBGOBLIN_MATH_VECTOR_HPP
#define UHOBGOBLIN_MATH_VECTOR_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace math {

///////////////////////////////////////////////////////////////////////////
// MARK: VECTOR 3                                                        //
///////////////////////////////////////////////////////////////////////////

using sf::Vector3;
using sf::Vector3i;
using Vector3u = Vector3<unsigned>;
using sf::Vector3f;
using Vector3pz = Vector3<PZInteger>;
using Vector3d  = Vector3<double>;

///////////////////////////////////////////////////////////////////////////
// MARK: VECTOR 4                                                        //
///////////////////////////////////////////////////////////////////////////

//! A 4-dimensional vector.
template <typename taArithmetic>
struct Vector4 {
    //! \brief Default constructor, creates a zero vector.
    Vector4()
        : x{static_cast<taArithmetic>(0)}
        , y{static_cast<taArithmetic>(0)}
        , z{static_cast<taArithmetic>(0)}
        , w{static_cast<taArithmetic>(0)} {}

    //! \brief Construct from 4 vector components.
    //!
    //! \param aX Component of the 4D vector.
    //! \param aY Component of the 4D vector.
    //! \param aZ Component of the 4D vector.
    //! \param aW Component of the 4D vector.
    Vector4(taArithmetic aX, taArithmetic aY, taArithmetic aZ, taArithmetic aW)
        : x{aX}
        , y{aY}
        , z{aZ}
        , w{aW} {}

    //! \brief Conversion constructor.
    //!
    //! \param other 4D vector of different type.
    template <typename taArithmetic2>
    explicit Vector4(const Vector4<taArithmetic2>& aOther)
        : x(static_cast<taArithmetic>(aOther.x))
        , y(static_cast<taArithmetic>(aOther.y))
        , z(static_cast<taArithmetic>(aOther.z))
        , w(static_cast<taArithmetic>(aOther.w)) {}

    taArithmetic x; //!< 1st component (X) of the 4D vector.
    taArithmetic y; //!< 2nd component (Y) of the 4D vector.
    taArithmetic z; //!< 3rd component (Z) of the 4D vector.
    taArithmetic w; //!< 4th component (W) of the 4D vector.
};

using Vector4i  = Vector4<int>;
using Vector4u  = Vector4<unsigned>;
using Vector4f  = Vector4<float>;
using Vector4pz = Vector4<PZInteger>;
using Vector4d  = Vector4<double>;

} // namespace math
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_MATH_VECTOR_HPP

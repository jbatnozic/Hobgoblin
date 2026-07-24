// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_COMMON_ENUM_OP_HPP
#define UHOBGOBLIN_COMMON_ENUM_OP_HPP

#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

// clang-format off

//! Use this macro to declare a binary arithmetic operator for an anum or enum class type.
//! \note you can prepend `[[nodiscard]]`, `inline`, `static` and other function modifiers if needed.
#define HG_ENUM_DECLARE_ARITHMETIC_OP(_enum_, _op_) \
    constexpr _enum_ operator _op_(_enum_ aLhs, _enum_ aRhs);

//! Use this macro to define a binary arithmetic operator for an anum or enum class type.
//! \note you can prepend `[[nodiscard]]`, `inline`, `static` and other function modifiers if needed.
#define HG_ENUM_DEFINE_ARITHMETIC_OP(_enum_, _op_)                                        \
    constexpr _enum_ operator _op_(_enum_ aLhs, _enum_ aRhs) {                            \
        return static_cast<_enum_>(static_cast<std::underlying_type_t<_enum_>>(aLhs) _op_ \
                                   static_cast<std::underlying_type_t<_enum_>>(aRhs));    \
    }

// clang-format on

//! \brief Test if a numeric or enum value is non-zero.
//! \returns `true` if the given numeric or enum value `aValue` is NOT equal to its zero equivalent;
//!          `false` otherwise.
//! \note the meaning of "zero equivalent" can be changed by explicitly setting the second template
//!       parameter.
//! \note `T` must have the binary operators `&` and `==` or the function won't compile.
template <class T,
          T aZeroValue = static_cast<T>(0),
          T_ENABLE_IF(std::is_arithmetic_v<T> || std::is_enum_v<T>)>
bool IsNonZero(T aValue) {
    return !(aValue == aZeroValue);
}

//! \brief Test if certain bits are set in a numeric or enum value.
//! \returns `true` if all of the bits in `aBits` are also set (1) in value `aValue`;
//!          `false` otherwise.
//! \note `T` must have the binary operators `&` and `==` or the function won't compile.
//! \warning if no bits are set in `aBits`, the function always returns `false`!
template <class T, T_ENABLE_IF(std::is_integral_v<T> || std::is_enum_v<T>)>
bool AreBitsSet(T aValue, T aBits) {
    return !((aValue & aBits) == static_cast<T>(0));
}

HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_COMMON_ENUM_OP_HPP

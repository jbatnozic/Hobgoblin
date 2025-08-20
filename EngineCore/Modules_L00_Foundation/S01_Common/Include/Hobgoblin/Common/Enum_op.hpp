// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_COMMON_ENUM_OP_HPP
#define UHOBGOBLIN_COMMON_ENUM_OP_HPP

#include <type_traits>

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

#endif // !UHOBGOBLIN_COMMON_ENUM_OP_HPP

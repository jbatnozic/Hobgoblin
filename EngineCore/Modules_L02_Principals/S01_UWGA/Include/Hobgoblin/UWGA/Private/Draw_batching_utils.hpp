// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_UTILS_FNL_HPP
#define UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_UTILS_FNL_HPP

#include <Hobgoblin/UWGA/Primitive_type.hpp>

#include <array>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {
namespace detail {

inline constexpr bool IsBatchable(PrimitiveType aPrimitiveType) {
    // POINTS         => YES
    // LINES          => YES
    // LINE_STRIP     => NO
    // TRIANGLES      => YES
    // TRIANGLE_STRIP => NO
    // TRIANGLE_FAN   => NO

    // clang-format off
    constexpr int MASK = (1 << static_cast<int>(PrimitiveType::POINTS   )) |
                         (1 << static_cast<int>(PrimitiveType::LINES    )) |
                         (1 << static_cast<int>(PrimitiveType::TRIANGLES)) ;
    // clang-format on

    return (MASK & (1 << static_cast<int>(aPrimitiveType)));
}

} // namespace detail
} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_UTILS_FNL_HPP

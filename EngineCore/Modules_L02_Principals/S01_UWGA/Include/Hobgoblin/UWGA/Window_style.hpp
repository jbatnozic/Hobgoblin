// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_WINDOW_STYLE_HPP
#define UHOBGOBLIN_UWGA_WINDOW_STYLE_HPP

#include <Hobgoblin/Common/Enum_op.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

enum class WindowStyle : int {
    NONE       = 0,      ///< No border / title bar (this flag and all others are mutually exclusive)
    TITLEBAR   = 1 << 0, ///< Title bar + fixed border
    RESIZE     = 1 << 1, ///< Title bar + resizable border + maximize button
    CLOSE      = 1 << 2, ///< Title bar + close button
    FULLSCREEN = 1 << 3, ///< Fullscreen mode (this flag and all others are mutually exclusive)

    DEFAULT = TITLEBAR | RESIZE | CLOSE ///< Default window style
};

[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(WindowStyle, |);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(WindowStyle, &);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(WindowStyle, ^);

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_WINDOW_STYLE_HPP

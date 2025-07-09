// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_WINDOW_STYLE_HPP
#define UHOBGOBLIN_UGE_WINDOW_STYLE_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

enum class WindowStyle : int {
    NONE       = 0,      ///< No border / title bar (this flag and all others are mutually exclusive)
    TITLEBAR   = 1 << 0, ///< Title bar + fixed border
    RESIZE     = 1 << 1, ///< Title bar + resizable border + maximize button
    CLOSE      = 1 << 2, ///< Title bar + close button
    FULLSCREEN = 1 << 3, ///< Fullscreen mode (this flag and all others are mutually exclusive)

    DEFAULT = TITLEBAR | RESIZE | CLOSE ///< Default window style
};

inline WindowStyle operator|(WindowStyle aLhs, WindowStyle aRhs) {
    return static_cast<WindowStyle>(static_cast<int>(aLhs) | static_cast<int>(aRhs));
}

inline WindowStyle operator&(WindowStyle aLhs, WindowStyle aRhs) {
    return static_cast<WindowStyle>(static_cast<int>(aLhs) & static_cast<int>(aRhs));
}

inline WindowStyle operator^(WindowStyle aLhs, WindowStyle aRhs) {
    return static_cast<WindowStyle>(static_cast<int>(aLhs) ^ static_cast<int>(aRhs));
}

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_WINDOW_STYLE_HPP

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_WINDOW_WINDOW_STYLE_HPP
#define UHOBGOBLIN_WINDOW_WINDOW_STYLE_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace win {

enum class WindowStyle : int {
    None       = 0,      ///< No border / title bar (this flag and all others are mutually exclusive)
    Titlebar   = 1 << 0, ///< Title bar + fixed border
    Resize     = 1 << 1, ///< Title bar + resizable border + maximize button
    Close      = 1 << 2, ///< Title bar + close button
    Fullscreen = 1 << 3, ///< Fullscreen mode (this flag and all others are mutually exclusive)

    Default = Titlebar | Resize | Close ///< Default window style
};

inline
WindowStyle operator|(WindowStyle aLhs, WindowStyle aRhs) {
    return static_cast<WindowStyle>(
        static_cast<int>(aLhs) | static_cast<int>(aRhs)
    );
}

inline
WindowStyle operator&(WindowStyle aLhs, WindowStyle aRhs) {
    return static_cast<WindowStyle>(
        static_cast<int>(aLhs) & static_cast<int>(aRhs)
    );
}

inline
WindowStyle operator^(WindowStyle aLhs, WindowStyle aRhs) {
    return static_cast<WindowStyle>(
        static_cast<int>(aLhs) ^ static_cast<int>(aRhs)
    );
}

} // namespace win
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_WINDOW_WINDOW_STYLE_HPP

// clang-format on

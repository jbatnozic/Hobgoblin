// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_WINDOW_WINDOW_HANDLE_HPP
#define UHOBGOBLIN_WINDOW_WINDOW_HANDLE_HPP

// TODO: This leaks SFML headers!
#include <SFML/Window/WindowHandle.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace win {

using WindowHandle = sf::WindowHandle;

} // namespace win
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_WINDOW_WINDOW_HANDLE_HPP

// clang-format on

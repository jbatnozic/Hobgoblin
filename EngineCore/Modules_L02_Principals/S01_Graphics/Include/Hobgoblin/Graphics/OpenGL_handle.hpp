// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_GR_OPENGL_HANDLE_HPP
#define UHOBGOBLIN_GR_OPENGL_HANDLE_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

using OpenGLHandle = unsigned int;

constexpr OpenGLHandle OPENGL_HANDLE_NONE = 0;

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_GR_OPENGL_HANDLE_HPP

// clang-format on

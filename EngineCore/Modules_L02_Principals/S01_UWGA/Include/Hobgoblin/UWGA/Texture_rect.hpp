// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_TEXTURE_RECT_HPP
#define UHOBGOBLIN_UWGA_TEXTURE_RECT_HPP

#include <Hobgoblin/Math/Rectangle.hpp>

#include <cstdint>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! An instance of TextureRect marks an area withing a Texture.
using TextureRect = math::Rectangle<std::uint16_t>;

static_assert(sizeof(TextureRect) <= sizeof(std::intptr_t), "TextureRect object too big.");

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TEXTURE_RECT_HPP

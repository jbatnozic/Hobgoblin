// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_GR_TGA_EXPORT_HPP
#define UHOBGOBLIN_GR_TGA_EXPORT_HPP

#include <Hobgoblin/Graphics/Texture.hpp>

#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

void ExportTextureToTgaFile(const Texture& aTexture, const std::string& aFilePath);

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_GR_TGA_EXPORT_HPP

// clang-format on

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SPRITE_MANIFEST_PARSER_HPP
#define UHOBGOBLIN_UWGA_SPRITE_MANIFEST_PARSER_HPP

#include <Hobgoblin/UWGA/Sprite_loader.hpp>

#include <filesystem>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

void ParseSpriteManifestFile(const std::filesystem::path&                aPath,
                             SpriteLoader&                               aSpriteLoader,
                             SpriteLoader::SpriteManifestEnumerationMap* aSpriteManifestEnumerationMap);

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SPRITE_MANIFEST_PARSER_HPP

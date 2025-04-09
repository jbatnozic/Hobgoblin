// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include "Resource_manager.hpp"

#include <filesystem>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace hg  = ::jbatnozic::hobgoblin;
namespace spe = ::jbatnozic::spempe;

class DefaultResourceManager : public ResourceManager {
public:
    DefaultResourceManager(const std::filesystem::path& aDefinitionsDir,
                           const std::filesystem::path& aSpritesDir);

    const hg::gr::SpriteLoader& getSpriteLoader() const override;

    const AllDefinitions& getAllDefinitions() const override;

private:
    hg::gr::SpriteLoader _spriteLoader;

    AllDefinitions _allDefinitions;

    void _loadSprites(const std::filesystem::path& aSpritesDir);
    void _loadAllDefinitions(const std::filesystem::path& aDefinitionsDir);
};

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
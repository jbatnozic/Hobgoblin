// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Resource_manager_default.hpp"

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

DefaultResourceManager::DefaultResourceManager(const std::filesystem::path& aDefinitionsDir,
                                               const std::filesystem::path& aSpritesDir) {}

const hg::gr::SpriteLoader& DefaultResourceManager::getSpriteLoader() const {
    return _spriteLoader;
}

const AllDefinitions& DefaultResourceManager::getAllDefinitions() const {
    return _allDefinitions;
}

void DefaultResourceManager::_loadSprites(const std::filesystem::path& aSpritesDir) {
    // TODO
}

void DefaultResourceManager::_loadAllDefinitions(const std::filesystem::path& aDefinitionsDir) {
    hg::UnicodeString contents;
    if (auto path = aDefinitionsDir / "cells+world.json"; std::filesystem::exists(path)) {
        contents = hg::LoadWholeFile(path);
    } else if (auto path = aDefinitionsDir / "cells+world.txt"; std::filesystem::exists(path)) {
        contents = hg::LoadWholeFile(path);
    } else {
        HG_THROW_TRACED(hg::TracedRuntimeError,
                        0,
                        "Neither 'cells+world.json' nor 'cells+world.txt' were found in '{}'.",
                        aDefinitionsDir.string());
    }

    _allDefinitions =
        JsonStringToAllDefinitions(hg::UniStrConv(hobgoblin::TO_UTF8_STD_STRING, contents));
}

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
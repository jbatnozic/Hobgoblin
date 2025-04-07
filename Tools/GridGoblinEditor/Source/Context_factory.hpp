// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <SPeMPE/SPeMPE.hpp>

#include <filesystem>
#include <memory>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace spe = ::jbatnozic::spempe;

struct EditorConfig {
    std::filesystem::path assetsDir;
    std::filesystem::path definitionsDir;
    std::filesystem::path spritesDir;
    std::filesystem::path worldCacheDir;
};

std::unique_ptr<spe::GameContext> CreateEditorSPeMPEContext(const EditorConfig& aConfig);

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic

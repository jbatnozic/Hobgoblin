// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/GridGoblin.hpp>
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace hg = ::jbatnozic::hobgoblin;

struct CellDefinition {
    std::optional<CellModel::Wall>  wall;
    std::optional<CellModel::Floor> floor;
    std::int64_t                    userData;
};

using CellDefinitions = std::unordered_map<std::string, CellDefinition>;

struct WorldDefinition {
    hg::PZInteger chunkCountX;
    hg::PZInteger chunkCountY;
    hg::PZInteger cellsPerChunkX;
    hg::PZInteger cellsPerChunkY;
    float         cellResolution;

    std::string defaultCellDefinition;
};

struct AllDefinitions {
    CellDefinitions cellDefinitions;
    WorldDefinition worldDefinition;
};

class JsonParseError : public hg::TracedRuntimeError {
public:
    using hg::TracedRuntimeError::TracedRuntimeError;
};

AllDefinitions JsonStringToAllDefinitions(std::string aJsonString);

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Building_block.hpp>

#include <sstream>

namespace jbatnozic {
namespace gridgoblin {

std::string BuildingBlockMaskToString(BuildingBlockMask aBbMask) {
    std::ostringstream oss;

    if ((aBbMask & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
        oss << "CELL_KIND_ID|";
    }
    if ((aBbMask & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
        oss << "FLOOR_SPRITE|";
    }
    if ((aBbMask & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
        oss << "WALL_SPRITE|";
    }
    if ((aBbMask & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
        oss << "SPATIAL_INFO|";
    }
    if ((aBbMask & BuildingBlock::RENDERER_AUX_DATA) != BuildingBlock::NONE) {
        oss << "RENDERER_AUX_DATA|";
    }
    if ((aBbMask & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
        oss << "USER_DATA|";
    }

    auto result = oss.str();
    if (result.empty()) {
        result = "NONE";
    } else {
        result.pop_back();
    }
    return result;
}

BuildingBlockMask StringToBuildingBlockMask(const std::string_view& aString) {
    BuildingBlockMask result = BuildingBlock::NONE;
    if (aString.empty()) {
        return result;
    }

    if (aString.find("CELL_KIND_ID") != std::string::npos) {
        result = result | BuildingBlock::CELL_KIND_ID;
    }
    if (aString.find("FLOOR_SPRITE") != std::string::npos) {
        result = result | BuildingBlock::FLOOR_SPRITE;
    }
    if (aString.find("WALL_SPRITE") != std::string::npos) {
        result = result | BuildingBlock::WALL_SPRITE;
    }
    if (aString.find("SPATIAL_INFO") != std::string::npos) {
        result = result | BuildingBlock::SPATIAL_INFO;
    }
    if (aString.find("RENDERER_AUX_DATA") != std::string::npos) {
        result = result | BuildingBlock::RENDERER_AUX_DATA;
    }
    if (aString.find("USER_DATA") != std::string::npos) {
        result = result | BuildingBlock::USER_DATA;
    }

    return result;
}

} // namespace gridgoblin
} // namespace jbatnozic

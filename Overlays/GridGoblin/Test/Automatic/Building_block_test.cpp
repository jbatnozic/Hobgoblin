// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Building_block.hpp>

#include <gtest/gtest.h>

namespace jbatnozic {
namespace gridgoblin {

TEST(BuildingBlockMaskConversionTest, ConvertNoneToStringAndBack) {
    const auto bb     = BuildingBlock::NONE;
    const auto string = BuildingBlockMaskToString(bb);
    EXPECT_EQ(string, "NONE");
    EXPECT_EQ(bb, StringToBuildingBlockMask(string));
}

TEST(BuildingBlockMaskConversionTest, ConvertSpatialInfoToStringAndBack) {
    const auto bb     = BuildingBlock::SPATIAL_INFO;
    const auto string = BuildingBlockMaskToString(bb);
    EXPECT_EQ(string, "SPATIAL_INFO");
    EXPECT_EQ(bb, StringToBuildingBlockMask(string));
}

TEST(BuildingBlockMaskConversionTest, ConvertMultipleInfoToStringAndBack) {
    const auto bb = BuildingBlock::FLOOR_SPRITE | BuildingBlock::SPATIAL_INFO | BuildingBlock::USER_DATA;
    const auto string = BuildingBlockMaskToString(bb);
    EXPECT_EQ(string, "FLOOR_SPRITE|SPATIAL_INFO|USER_DATA");
    EXPECT_EQ(bb, StringToBuildingBlockMask(string));
}

} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Private/Model_conversions.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <gtest/gtest.h>

#include <iostream>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

class GridGoblinConversionsTest : public ::testing::Test {
protected:
    json::Document _document;

    json::Document _jsonDocumentFromString(const char* aJsonString) {
        json::Document doc;
        doc.Parse(aJsonString);
        return doc;
    }

#if 1
    void _print(const json::Document& aDoc) {
        json::StringBuffer                     stringbuf;
        json::PrettyWriter<json::StringBuffer> writer(stringbuf);
        aDoc.Accept(writer);
        std::cerr << stringbuf.GetString() << std::endl;
    }

    void _print(const json::Value& aVal) {
        json::StringBuffer                     stringbuf;
        json::PrettyWriter<json::StringBuffer> writer(stringbuf);
        aVal.Accept(writer);
        std::cerr << stringbuf.GetString() << std::endl;
    }
#endif
};

TEST_F(GridGoblinConversionsTest, Cell_CellKindIdConversion) {
    cell::CellKindId cellKindId;
    cellKindId.value = 5;

    EXPECT_EQ(cellKindId, JsonToCellKindId(CellKindIdToJson(cellKindId, _document.GetAllocator())));
}

TEST_F(GridGoblinConversionsTest, Cell_FloorSpriteConversion) {
    cell::FloorSprite floorSprite;
    floorSprite.id = 7;

    EXPECT_EQ(floorSprite, JsonToFloorSprite(FloorSpriteToJson(floorSprite, _document.GetAllocator())));
}

TEST_F(GridGoblinConversionsTest, Cell_WallSpriteConversion) {
    cell::WallSprite wallSprite;
    wallSprite.id         = 11;
    wallSprite.id_reduced = 127;

    EXPECT_EQ(wallSprite, JsonToWallSprite(WallSpriteToJson(wallSprite, _document.GetAllocator())));
}

TEST_F(GridGoblinConversionsTest, Cell_SpatialInfoConversion) {
    cell::SpatialInfo spatialInfo;

    EXPECT_EQ(spatialInfo, JsonToSpatialInfo(SpatialInfoToJson(spatialInfo, _document.GetAllocator())));
}

TEST_F(GridGoblinConversionsTest, Cell_RendererAuxDataConversion) {
    cell::RendererAuxData rendererAuxData;

    EXPECT_EQ(rendererAuxData,
              JsonToRendererAuxData(RendererAuxDataToJson(rendererAuxData, _document.GetAllocator())));
}

TEST_F(GridGoblinConversionsTest, Cell_UserDataConversion) {
    cell::UserData userData;

    EXPECT_EQ(userData, JsonToUserData(UserDataToJson(userData, _document.GetAllocator())));
}

#if 0
TEST_F(GridGoblinConversionsTest, EmptyCellConversionFromJson) {
    static constexpr auto EMPTY_CELL_JSON = R"_({ "flags": 0 })_";

    const auto cell = JsonToCell(_jsonDocumentFromString(EMPTY_CELL_JSON));

    const CellModel emptyCell{};

    EXPECT_EQ(cell, emptyCell);
}

TEST_F(GridGoblinConversionsTest, CellConversionFromInvalidJson) {
    static constexpr auto INVALID_CELL_JSON_1 = R"_({ "flags": "asdf" })_";
    static constexpr auto INVALID_CELL_JSON_2 = R"_({ "flaks":    30  })_";

    EXPECT_THROW(JsonToCell(_jsonDocumentFromString(INVALID_CELL_JSON_1)), JsonParseError);
    EXPECT_THROW(JsonToCell(_jsonDocumentFromString(INVALID_CELL_JSON_2)), JsonParseError);
}
#endif

TEST_F(GridGoblinConversionsTest, ChunkConversionToJsonAndBack) {
    FatCell cell;
    cell.cellKindId = {62};
    cell.wallSprite = {14, 17};
    cell.userData   = {1337};

    const auto bbs = BuildingBlock::CELL_KIND_ID | BuildingBlock::WALL_SPRITE | BuildingBlock::USER_DATA;
    const auto memLayout = detail::CalcChunkMemoryLayoutInfo(4, 4, bbs);

    Chunk chunk{memLayout};
    chunk.setAll(memLayout, cell);

    cell.cellKindId = {97};
    cell.wallSprite = {5, 0};
    cell.userData   = {420};
    chunk.setCellDataAtUnchecked(memLayout, 2, 0, &cell.cellKindId);
    chunk.setCellDataAtUnchecked(memLayout, 3, 1, &cell.wallSprite);
    chunk.setCellDataAtUnchecked(memLayout, 1, 2, &cell.userData);

    EXPECT_TRUE(ChunksContainIdenticalCellData(
        memLayout,
        chunk,
        JsonStringToChunk(ChunkToJsonString(chunk, bbs, memLayout), memLayout)));
}

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic

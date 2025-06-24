// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/World/World_config.hpp>

#include <gtest/gtest.h>

#include <filesystem>

namespace jbatnozic {
namespace gridgoblin {

namespace {
const WorldConfig DEFAULT_WORLD_CONFIG = {.chunkCountX                 = 8,
                                          .chunkCountY                 = 16,
                                          .cellsPerChunkX              = 32,
                                          .cellsPerChunkY              = 64,
                                          .cellResolution              = 12.34f,
                                          .wallHeight                  = 56.78f,
                                          .maxCellOpenness             = 5,
                                          .maxLoadedNonessentialChunks = 50,
                                          .chunkDirectoryPath          = "asdf/abc123"};
}

TEST(WorldConfigTest, ChunkGetters) {
    const std::filesystem::path path    = "temp_world_config_file";
    const auto                  cleanUp = [&]() {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    };

    try {
        WorldConfig::saveToFile(DEFAULT_WORLD_CONFIG, path);
        const auto loaded = WorldConfig::loadFromFile(path);

        EXPECT_EQ(DEFAULT_WORLD_CONFIG.chunkCountX, loaded.chunkCountX);
        EXPECT_EQ(DEFAULT_WORLD_CONFIG.chunkCountY, loaded.chunkCountY);
        EXPECT_EQ(DEFAULT_WORLD_CONFIG.cellsPerChunkX, loaded.cellsPerChunkX);
        EXPECT_EQ(DEFAULT_WORLD_CONFIG.cellsPerChunkY, loaded.cellsPerChunkY);
        EXPECT_NEAR(DEFAULT_WORLD_CONFIG.cellResolution, loaded.cellResolution, 0.001);
        EXPECT_NEAR(DEFAULT_WORLD_CONFIG.wallHeight, loaded.wallHeight, 0.001);
        EXPECT_EQ(DEFAULT_WORLD_CONFIG.maxCellOpenness, loaded.maxCellOpenness);
        EXPECT_EQ(DEFAULT_WORLD_CONFIG.maxLoadedNonessentialChunks, loaded.maxLoadedNonessentialChunks);
        EXPECT_EQ(DEFAULT_WORLD_CONFIG.chunkDirectoryPath, loaded.chunkDirectoryPath);
    } catch (...) {
        cleanUp();
        throw;
    }

    cleanUp();
}

} // namespace gridgoblin
} // namespace jbatnozic

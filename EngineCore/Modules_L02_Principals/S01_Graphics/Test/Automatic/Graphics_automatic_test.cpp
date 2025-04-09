// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Graphics.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <filesystem>

#include <Hobgoblin/Logging.hpp>

namespace gr = hg::gr;

namespace {
template <class taSprite>
void CheckOriginCorrectness(const taSprite& aSprite, const gr::OriginOffset& aExpectedOffset) {
    static constexpr float EPSILON = 0.001f;

    const auto origin = aSprite.getOrigin();
    const auto size   = aSprite.getLocalBounds();

    switch (aExpectedOffset.kind) {
    case gr::OriginOffset::RELATIVE_TO_TOP_LEFT:
    case gr::OriginOffset::RELATIVE_TO_CENTER:
        EXPECT_NEAR(origin.x, aExpectedOffset.offset.x, EPSILON);
        EXPECT_NEAR(origin.y, aExpectedOffset.offset.y, EPSILON);
        break;

    default:
        HG_UNREACHABLE("Invalid value for OriginOffset::Kind ({}).", (int)aExpectedOffset.kind);
    }
}
} // namespace

TEST(SpriteLoadingTest, LoadOriginOffset_SingleSubspriteExamples) {
    struct Case {
        const char16_t*  path;
        gr::OriginOffset expectedOffset;
    };

    const auto SPRITE_SIZE = hg::math::Vector2f{64.f, 32.f};

    // clang-format off
    const std::array<const Case, 4> CASES = {
        Case{HG_UNILIT("tile-0.png"), {{0.f, 0.f}, gr::OriginOffset::RELATIVE_TO_TOP_LEFT}},
        Case{HG_UNILIT("tile-1.png"), {{1.5f, 123.456f}, gr::OriginOffset::RELATIVE_TO_TOP_LEFT}},
        Case{HG_UNILIT("tile-2.png"), {{SPRITE_SIZE.x / 2.f, SPRITE_SIZE.y / 2.f}, gr::OriginOffset::RELATIVE_TO_CENTER}},
        Case{HG_UNILIT("tile-3.png"), {{SPRITE_SIZE.x / 2.f - 10.f, SPRITE_SIZE.y / 2.f + 15.f}, gr::OriginOffset::RELATIVE_TO_CENTER}}
    };
    // clang-format on

    constexpr gr::SpriteIdNumerical SPRITE_ID = 49;

    for (const auto& _case : CASES) {
        SCOPED_TRACE(hg::UniStrConv(hg::TO_ASCII_STD_STRING, _case.path));

        auto path = std::filesystem::path{HG_TEST_ASSET_DIR};
        path /= hg::UniStrConv(hg::TO_STD_PATH, _case.path);

        gr::SpriteLoader loader;
        loader.startTexture(128, 128)
            ->addSprite(SPRITE_ID, path)
            ->finalize(gr::TexturePackingHeuristic::BestAreaFit);

        // Make sprite
        {
            const auto blueprint = loader.getBlueprint(SPRITE_ID);
            EXPECT_TRUE(blueprint.hasExplicitOrigin());
            const auto sprite = blueprint.spr();
            CheckOriginCorrectness(sprite, _case.expectedOffset);
        }

        // Make multisprite
        {
            const auto blueprint = loader.getMultiBlueprint(SPRITE_ID);
            EXPECT_TRUE(blueprint.hasExplicitOrigin());

            const auto sprite = blueprint.multispr();
            CheckOriginCorrectness(sprite, _case.expectedOffset);
        }
    }
}

TEST(SpriteManifestTest, Test1) {
    enum SpriteIds {
        TILE_0 = 0,
        TILE_1 = 1
    };

    const auto originalSeveity = hg::log::GetMinimalLogSeverity();
    try {
        hg::log::SetMinimalLogSeverity(hg::log::Severity::Debug);

        const std::array<std::filesystem::path, 4> paths = {
            std::filesystem::path{HG_TEST_ASSET_DIR} / "SpriteManifest1.txt",
            std::filesystem::path{HG_TEST_ASSET_DIR} / "SpriteManifest2.txt",
            std::filesystem::path{HG_TEST_ASSET_DIR} / "SpriteManifest3.txt",
            std::filesystem::path{HG_TEST_ASSET_DIR} / "SpriteManifest4.txt"};

        hg::gr::SpriteLoader::SpriteManifestEnumerationMap enumMap;

        for (const auto& path : paths) {
            SCOPED_TRACE(path.string());

            hg::gr::SpriteLoader loader;
            ASSERT_NO_THROW(loader.loadSpriteManifest(path, &enumMap));

            EXPECT_NO_THROW(loader.getMultiBlueprint(TILE_0).multispr());
            EXPECT_NO_THROW(loader.getMultiBlueprint(TILE_1).multispr());
            EXPECT_NO_THROW(loader.getMultiBlueprint(2).multispr());
            EXPECT_NO_THROW(loader.getMultiBlueprint("tile-3").multispr());

            EXPECT_TRUE(enumMap.contains("TILE_0") && enumMap["TILE_0"] == TILE_0);
            EXPECT_TRUE(enumMap.contains("TILE_1") && enumMap["TILE_1"] == TILE_1);
        }
    } catch (...) {
        hg::log::SetMinimalLogSeverity(originalSeveity);
        throw;
    }
    hg::log::SetMinimalLogSeverity(originalSeveity);
}

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <filesystem>

#include <Hobgoblin/Logging.hpp>

namespace uwga = hg::uwga;

namespace {
using OriginOffset = uwga::SpriteAttributes::OriginOffset;

uwga::SpriteAttributes AttrsFromOriginOffset(hg::math::Vector2f aOffset, OriginOffset::Kind aKind) {
    uwga::SpriteAttributes attrs;
    attrs.originOffset = OriginOffset{aOffset, aKind};
    return attrs;
}

void CheckAttrsCorrectness(const uwga::Sprite& aSprite, const uwga::SpriteAttributes& aExpectedAttrs) {
    static constexpr float EPSILON = 0.001f;

    const auto origin = aSprite.getOrigin();
    const auto size   = aSprite.getLocalBounds();

    switch (aExpectedAttrs.originOffset->kind) {
    case uwga::SpriteAttributes::OriginOffset::RELATIVE_TO_TOP_LEFT:
    case uwga::SpriteAttributes::OriginOffset::RELATIVE_TO_CENTER:
        EXPECT_NEAR(origin.x, aExpectedAttrs.originOffset->value.x, EPSILON);
        EXPECT_NEAR(origin.y, aExpectedAttrs.originOffset->value.y, EPSILON);
        break;

    default:
        HG_UNREACHABLE("Invalid value for OriginOffset::Kind ({}).",
                       (int)aExpectedAttrs.originOffset->kind);
    }
}
} // namespace

TEST(SpriteLoadingTest, LoadSpriteAttrs_SingleSubspriteExamples) {
    struct Case {
        const char16_t*        path;
        uwga::SpriteAttributes expectedAttrs;
    };

    const auto SPRITE_SIZE = hg::math::Vector2f{64.f, 32.f};

    // clang-format off
    const std::array<const Case, 4> CASES = {
        Case{
            HG_UNILIT("tile-0.png"),
            AttrsFromOriginOffset({0.f, 0.f}, OriginOffset::RELATIVE_TO_TOP_LEFT)
        },
        Case{
            HG_UNILIT("tile-1.png"),
            AttrsFromOriginOffset({1.5f, 123.456f}, OriginOffset::RELATIVE_TO_TOP_LEFT)
        },
        Case{
            HG_UNILIT("tile-2.png"),
            AttrsFromOriginOffset({SPRITE_SIZE.x / 2.f, SPRITE_SIZE.y / 2.f}, OriginOffset::RELATIVE_TO_CENTER)
        },
        Case{
            HG_UNILIT("tile-3.png"),
            AttrsFromOriginOffset({SPRITE_SIZE.x / 2.f - 10.f, SPRITE_SIZE.y / 2.f + 15.f}, OriginOffset::RELATIVE_TO_CENTER)
        }
    };
    // clang-format on

    constexpr uwga::SpriteIdNumerical SPRITE_ID = 49;

    auto system = uwga::CreateGraphicsSystem("SFML");

    for (const auto& _case : CASES) {
        SCOPED_TRACE(hg::UniStrConv(hg::TO_ASCII_STD_STRING, _case.path));

        auto path = std::filesystem::path{HG_TEST_ASSET_DIR};
        path /= hg::UniStrConv(hg::TO_STD_PATH, _case.path);

        uwga::SpriteLoader loader{*system};
        loader.startTexture(128, 128)
            ->addSprite(SPRITE_ID, path)
            ->finalize(uwga::TexturePackingHeuristic::BEST_AREA_FIT);

        // Make sprite
        const auto blueprint = loader.getBlueprint(SPRITE_ID);
        EXPECT_FALSE(blueprint.getSpriteAttributes().isDefault());
        const auto sprite = blueprint.spr();
        CheckAttrsCorrectness(sprite, _case.expectedAttrs);
    }
}

TEST(SpriteLoadingTest, UseSpriteManifest) {
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

        hg::uwga::SpriteLoader::SpriteManifestEnumerationMap enumMap;

        auto system = uwga::CreateGraphicsSystem("SFML");

        for (const auto& path : paths) {
            SCOPED_TRACE(path.string());

            hg::uwga::SpriteLoader loader{*system};
            ASSERT_NO_THROW(loader.loadSpriteManifest(path, &enumMap));

            EXPECT_NO_THROW(loader.getBlueprint(TILE_0).spr());
            EXPECT_NO_THROW(loader.getBlueprint(TILE_1).spr());
            EXPECT_NO_THROW(loader.getBlueprint(2).spr());
            EXPECT_NO_THROW(loader.getBlueprint("tile-3").spr());

            EXPECT_TRUE(enumMap.contains("TILE_0") && enumMap["TILE_0"] == TILE_0);
            EXPECT_TRUE(enumMap.contains("TILE_1") && enumMap["TILE_1"] == TILE_1);
        }
    } catch (...) {
        hg::log::SetMinimalLogSeverity(originalSeveity);
        throw;
    }
    hg::log::SetMinimalLogSeverity(originalSeveity);
}

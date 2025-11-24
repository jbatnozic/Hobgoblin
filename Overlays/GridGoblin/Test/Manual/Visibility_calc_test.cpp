// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/GridGoblin.hpp>

#include <GridGoblin/Rendering/Visibility_calculator.hpp>

#include <Hobgoblin/Input.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Log_with_scoped_stopwatch.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>
#include <Hobgoblin/Utility/Time_utils.hpp>

#include <array>
#include <chrono>
#include <utility>

#include <Hobgoblin/Logging.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace {

#define LOG_ID "GridGoblin.ManualTest"

namespace hg   = jbatnozic::hobgoblin;
namespace uwga = hg::uwga;

#define CELL_COUNT_X     40
#define CELL_COUNT_Y     40
#define CELLRES          32.f
#define CELL_PROBABILITY 40

enum Sprites {
    SPR_FULL_SQUARE,
    SPR_LARGE_TRIANGLE,
    SPR_SMALL_TRIANGLE_HOR,
    SPR_TALL_SMALL_TRIANGLE_HOR,
    SPR_HALF_SQUARE_HOR,
    SPR_SMALL_TRIANGLE_VER,
    SPR_TALL_SMALL_TRIANGLE_VER,
    SPR_HALF_SQUARE_VER,
};

void SelectRandom(gridgoblin::SpriteId& aSpriteId, gridgoblin::Shape& aShape) {
    namespace gg = gridgoblin;

    // clang-format off
    static constexpr std::array<std::pair<gg::SpriteId, gg::Shape>, 8> CELLS = {{
        {SPR_FULL_SQUARE,             gg::Shape::FULL_SQUARE},
        {SPR_LARGE_TRIANGLE,          gg::Shape::LARGE_TRIANGLE},
        {SPR_SMALL_TRIANGLE_HOR,      gg::Shape::SMALL_TRIANGLE_HOR},
        {SPR_TALL_SMALL_TRIANGLE_HOR, gg::Shape::TALL_SMALL_TRIANGLE_HOR},
        {SPR_HALF_SQUARE_HOR,         gg::Shape::HALF_SQUARE_HOR},
        {SPR_SMALL_TRIANGLE_VER,      gg::Shape::SMALL_TRIANGLE_VER},
        {SPR_TALL_SMALL_TRIANGLE_VER, gg::Shape::TALL_SMALL_TRIANGLE_VER},
        {SPR_HALF_SQUARE_VER,         gg::Shape::HALF_SQUARE_VER},
    }};
    // clang-format on

    const auto index = hg::util::GetRandomNumber<std::size_t>(0, CELLS.size() - 1);
    aSpriteId        = CELLS[index].first;
    aShape           = CELLS[index].second;

    const auto rotation = hg::util::GetRandomNumber<int>(0, 3);
    switch (rotation) {
    case 0: // Flip none
        break;

    case 1: // Flip horizontal
        aSpriteId = aSpriteId | SPRITEID_HFLIP;
        aShape    = aShape | Shape::HFLIP;
        break;

    case 2: // Flip vertical
        aSpriteId = aSpriteId | SPRITEID_VFLIP;
        aShape    = aShape | Shape::VFLIP;
        break;

    case 3: // Flip both
        aSpriteId = aSpriteId | SPRITEID_HFLIP | SPRITEID_VFLIP;
        aShape    = aShape | Shape::HVFLIP;
        break;

    default:
        HG_UNREACHABLE();
    }
}

SpriteId TopDown(SpriteId aSpriteId) {
    return aSpriteId | SPRITEID_PROJECTION_TOPDOWN;
}

void RunVisibilityCalculatorTestImpl() {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);

    auto uwgaSystem = uwga::CreateGraphicsSystem("SFML");

    const auto root = std::filesystem::path{HG_TEST_ASSET_DIR};

    uwga::SpriteLoader spriteLoader{*uwgaSystem};
    // clang-format off
    spriteLoader.startTexture(256, 256)
        ->addSprite(TopDown(SPR_FULL_SQUARE),             root / "full-square.png")
        ->addSprite(TopDown(SPR_LARGE_TRIANGLE),          root / "large-triangle.png")
        ->addSprite(TopDown(SPR_SMALL_TRIANGLE_HOR),      root / "small-triangle-hor.png")
        ->addSprite(TopDown(SPR_TALL_SMALL_TRIANGLE_HOR), root / "tall-small-triangle-hor.png")
        ->addSprite(TopDown(SPR_HALF_SQUARE_HOR),         root / "half-square-hor.png")
        ->addSprite(TopDown(SPR_SMALL_TRIANGLE_VER),      root / "small-triangle-ver.png")
        ->addSprite(TopDown(SPR_TALL_SMALL_TRIANGLE_VER), root / "tall-small-triangle-ver.png")
        ->addSprite(TopDown(SPR_HALF_SQUARE_VER),         root / "half-square-ver.png")
        ->finalize(hg::uwga::TexturePackingHeuristic::BEST_AREA_FIT);
    // clang-format on

    ContentsConfig cc{.chunkCountX                 = 1,
                      .chunkCountY                 = 1,
                      .cellsPerChunkX              = CELL_COUNT_X,
                      .cellsPerChunkY              = CELL_COUNT_Y,
                      .buildingBlocks              = BuildingBlockMask::ALL,
                      .cellResolution              = CELLRES,
                      .maxCellOpenness             = 3,
                      .maxLoadedNonessentialChunks = 1};

    World world{cc};

    // Generate world:
    {
        hg::util::DoWith32bitRNG([](std::mt19937& aRng) {
            // const auto seed = hg::util::Generate32bitSeed();
            // const auto seed = 2593577924ULL;
            const auto seed = 0xDEADBEEF;
            aRng.seed(seed);
            HG_LOG_INFO(LOG_ID, "32bit seed = {}", seed);
        });
        hg::util::DoWith64bitRNG([](std::mt19937_64& aRng) {
            // const auto seed = hg::util::Generate64bitSeed();
            // const auto seed = 11823085330007581526ULL;
            const auto seed = 0xDEADBEEFDEADBEEF;
            aRng.seed(seed);
            HG_LOG_INFO(LOG_ID, "64bit seed = {}", seed);
        });

        auto perm = world.getPermissionToEdit();
        world.edit(*perm, [&world](World::Editor& aEditor) {
            for (int y = 0; y < world.getCellCountY(); y += 1) {
                for (int x = 0; x < world.getCellCountX(); x += 1) {
                    cell::FloorSprite floorSprite{.id = SPRITEID_NONE};
                    cell::WallSprite  wallSprite{.id = SPRITEID_NONE, .id_reduced = SPRITEID_NONE};
                    cell::SpatialInfo spatialInfo{.wallShape = Shape::EMPTY};

                    if (x == CELL_COUNT_X / 2 - 1 || x == CELL_COUNT_X / 2) {
                        aEditor.setCellDataAt(x, y, &floorSprite, &wallSprite, &spatialInfo);
                        continue;
                    }
                    if (hg::util::GetRandomNumber(0, 99) >= CELL_PROBABILITY) {
                        aEditor.setCellDataAt(x, y, &floorSprite, &wallSprite, &spatialInfo);
                        continue;
                    }

                    SelectRandom(wallSprite.id, spatialInfo.wallShape);
                    aEditor.setCellDataAt(x, y, &floorSprite, &wallSprite, &spatialInfo);
                }
            }
        });
    }

    TopDownRenderer            renderer{world, spriteLoader};
    VisibilityCalculatorConfig visCalcConfig;
    visCalcConfig.minRingsBeforeRaycasting = 0;
    VisibilityCalculator visCalc{world, visCalcConfig};
    auto                 vcImage   = uwgaSystem->createImage();
    auto                 vcTexture = uwgaSystem->createTexture();

    const auto generateLoS = [&](hg::math::Vector2d pos) {
        HG_LOG_INFO(LOG_ID, "===============================================");
        HG_LOG_INFO(LOG_ID, "Running calc()...");
        {
            HG_LOG_WITH_SCOPED_STOPWATCH_MS(INFO, LOG_ID, "calc() took {}ms", elapsed_time_ms);
            visCalc.calc({CELL_COUNT_X * CELLRES * 0.5f, CELL_COUNT_Y * CELLRES * 0.5f},
                         {CELL_COUNT_X * CELLRES, CELL_COUNT_Y * CELLRES},
                         PositionInWorld{pos});
        }

        HG_LOG_INFO(LOG_ID,
                    "Triangles: {}, Comparisons: {}, Rings: {}",
                    visCalc.getStats().triangleCount,
                    visCalc.getStats().triangleCheckCount,
                    visCalc.getStats().highDetailRingCount);

        HG_LOG_INFO(LOG_ID, "Generating image...");
        {
            HG_LOG_WITH_SCOPED_STOPWATCH_MS(INFO, LOG_ID, "Image generation took {}ms", elapsed_time_ms);
            vcImage->reset(hg::ToPz(CELL_COUNT_X * CELLRES), hg::ToPz(CELL_COUNT_Y * CELLRES));
            for (int y = 0; y < hg::ToPz(CELL_COUNT_X * CELLRES); y += 1) {
                for (int x = 0; x < hg::ToPz(CELL_COUNT_Y * CELLRES); x += 1) {
                    const auto v = visCalc.testVisibilityAt({(double)x, (double)y});
                    if (!v.has_value() || *v == false) {
                        vcImage->setPixel(x, y, hg::uwga::COLOR_BLACK.withAlpha(150));
                    } else {
                        vcImage->setPixel(x, y, hg::uwga::COLOR_TRANSPARENT);
                    }
                }
            }
        }

        HG_LOG_INFO(LOG_ID, "Loading texture...");
        vcTexture->reset(*vcImage);
        HG_LOG_INFO(LOG_ID, "Done!");
    };

    generateLoS({CELL_COUNT_X * CELLRES * 0.5f, CELL_COUNT_Y * CELLRES * 0.5f});

    hg::uwga::Sprite vcSprite{*vcTexture};

    auto window = uwgaSystem->createRenderWindow(1280, 1280, uwga::WindowStyle::DEFAULT, "GridWorld");
    window->setFramerateLimit(120);

    auto view = uwgaSystem->createView();
    view->setSize({CELL_COUNT_X * CELLRES, CELL_COUNT_Y * CELLRES});
    view->setCenter({CELL_COUNT_X * CELLRES * 0.5f, CELL_COUNT_Y * CELLRES * 0.5f});
    window->setView(*view);

    hg::util::Stopwatch swatch;

    while (true) {
        const auto frameTime = swatch.restart<std::chrono::microseconds>();

        hg::uwga::WindowEvent ev;
        while (window && window->pollEvent(ev)) {
            ev.visit(
                [&](hg::uwga::WindowEvent::Closed&) {
                    window.reset();
                },
                [&](hg::uwga::WindowEvent::MouseButtonPressed& aButton) {
                    if (aButton.button == hg::in::MB_LEFT) {
                        const auto coords = window->mapPixelToCoords({aButton.x, aButton.y});
                        HG_LOG_INFO(LOG_ID, "Coords = {}, {}", coords.x, coords.y);
                        generateLoS(coords.cast<double>());
                    }
                });
        } // end event processing
        if (!window) {
            break;
        }

        window->clear(hg::uwga::Color{155, 155, 200});

        const Renderer::RenderParameters renderParams{
            .viewCenter  = PositionInWorld{view->getAnchor() + view->getCenter().cast<double>()},
            .viewSize    = view->getSize(),
            .pointOfView = {},
            .flags       = 0};

        renderer.startPrepareToRender(renderParams);
        renderer.endPrepareToRender();
        renderer.render(*window);

        window->draw(vcSprite);
        window->flush();
        window->display();
    }
}

} // namespace
} // namespace gridgoblin
} // namespace jbatnozic

void RunVisibilityCalculatorTest(const std::vector<const char*>& /*aArgs*/) {
    jbatnozic::gridgoblin::RunVisibilityCalculatorTestImpl();
}

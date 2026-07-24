// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/GridGoblin.hpp>

#include <Hobgoblin/Input.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>
#include <Hobgoblin/Utility/Time_utils.hpp>

#include <GL/glew.h>

#include <chrono>
#include <iostream>

#include <Hobgoblin/Logging.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace {

namespace hg   = jbatnozic::hobgoblin;
namespace uwga = hg::uwga;

void DrawIsometricSquareAt(hg::uwga::Canvas&  aCanvas,
                           float              aSize,
                           hg::uwga::Color    aColor,
                           hg::math::Vector2f aPosition) {
    const float coords[5][2] = {
        {  0.f,   0.f},
        {  0.f, aSize},
        {aSize, aSize},
        {aSize,   0.f},
        {  0.f,   0.f}
    };

    using hg::uwga::Vertex;
    using hg::uwga::VertexArray;

    uwga::VertexArray va;
    va.primitiveType = hg::uwga::PrimitiveType::LINE_STRIP;

    for (int i = 0; i < 5; i += 1) {
        auto iso = dimetric::ToPositionInView(
            PositionInWorld{coords[i][0] + aPosition.x, coords[i][1] + aPosition.y});
        va.vertices.push_back(uwga::Vertex{(*iso).cast<float>(), aColor});
    }

    aCanvas.draw(va);
}

#define SPR_STONE_TILE 0
#define SPR_WALL       1
#define SPR_WALL_SHORT 2
#define SPR_LIGHT      9

void RunDimetricRenderingTestImpl() {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);

    auto uwgaSystem = uwga::CreateGraphicsSystem("SFML");

    hg::uwga::SpriteLoader loader{*uwgaSystem};
    loader.startTexture(1024, 1024)
        ->addSprite(SPR_STONE_TILE, (HG_TEST_ASSET_DIR "/isometric-stone-tile.png"))
        ->addSprite(SPR_WALL, (HG_TEST_ASSET_DIR "/isometric-wall.png"))
        ->addSprite(SPR_WALL_SHORT, (HG_TEST_ASSET_DIR "/isometric-wall-short.png"))
        ->addSprite(SPR_LIGHT, (HG_TEST_ASSET_DIR "/light.png"))
        ->finalize(hg::uwga::TexturePackingHeuristic::BEST_AREA_FIT);

    ContentsConfig cc{.chunkCountX                 = 8,
                      .chunkCountY                 = 8,
                      .cellsPerChunkX              = 8,
                      .cellsPerChunkY              = 8,
                      .buildingBlocks              = BuildingBlockMask::ALL,
                      .cellResolution              = 32.0,
                      .wallHeight                  = 32.0 * 6.0,
                      .maxCellOpenness             = 3,
                      .maxLoadedNonessentialChunks = 64};

    World world{cc};

    // Generate world:
    {
        auto perm = world.getPermissionToEdit();
        world.toggleGeneratorMode(*perm, true);
        world.edit(*perm, [&world](World::Editor& aEditor) {
            for (int y = 0; y < world.getCellCountY(); y += 1) {
                for (int x = 0; x < world.getCellCountX(); x += 1) {
                    cell::FloorSprite floorSprite{.id = SPR_STONE_TILE};
                    cell::WallSprite  wallSprite{.id = SPR_WALL, .id_reduced = SPR_WALL_SHORT};
                    cell::SpatialInfo spatialInfo{.wallShape = Shape::FULL_SQUARE};

                    if (hg::util::GetRandomNumber(0, 99) > 15) {
                        wallSprite = {.id = SPRITEID_NONE, .id_reduced = SPRITEID_NONE};
                        spatialInfo.wallShape = Shape::EMPTY;
                    }

                    aEditor.setCellDataAtUnchecked(x, y, &floorSprite, &wallSprite, &spatialInfo);
                }
            }
        });
        world.toggleGeneratorMode(*perm, false);
    }

    auto window = uwgaSystem->createRenderWindow(1280, 1280, uwga::WindowStyle::DEFAULT, "GridWorld");
    window->setFramerateLimit(120);

    auto view = uwgaSystem->createView();
    view->setSize(1280, 1280);
    window->setView(*view);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    DimetricRenderer     renderer{loader};
    VisibilityCalculator visCalc{world};

    // clang-format off
    RenderContext renderCtx = {
        .world = &world,
        .impls = {
            .renderer           = &renderer,
            .visibilityProvider = &visCalc,
        },
    };
    // clang-format on

    hg::util::Stopwatch swatch;

    while (true) {
        const auto frameTime = swatch.restart<std::chrono::microseconds>();

        bool mouseLClick = false;
        bool mouseRClick = false;

        hg::uwga::WindowEvent ev;
        while (window && window->pollEvent(ev)) {
            ev.visit(
                [&](hg::uwga::WindowEvent::Closed&) {
                    window.reset();
                },
                [&](hg::uwga::WindowEvent::MouseButtonPressed& aButton) {
                    if (aButton.button == hg::in::MB_LEFT) {
                        mouseLClick = true;
                    }
                    if (aButton.button == hg::in::MB_RIGHT) {
                        mouseRClick = true;
                    }
                });
        } // end event processing
        if (!window) {
            break;
        }

        {
            using namespace hg::in;
            const auto lr = (float)CheckPressedPK(PK_D) - (float)CheckPressedPK(PK_A);
            const auto ud = (float)CheckPressedPK(PK_S) - (float)CheckPressedPK(PK_W);
            view->setAnchor(view->getAnchor() + hg::math::Vector2d{lr * 6.0, ud * 6.0});
            window->setView(*view);
        }

        window->clear(hg::uwga::Color{0, 0, 55});

        const auto cursorWindowPos =
            window->mapPixelToCoords(window->getRelativeCursorPosition(), *view);

        const auto cursorWorldPos = dimetric::ToPositionInWorld(PositionInView{cursorWindowPos});

        // Edit the world
        {
            const auto xx = static_cast<int>(cursorWorldPos->x / world.getCellResolution());
            const auto yy = static_cast<int>(cursorWorldPos->y / world.getCellResolution());

            if (xx >= 0 && xx < world.getCellCountX() && yy >= 0 && yy < world.getCellCountY()) {
                auto perm = world.getPermissionToEdit();
                // clang-format off
                if (mouseLClick) {
                    world.edit(*perm, [&world, xx, yy](World::Editor& aEditor) {
                        const auto ws = cell::WallSprite{.id = SPR_WALL, .id_reduced = SPR_WALL_SHORT};
                        const auto si =  cell::SpatialInfo{.wallShape = Shape::FULL_SQUARE};
                        aEditor.setCellDataAtUnchecked(xx, yy, &ws, &si);
                    });
                } else if (mouseRClick) {
                    world.edit(*perm, [&world, xx, yy](World::Editor& aEditor) {
                        const auto ws = cell::WallSprite{.id = SPRITEID_NONE, .id_reduced = SPRITEID_NONE};
                        const auto si =  cell::SpatialInfo{.wallShape = Shape::EMPTY};
                        aEditor.setCellDataAtUnchecked(xx, yy, &ws, &si);
                    });
                }
                // clang-format on
            }
        }

        const auto t1 = std::chrono::steady_clock::now();

        const double overdrawLeft  = 32.0;
        const double overdrawRight = 32.0;
        const double overdrawUp    = 32.0;
        const double overdrawDown  = 256.0; // TODO: unused!

        // clang-format off
        renderCtx.dynamic = {
            .viewCenter  = PositionInView{view->getAnchor() + view->getCenter().cast<double>()},
            .viewSize    = view->getSize(),
            .pointOfView = cursorWorldPos,
            .flags       = RenderFlags::REDUCE_WALLS_BASED_ON_POSITION
        };
        // clang-format on

        if (hg::in::CheckPressedVK(hg::in::VK_SPACE)) {
            visCalc.calculate(dimetric::ToPositionInWorld(renderCtx.dynamic.viewCenter),
                              renderCtx.dynamic.viewSize,
                              renderCtx.dynamic.pointOfView);
            renderCtx.dynamic.flags =
                /*renderCtx.dynamic.flags |*/ RenderFlags::REDUCE_WALLS_BASED_ON_VISIBILITY;
        }

        Reset(renderCtx);
        PrepareToRender(renderCtx);
        Render(renderCtx, *window);

        const auto t2 = std::chrono::steady_clock::now();
        // std::cout << "Time to render: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -
        // t1).count() / 1000.0 << "ms "
        //           << "frame time: " << frameTime.count() / 1000.0 << "ms.\n";

        if constexpr (true) {
            const float xx = floorf((float)cursorWorldPos->x / 32.f) * 32.f;
            const float yy = floorf((float)cursorWorldPos->y / 32.f) * 32.f;
            DrawIsometricSquareAt(*window, 32.f, hg::uwga::COLOR_RED, {xx, yy});
        }

        if constexpr (false) {
            visCalc.__ggimpl_experimental_render(*window);
        }

        window->display();
    }
}

} // namespace
} // namespace gridgoblin
} // namespace jbatnozic

void RunDimetricRenderingTest(const std::vector<const char*>& /*aArgs*/) {
    jbatnozic::gridgoblin::RunDimetricRenderingTestImpl();
}

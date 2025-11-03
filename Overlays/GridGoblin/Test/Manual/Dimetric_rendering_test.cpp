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

namespace hg = jbatnozic::hobgoblin;

#define vector_cast hg::math::VectorCast

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

    // VertexArray va;
    // va.primitiveType = hg::uwga::PrimitiveType::LINE_STRIP;

    // for (int i = 0; i < 5; i += 1) {
    //     auto iso = gridworld::IsometricCoordinatesToScreen(
    //         {coords[i][0] + aPosition.x, coords[i][1] + aPosition.y});
    //     va.vertices.push_back(Vertex{iso, aColor});
    // }

    // aCanvas.draw(va);
}

#define SPR_STONE_TILE 0
#define SPR_WALL       1
#define SPR_WALL_SHORT 2
#define SPR_LIGHT      9

void RunDimetricRenderingTestImpl() {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);

    hg::uwga::SpriteLoader loader;
    loader.startTexture(1024, 1024)
        ->addSprite(SPR_STONE_TILE, (HG_TEST_ASSET_DIR "/isometric-stone-tile.png"))
        ->addSprite(SPR_WALL, (HG_TEST_ASSET_DIR "/isometric-wall.png"))
        ->addSprite(SPR_WALL_SHORT, (HG_TEST_ASSET_DIR "/isometric-wall-short.png"))
        ->addSprite(SPR_LIGHT, (HG_TEST_ASSET_DIR "/light.png"))
        ->finalize(hg::uwga::TexturePackingHeuristic::BestAreaFit);

    WorldConfig config{.chunkCountX                 = 8,
                       .chunkCountY                 = 8,
                       .cellsPerChunkX              = 8,
                       .cellsPerChunkY              = 8,
                       .cellResolution              = 32.f,
                       .maxCellOpenness             = 3,
                       .maxLoadedNonessentialChunks = 64,
                       .chunkDirectoryPath          = "GGManualTest_WorkDir"};

    World world{config};

    // Generate world:
    {
        auto perm = world.getPermissionToEdit();
        world.toggleGeneratorMode(*perm, true);
        world.edit(*perm, [&world](World::Editor& aEditor) {
            for (int y = 0; y < world.getCellCountY(); y += 1) {
                for (int x = 0; x < world.getCellCountX(); x += 1) {
                    aEditor.setFloorAt({x, y}, CellModel::Floor{SPR_STONE_TILE});
                    if (hg::util::GetRandomNumber(0, 99) < 15) {
                        aEditor.setWallAt({x, y},
                                          CellModel::Wall{SPR_WALL, SPR_WALL_SHORT, Shape::FULL_SQUARE});
                    }
                }
            }
        });
        world.toggleGeneratorMode(*perm, false);
    }

    hg::uwga::RenderWindow window{
        hg::win::VideoMode{1280, 950},
        "GridWorld"
    };
    window.setFramerateLimit(120);
    // window.setVerticalSyncEnabled(true);
    window.getView().setSize({1280, 950});

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    VisibilityCalculator visCalc{world};
    DimetricRenderer     renderer{world, loader};
    double               renderingXOffset = 0.0;
    double               renderingYOffset = 0.0;

    hg::util::Stopwatch swatch;

    while (window.isOpen()) {
        const auto frameTime = swatch.restart<std::chrono::microseconds>();

        bool           mouseLClick = false;
        bool           mouseRClick = false;
        hg::win::Event ev;
        while (window.pollEvent(ev)) {
            ev.visit(
                [&](hg::win::Event::Closed&) {
                    window.close();
                },
                [&](hg::win::Event::MouseButtonPressed& aButton) {
                    if (aButton.button == hg::in::MB_LEFT) {
                        mouseLClick = true;
                    }
                    if (aButton.button == hg::in::MB_RIGHT) {
                        mouseRClick = true;
                    }
                });
        } // end event processing

        {
            using namespace hg::in;
            const auto lr = (float)CheckPressedPK(PK_D) - (float)CheckPressedPK(PK_A);
            const auto ud = (float)CheckPressedPK(PK_S) - (float)CheckPressedPK(PK_W);
            window.getView().move({lr * 4.f, ud * 4.f});
        }
        {
            using namespace hg::in;
            const auto lr = (double)CheckPressedPK(PK_L) - (double)CheckPressedPK(PK_J);
            const auto ud = (double)CheckPressedPK(PK_K) - (double)CheckPressedPK(PK_I);
            renderingXOffset += lr * 4.0;
            renderingYOffset += ud * 4.0;
        }

        window.clear(hg::uwga::Color{0, 0, 55});

        const auto mouseWindowPos = hg::win::GetMousePositionRelativeToWindow(window);
        const auto cursorInWorld  = dimetric::ToPositionInWorld(
            PositionInView{vector_cast<double>(window.mapPixelToCoords(mouseWindowPos))});

        // Edit the world
        {
            const auto xx = static_cast<int>(cursorInWorld->x / world.getCellResolution());
            const auto yy = static_cast<int>(cursorInWorld->y / world.getCellResolution());

            if (xx >= 0 && xx < world.getCellCountX() && yy >= 0 && yy < world.getCellCountY()) {
                auto perm = world.getPermissionToEdit();
                // clang-format off
                if (mouseLClick) {
                    world.edit(*perm, [&world, xx, yy](World::Editor& aEditor) {
                        aEditor.setWallAtUnchecked(xx, yy, {{SPR_WALL, SPR_WALL_SHORT, Shape::FULL_SQUARE}});
                    });
                } else if (mouseRClick) {
                    world.edit(*perm, [&world, xx, yy](World::Editor& aEditor) {
                        aEditor.setWallAtUnchecked(xx, yy, std::nullopt);
                    });
                }
                // clang-format on
            }
        }

        const auto t1 = std::chrono::steady_clock::now();

        const double overdrawLeft  = 32.0;
        const double overdrawRight = 32.0;
        const double overdrawUp    = 32.0;
        const double overdrawDown  = 256.0;

        const auto viewCenter =
            vector_cast<double>(window.getView(0).getCenter()) +
            hg::math::Vector2d{(overdrawRight - overdrawLeft) * 0.5, (overdrawDown - overdrawUp) * 0.5};

        const auto viewSize =
            vector_cast<double>(window.getView(0).getSize()) +
            hg::math::Vector2d{overdrawLeft + overdrawRight, overdrawUp + overdrawDown};

        const Renderer::RenderParameters renderParams{.viewCenter  = PositionInWorld{viewCenter},
                                                      .viewSize    = viewSize,
                                                      .pointOfView = cursorInWorld,
                                                      .xOffset     = renderingXOffset,
                                                      .yOffset     = renderingYOffset};

        if (!hg::in::CheckPressedVK(hg::in::VK_SPACE)) {
            renderer.startPrepareToRender(renderParams,
                                          DimetricRenderer::REDUCE_WALLS_BASED_ON_POSITION,
                                          nullptr);
        } else {
            visCalc.calc(dimetric::ToPositionInWorld(
                             PositionInView{vector_cast<double>(window.getView(0).getCenter())}),
                         vector_cast<double>(window.getView(0).getSize()),
                         cursorInWorld);
            renderer.startPrepareToRender(renderParams,
                                          DimetricRenderer::REDUCE_WALLS_BASED_ON_POSITION |
                                              DimetricRenderer::REDUCE_WALLS_BASED_ON_VISIBILITY,
                                          &visCalc);
        }
        renderer.endPrepareToRender();
        renderer.render(window);
        const auto t2 = std::chrono::steady_clock::now();
        // std::cout << "Time to render: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -
        // t1).count() / 1000.0 << "ms "
        //           << "frame time: " << frameTime.count() / 1000.0 << "ms.\n";

        // if (true) {
        //     const float xx = floorf(isoCoords.x / 32.f) * 32.f;
        //     const float yy = floorf(isoCoords.y / 32.f) * 32.f;
        //     DrawIsometricSquareAt(window, 32.f, hg::uwga::COLOR_RED, {xx, yy});
        // }

        window.display();
    }
}

} // namespace
} // namespace gridgoblin
} // namespace jbatnozic

void RunDimetricRenderingTest(const std::vector<const char*>& /*aArgs*/) {
    jbatnozic::gridgoblin::RunDimetricRenderingTestImpl();
}

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Input.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Time_utils.hpp>

#include "Fake_disk_io_handler.hpp"
#include <GridGoblin/Private/Chunk_spooler_default.hpp>
#include <GridGoblin/World/World.hpp>

#include <algorithm>
#include <vector>

#include <Hobgoblin/Logging.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace hg = jbatnozic::hobgoblin;

namespace {
constexpr auto LOG_ID = "GridGoblin.ManualTest";

class Fixture : public hg::uwga::Drawable {
public:
    Fixture()
        : _fakeDiskIoHandler{}
        , _world{_makeWorldConfig(), &_fakeDiskIoHandler}
        , _activeArea{_world.createActiveArea()} {}

    void update() {
        _world.update();
        _world.prune();
    }

    void onMouseButtonPressed(hg::in::MouseButton aButton, hg::math::Vector2d aPos) {
        if (aPos.x < 0.0 || aPos.x >= _world.getCellCountX() * _world.getCellResolution() ||
            aPos.y < 0.0 || aPos.y >= _world.getCellCountY() * _world.getCellResolution()) {
            return;
        }

        auto perm = _world.getPermissionToEdit();

        switch (aButton) {
        case hg::in::MB_LEFT:
            {
                const auto        cellXY = _world.posToCell(aPos.x, aPos.y);
                cell::SpatialInfo spatialInfo;
                const auto        cellIsLoaded = _world.getCellDataAtUnchecked(cellXY, &spatialInfo);
                if (cellIsLoaded) {
                    if (spatialInfo.hasNonEmptyWallShape()) {
                        spatialInfo.wallShape = Shape::EMPTY;
                    } else {
                        spatialInfo.wallShape = Shape::FULL_SQUARE;
                    }
                    _world.edit(*perm, [cellXY, spatialInfo](World::Editor& aEditor) {
                        aEditor.setCellDataAtUnchecked(cellXY, &spatialInfo);
                    });
                }
            }
            break;

        case hg::in::MB_RIGHT:
            {
                const auto cell    = _world.posToCell(aPos.x, aPos.y);
                const auto chunkId = _world.cellToChunkIdUnchecked(cell);

                const auto iter = std::find(_activeChunks.begin(), _activeChunks.end(), chunkId);
                if (iter == _activeChunks.end()) {
                    _activeChunks.push_back(chunkId);
                } else {
                    _activeChunks.erase(iter);
                }

                _activeArea.setToChunkList(_activeChunks);
            }
            break;

        default:
            break;
        }
    }

    void drawOnto(
        hg::uwga::Canvas&             aCanvas,
        const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) const override //
    {
        for (hg::PZInteger y = 0; y < _world.getChunkCountY(); y += 1) {
            for (hg::PZInteger x = 0; x < _world.getChunkCountX(); x += 1) {
                _drawChunk(aCanvas, {x, y});
            }
        }
    }

private:
    test::FakeDiskIoHandler _fakeDiskIoHandler;
    World                   _world;
    ActiveArea              _activeArea;

    std::vector<ChunkId> _activeChunks;

    static WorldConfig _makeWorldConfig() {
        return {.chunkCountX                 = 4,
                .chunkCountY                 = 4,
                .cellsPerChunkX              = 8,
                .cellsPerChunkY              = 8,
                .cellResolution              = 32.f,
                .maxCellOpenness             = 5,
                .maxLoadedNonessentialChunks = 0,
                .chunkDirectoryPath          = "GGManualTest_WorkDir"};
    }

    void _drawChunk(hg::uwga::Canvas& aCanvas, ChunkId aChunkId) const {
        auto* chunk = _world.getChunkAtId(aChunkId);
        if (!chunk) {
            return;
        }

        auto& uwgaSystem = aCanvas.getSystem();

        const auto         cellRes = (float)_world.getCellResolution();
        hg::math::Vector2f start{aChunkId.x * _world.getCellsPerChunkX() * cellRes,
                                 aChunkId.y * _world.getCellsPerChunkY() * cellRes};

        hg::uwga::RectangleShape rect{
            uwgaSystem,
            {cellRes, cellRes}
        };
        rect.setOutlineColor(hg::uwga::COLOR_BLACK);
        rect.setOutlineThickness(1.f);

        auto text =
            uwgaSystem.createText(uwgaSystem.getBuiltinFont(hg::uwga::BuiltInFont::TITILLIUM_REGULAR));
        text->setFillColor(hg::uwga::COLOR_YELLOW);

        const float textOffset = 4.f;

        for (hg::PZInteger y = 0; y < _world.getCellsPerChunkY(); y += 1) {
            for (hg::PZInteger x = 0; x < _world.getCellsPerChunkX(); x += 1) {
                rect.setPosition(start.x + x * cellRes, start.y + y * cellRes);
                cell::SpatialInfo spatialInfo;
                chunk->getCellDataAtUnchecked(_world.getChunkMemoryLayoutInfo(), x, y, &spatialInfo);
                if (spatialInfo.hasNonEmptyWallShape()) {
                    rect.setFillColor(hg::uwga::COLOR_BLACK);
                } else {
                    rect.setFillColor(hg::uwga::COLOR_TRANSPARENT);
                }
                aCanvas.draw(rect);

                text->setString(std::to_string((int)spatialInfo.openness));
                text->setPosition(start.x + x * cellRes + textOffset,
                                  start.y + y * cellRes + textOffset);
                text->setScale({0.75f, 0.75f});
                aCanvas.draw(*text);
            }
        }
    }
};

void OpennessTestImpl() {
    Fixture fixture;

    auto system = hg::uwga::CreateGraphicsSystem("SFML");
    auto window = system->createRenderWindow(1024, 1024, hg::uwga::WindowStyle::DEFAULT, "OpennessTest");
    window->setFramerateLimit(60);

    auto view = system->createView();
    view->setCenter({512, 512});
    view->setSize({1024, 1024});
    view->setViewport({0.f, 0.f, 1.f, 1.f});
    window->setView(*view);

    while (true) {
        hg::uwga::WindowEvent ev;
        while (window && window->pollEvent(ev)) {
            ev.visit(
                [&](const hg::uwga::WindowEvent::Closed&) {
                    window.reset();
                },
                [&](const hg::uwga::WindowEvent::MouseButtonPressed& aData) {
                    fixture.onMouseButtonPressed(aData.button,
                                                 window->mapPixelToCoords({aData.x, aData.y}));
                });
        }
        if (!window) {
            break;
        }

        window->clear(hg::uwga::COLOR_DARK_GRAY);
        fixture.update();
        window->draw(fixture);
        window->display();
    }
}
} // namespace
} // namespace gridgoblin
} // namespace jbatnozic

void RunOpennessTest(int, const char**) {
    jbatnozic::gridgoblin::OpennessTestImpl();
}

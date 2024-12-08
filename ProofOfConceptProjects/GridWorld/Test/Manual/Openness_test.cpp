

#include <Hobgoblin/Graphics.hpp>
#include <Hobgoblin/Input.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/Utility/Time_utils.hpp>
#include <Hobgoblin/Window.hpp>

#include "../../Source/Detail_access.hpp"
#include "../../Source/Model/Chunk_spooler_default.hpp"
#include "Fake_disk_io_handler.hpp"
#include <GridWorld/World/World.hpp>

#include <algorithm>
#include <vector>

#include <Hobgoblin/Logging.hpp>

namespace gridworld {
namespace hg = jbatnozic::hobgoblin;

namespace {
constexpr auto LOG_ID = "Griddy.ManualTest";

class Fixture {
public:
    Fixture()
        : _fakeDiskIoHandler{}
        , _world{_makeWorldConfig(), &_fakeDiskIoHandler}
        , _activeArea{_world.createActiveArea()} {}

    void update() {
        _world.update();
        _world.prune();
    }

    void onMouseButtonPressed(hg::in::MouseButton aButton, hg::math::Vector2f aPos) {
        if (aPos.x < 0.f || aPos.x >= _world.getCellCountX() * _world.getCellResolution() ||
            aPos.y < 0.f || aPos.y >= _world.getCellCountY() * _world.getCellResolution()) {
            return;
        }

        auto perm = _world.getPermissionToEdit();

        switch (aButton) {
        case hg::in::MB_LEFT:
            {
                const auto  cellXY = _world.posToCell(aPos);
                const auto* cell   = _world.getCellAtUnchecked(cellXY);
                if (cell) {
                    if (!cell->isWallInitialized()) {
                        _world.edit(*perm, [cellXY](World::Editor& aEditor) {
                            aEditor.setWallAtUnchecked(cellXY, CellModel::Wall{});
                        });
                    } else {
                        _world.edit(*perm, [cellXY](World::Editor& aEditor) {
                            aEditor.setWallAt(cellXY, std::nullopt);
                        });
                    }
                }
            }
            break;

        case hg::in::MB_RIGHT:
            {
                const auto cell    = _world.posToCell(aPos);
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

    void draw(hg::gr::Canvas& aCanvas) {
        for (hg::PZInteger y = 0; y < _world.getChunkCountY(); y += 1) {
            for (hg::PZInteger x = 0; x < _world.getChunkCountX(); x += 1) {
                auto* chunk = _world.getChunkAtId({x, y});
                if (chunk) {
                    _drawChunk(aCanvas, *chunk, {x, y});
                } else {
                    // DO NOTHING
                }
            }
        }
    }

private:
    test::FakeDiskIoHandler _fakeDiskIoHandler;
    World                   _world;
    ActiveArea              _activeArea;

    std::vector<ChunkId> _activeChunks;

    static WorldConfig _makeWorldConfig() {
        return {.chunkCountX                = 4,
                .chunkCountY                = 4,
                .cellsPerChunkX             = 8,
                .cellsPerChunkY             = 8,
                .cellResolution             = 32.f,
                .maxCellOpenness            = 4,
                .maxLoadedNonessentialCells = 0};
    }

    void _drawChunk(hg::gr::Canvas& aCanvas, const Chunk& aChunk, ChunkId aChunkId) const {
        const auto         cellRes = _world.getCellResolution();
        hg::math::Vector2f start{aChunkId.x * aChunk.getCellCountX() * cellRes,
                                 aChunkId.y * aChunk.getCellCountY() * cellRes};

        hg::gr::RectangleShape rect{
            {cellRes, cellRes}
        };
        rect.setOutlineColor(hg::gr::COLOR_BLACK);
        rect.setOutlineThickness(1.f);

        hg::gr::Text text{hg::gr::BuiltInFonts::getFont(hg::gr::BuiltInFonts::TITILLIUM_REGULAR)};
        text.setFillColor(hg::gr::COLOR_YELLOW);

        const float textOffset = 4.f;

        for (hg::PZInteger y = 0; y < aChunk.getCellCountY(); y += 1) {
            for (hg::PZInteger x = 0; x < aChunk.getCellCountX(); x += 1) {
                rect.setPosition(start.x + x * cellRes, start.y + y * cellRes);
                const auto& cell = aChunk.getCellAtUnchecked(x, y);
                if (cell.isWallInitialized()) {
                    rect.setFillColor(hg::gr::COLOR_BLACK);
                } else {
                    rect.setFillColor(hg::gr::COLOR_TRANSPARENT);
                }
                aCanvas.draw(rect);

                text.setString(std::to_string((int)GetExtensionData(cell).openness));
                text.setPosition(start.x + x * cellRes + textOffset, start.y + y * cellRes + textOffset);
                text.setScale({0.75f, 0.75f});
                aCanvas.draw(text);
            }
        }
    }
};

void OpennessTestImpl() {
    Fixture fixture;

    hg::gr::RenderWindow window;
    window.create(hg::win::VideoMode{1024, 1024}, "OpennessTest");
    window.setViewCount(1);
    window.getView(0).setCenter({512, 512});
    window.getView(0).setSize({1024, 1024});
    window.getView(0).setViewport({0.f, 0.f, 1.f, 1.f});
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        hg::win::Event ev;
        while (window.pollEvent(ev)) {
            ev.visit(
                [&](const hg::win::Event::Closed&) {
                    window.close();
                },
                [&](const hg::win::Event::MouseButtonPressed& aData) {
                    fixture.onMouseButtonPressed(aData.button,
                                                 window.mapPixelToCoords({aData.x, aData.y}));
                });
        }

        window.clear(hg::gr::COLOR_DARK_GRAY);
        fixture.update();
        fixture.draw(window);
        window.display();
    }
}
} // namespace
} // namespace gridworld

void RunOpennessTest() {
    gridworld::OpennessTestImpl();
}

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Private/Chunk_spooler_default.hpp>
#include <GridGoblin/Private/Chunk_holder.hpp>

#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Grids.hpp>

#include "Fake_disk_io_handler.hpp"

namespace hg = jbatnozic::hobgoblin;

using jbatnozic::gridgoblin::ActiveArea;
using jbatnozic::gridgoblin::Binder;
using jbatnozic::gridgoblin::ChunkId;
using jbatnozic::gridgoblin::WorldConfig;
using jbatnozic::gridgoblin::detail::ChunkSpoolerInterface;
using jbatnozic::gridgoblin::detail::ChunkHolder;

#define CHUNK_COUNT_X 32
#define CHUNK_COUNT_Y 32

namespace {
constexpr auto LOG_ID = "GridGoblin.ManualTest";

template <class taContainer>
bool Contains(const taContainer& aContainer, const typename taContainer::value_type& aValue) {
    for (const auto& item : aContainer) {
        if (item == aValue) {
            return true;
        }
    }
    return false;
}

class FakeWorld
    : public Binder
    , public hg::uwga::Drawable {
public:
    FakeWorld(hg::PZInteger          aWorldWidth,
              hg::PZInteger          aWorldHeight,
              ChunkSpoolerInterface& aChunkSpooler)
        : _chunkStorageHandler{_makeWorldConfig()}
        , _activeArea{_chunkStorageHandler.createNewActiveArea()} //
    {
        _chunkStorageHandler.setChunkSpooler(&aChunkSpooler);
        _chunkStorageHandler.setBinder(this);
    }

    void update(hg::math::Vector2pz aPosition) {
        if (aPosition != _playerPosition) {
            _playerPosition = aPosition;
            const auto id   = ChunkId{(std::uint16_t)aPosition.x, (std::uint16_t)aPosition.y};
            _activeArea.setToChunkRingDiamond(id, 2);
        }

        _chunkStorageHandler.update();
        _chunkStorageHandler.prune();
    }

    void drawOnto(
        hg::uwga::Canvas&             aCanvas,
        const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) const override //
    {
        const auto& uwgaSystem = aCanvas.getSystem();

        const float resolution = 16.f;

        hg::uwga::RectangleShape emptyRect{
            uwgaSystem,
            {resolution, resolution}
        };
        emptyRect.setFillColor(hg::uwga::COLOR_TRANSPARENT);
        emptyRect.setOutlineColor(hg::uwga::COLOR_BLACK);
        emptyRect.setOutlineThickness(2.f);

        hg::uwga::RectangleShape filledRect{
            uwgaSystem,
            {resolution, resolution}
        };
        filledRect.setFillColor(hg::uwga::COLOR_GREEN);

        for (hg::PZInteger y = 0; y < CHUNK_COUNT_Y; y += 1) {
            for (hg::PZInteger x = 0; x < CHUNK_COUNT_X; x += 1) {
                if (_chunkStorageHandler.getCellAtUnchecked(x, y) == nullptr) {
                    emptyRect.setPosition({x * resolution, y * resolution});
                    aCanvas.draw(emptyRect);
                } else {
                    if (Contains(_activeArea.getChunkList(), ChunkId{x, y})) {
                        filledRect.setFillColor(hg::uwga::COLOR_GREEN);
                    } else {
                        filledRect.setFillColor(hg::uwga::COLOR_DARK_TURQUOISE);
                    }
                    filledRect.setPosition({x * resolution, y * resolution});
                    aCanvas.draw(filledRect);
                }
                if (x == _playerPosition.x && y == _playerPosition.y) {
                    hg::uwga::CircleShape circle{uwgaSystem, resolution / 4.f};
                    circle.setFillColor(hg::uwga::COLOR_RED);
                    circle.setOrigin({resolution / 4.f, resolution / 4.f});
                    circle.setPosition({(x + 0.5f) * resolution, (y + 0.5f) * resolution});
                    aCanvas.draw(circle);
                }
            }
        }
    }

private:
    ChunkHolder _chunkStorageHandler;
    hg::math::Vector2pz _playerPosition;
    ActiveArea          _activeArea;

    static WorldConfig _makeWorldConfig() {
        return {.chunkCountX                 = CHUNK_COUNT_X,
                .chunkCountY                 = CHUNK_COUNT_Y,
                .cellsPerChunkX              = 1,
                .cellsPerChunkY              = 1,
                .cellResolution              = 32.f,
                .maxCellOpenness             = 4,
                .maxLoadedNonessentialChunks = 32,
                .chunkDirectoryPath          = "GGManualTest_WorkDir"};
    }
};

class Fixture {
public:
    jbatnozic::gridgoblin::test::FakeDiskIoHandler     _fakeDiskIoHandler;
    jbatnozic::gridgoblin::detail::DefaultChunkSpooler _chunkSpooler;
    FakeWorld                                          _fakeWorld{32, 32, _chunkSpooler};

    Fixture() {
        _chunkSpooler.setDiskIoHandler(&_fakeDiskIoHandler);
    }
};
} // namespace

void RunStorageHandlerTest(int, const char**) {
    Fixture fixture;

    auto system = hg::uwga::CreateGraphicsSystem("SFML");
    auto window =
        system->createRenderWindow(800, 800, hg::uwga::WindowStyle::DEFAULT, "StorageHandlerTest");
    window->setFramerateLimit(60);

    auto view = system->createView();
    view->setCenter({16 * 16.f, 16 * 16.f});
    view->setSize({32 * 16.f, 32 * 16.f});
    view->setViewport({0.f, 0.f, 1.f, 1.f});
    window->setView(*view);

    hg::math::Vector2pz playerPosition{0, 0};
    fixture._fakeWorld.update(playerPosition);

    while (true) {
        hg::uwga::WindowEvent ev;
        while (window && window->pollEvent(ev)) {
            ev.visit(
                [&](const hg::uwga::WindowEvent::Closed&) {
                    window.reset();
                },
                [&](const hg::uwga::WindowEvent::KeyPressed& aData) {
                    switch (aData.physicalKey) {
                    case hg::in::PK_LEFT:
                        if (playerPosition.x > 0) {
                            playerPosition.x -= 1;
                        }
                        break;

                    case hg::in::PK_RIGHT:
                        if (playerPosition.x < CHUNK_COUNT_X - 1) {
                            playerPosition.x += 1;
                        }
                        break;

                    case hg::in::PK_UP:
                        if (playerPosition.y > 0) {
                            playerPosition.y -= 1;
                        }
                        break;

                    case hg::in::PK_DOWN:
                        if (playerPosition.y < CHUNK_COUNT_Y - 1) {
                            playerPosition.y += 1;
                        }
                        break;

                    default:
                        break;
                    }
                });
        }
        if (!window) {
            return;
        }

        window->clear(hg::uwga::COLOR_LIGHT_GRAY);
        fixture._fakeWorld.update(playerPosition);
        window->draw(fixture._fakeWorld);
        window->display();
    }
}

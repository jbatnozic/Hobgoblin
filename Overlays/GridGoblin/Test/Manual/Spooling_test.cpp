// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Grids.hpp>

#include "Fake_disk_io_handler.hpp"
#include <GridGoblin/Private/Chunk_impl.hpp>
#include <GridGoblin/Private/Chunk_memory_layout_info.hpp>
#include <GridGoblin/Private/Chunk_spooler_default.hpp>

#include <memory>
#include <unordered_map>

namespace hg = jbatnozic::hobgoblin;

using jbatnozic::gridgoblin::BuildingBlockMask;
using jbatnozic::gridgoblin::Chunk;
using jbatnozic::gridgoblin::ChunkId;
using jbatnozic::gridgoblin::ChunkMemoryLayoutInfo;
using jbatnozic::gridgoblin::detail::ChunkSpoolerInterface;

#define CHUNK_COUNT_X 32
#define CHUNK_COUNT_Y 32

namespace {
constexpr auto LOG_ID = "GridGoblin.ManualTest";

class FakeWorld : public hg::uwga::Drawable {
public:
    FakeWorld(hg::PZInteger                aWorldWidth,
              hg::PZInteger                aWorldHeight,
              ChunkSpoolerInterface&       aChunkSpooler,
              const ChunkMemoryLayoutInfo& aChunkMemLayout)
        : _chunkspool{aChunkSpooler}
        , _chunkMemLayout{aChunkMemLayout} //
    {
        _chunkGrid.reset(aWorldWidth, aWorldHeight);
    }

    void update(hg::math::Vector2pz aPosition) {
        _chunkspool.pause();
        _playerPosition = aPosition;

        // Collect loaded chunks
        {
            for (auto iter = _requests.begin(); iter != _requests.end();) {
                auto& handle = iter->second;
                HG_HARD_ASSERT(handle != nullptr);

                if (handle->isFinished()) {
                    const auto id    = handle->getChunkId();
                    auto       chunk = handle->takeChunk();
                    if (chunk) {
                        HG_HARD_ASSERT(_chunkGrid.at(id.y, id.x) == nullptr);
                        _chunkGrid.at(id.y, id.x) = std::make_unique<Chunk>(std::move(*chunk));
                        HG_LOG_INFO(LOG_ID, "Loaded chunk {}, {} from disk.", id.x, id.y);
                    } else {
                        // create new one
                        _chunkGrid.at(id.y, id.x) = std::make_unique<Chunk>(_chunkMemLayout);
                        HG_LOG_INFO(LOG_ID, "Created chunk {}, {}.", id.x, id.y);
                    }
                    iter = _requests.erase(iter);
                } else {
                    iter = std::next(iter);
                }
            }
        }

        // Unload excess tiles
        {
            for (hg::PZInteger y = 0; y < _chunkGrid.getHeight(); y += 1) {
                for (hg::PZInteger x = 0; x < _chunkGrid.getWidth(); x += 1) {
                    auto& chunkPtr = _chunkGrid.at(y, x);
                    if (!chunkPtr) {
                        continue;
                    }
                    if (std::abs(_playerPosition.x - x) > 2 || std::abs(_playerPosition.y - y) > 2) {
                        _chunkspool.unloadChunk({(std::uint16_t)x, (std::uint16_t)y},
                                                std::move(*chunkPtr));
                        chunkPtr.reset();
                        HG_LOG_INFO(LOG_ID, "Unloaded chunk {}, {}.", x, y);
                    }
                }
            }
        }

        // Cancel requests we don't need anymore
        {
            for (auto iter = _requests.begin(); iter != _requests.end();) {
                auto& handle = iter->second;
                HG_HARD_ASSERT(handle != nullptr);

                const auto id = handle->getChunkId();
                if (std::abs(_playerPosition.x - id.x) > 2 || std::abs(_playerPosition.y - id.y) > 2) {
                    handle->cancel();
                    iter = _requests.erase(iter);
                } else {
                    iter = std::next(iter);
                }
            }
        }

        // Set new chunks to load
        {
            std::vector<ChunkSpoolerInterface::LoadRequest> loadRequests;
            for (int yOff = -2; yOff <= 2; yOff += 1) {
                for (int xOff = -2; xOff <= 2; xOff += 1) {
                    const auto x = _playerPosition.x + xOff;
                    const auto y = _playerPosition.y + yOff;
                    if (x >= 0 && x < CHUNK_COUNT_X && y >= 0 && y < CHUNK_COUNT_Y) {
                        if (_chunkGrid[y][x] == nullptr) {
                            const auto reqIter = _requests.find({(std::uint16_t)x, (std::uint16_t)y});
                            if (reqIter == _requests.end()) {
                                ChunkSpoolerInterface::LoadRequest lr;
                                lr.chunkId  = {(std::uint16_t)x, (std::uint16_t)y};
                                lr.priority = (std::abs(xOff) + std::abs(yOff));
                                loadRequests.push_back(lr);
                            } else {
                                reqIter->second->trySwapPriority((std::abs(xOff) + std::abs(yOff)));
                            }
                        }
                    }
                }
            }
            auto handles = _chunkspool.loadChunks(std::move(loadRequests));
            for (auto& handle : handles) {
                // const auto iter = _requests.find(handle->getChunkId());
                // HG_HARD_ASSERT(iter != _requests.end());
                // (*iter).second = std::move(handle);
                _requests[handle->getChunkId()] = std::move(handle);
            }
        }

        _chunkspool.unpause();
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

        for (hg::PZInteger y = 0; y < _chunkGrid.getHeight(); y += 1) {
            for (hg::PZInteger x = 0; x < _chunkGrid.getWidth(); x += 1) {
                if (_chunkGrid[y][x] == nullptr) {
                    emptyRect.setPosition({x * resolution, y * resolution});
                    aCanvas.draw(emptyRect);
                } else {
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
    ChunkSpoolerInterface& _chunkspool;

    const ChunkMemoryLayoutInfo& _chunkMemLayout;

    hg::util::RowMajorGrid<std::unique_ptr<Chunk>> _chunkGrid;

    std::unordered_map<ChunkId, std::shared_ptr<ChunkSpoolerInterface::RequestHandleInterface>>
        _requests;

    hg::math::Vector2pz _playerPosition{0, 0};
};

class Fixture {
public:
    ChunkMemoryLayoutInfo _chunkMemoryLayoutInfoImpl =
        jbatnozic::gridgoblin::detail::CalcChunkMemoryLayoutInfo(1, 1, BuildingBlockMask::ALL);

    jbatnozic::gridgoblin::test::FakeDiskIoHandler     _fakeDiskIoHandler;
    jbatnozic::gridgoblin::detail::DefaultChunkSpooler _chunkSpooler{BuildingBlockMask::ALL,
                                                                     _chunkMemoryLayoutInfoImpl};
    FakeWorld _fakeWorld{32, 32, _chunkSpooler, _chunkMemoryLayoutInfoImpl};

    Fixture() {
        _chunkSpooler.setDiskIoHandler(&_fakeDiskIoHandler);
    }
};
} // namespace

void RunSpoolingTest(int, const char**) {
    Fixture fixture;

    auto system = hg::uwga::CreateGraphicsSystem("SFML");
    auto window = system->createRenderWindow(800, 800, hg::uwga::WindowStyle::DEFAULT, "SpoolingTest");
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
            break;
        }

        window->clear(hg::uwga::COLOR_LIGHT_GRAY);
        fixture._fakeWorld.update(playerPosition);
        window->draw(fixture._fakeWorld);
        window->display();
    }
}

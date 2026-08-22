// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <InteriorWorld/Interior_world.hpp>

#include <InteriorWorld/Cell_archs.hpp>

namespace cinnabar {

namespace grid = ::jbatnozic::gridgoblin;

namespace {
#define GRID_RESOLUTION 48.f

// clang-format off
constexpr grid::ContentsConfig WORLD_CONFIG = {
    .chunkCountX     = 64,
    .chunkCountY     = 128,
    .cellsPerChunkX  = 16,
    .cellsPerChunkY  = 16,
    .buildingBlocks  = grid::BuildingBlockMask::ALL,
    .cellResolution  = GRID_RESOLUTION,
    .wallHeight      = GRID_RESOLUTION,
    .maxCellOpenness = 0,
    .maxLoadedNonessentialChunks = 0xFFFFFF
};
// clang-format on

constexpr hg::PZInteger CELLS_PER_CHUNK_X = WORLD_CONFIG.cellsPerChunkX;
constexpr hg::PZInteger CELLS_PER_CHUNK_Y = WORLD_CONFIG.cellsPerChunkY;

static_assert(InteriorWorld::CENTER_OFFSET.x == WORLD_CONFIG.chunkCountX * GRID_RESOLUTION / 2.0);
static_assert(InteriorWorld::CENTER_OFFSET.y == WORLD_CONFIG.chunkCountY * GRID_RESOLUTION / 2.0);

} // namespace

InteriorWorld::InteriorWorld()
    : _world{WORLD_CONFIG} {}

const jbatnozic::gridgoblin::World& InteriorWorld::getUnderlying() const {
    return _world;
}

// MARK: GridGoblin Binder

void InteriorWorld::didPrepareChunk(grid::ChunkId aChunkId) { /* No implementation needed */ }

void InteriorWorld::willIntegrateNewChunk(grid::ChunkId                      aId,
                                          grid::Chunk&                       aChunk,
                                          const grid::ChunkMemoryLayoutInfo& aChunkMemLayout) {
    const grid::FatCell fatCell = {
        .cellKindId = interior::cell_archetype::SOLID_VOID.cellKindId,
        .floorSprite = interior::cell_archetype::SOLID_VOID.floorSprite,
        .wallSprite = interior::cell_archetype::SOLID_VOID.wallSprite,
        .spatialInfo = interior::cell_archetype::SOLID_VOID.spatialInfo
    };
    aChunk.setAll(aChunkMemLayout, fatCell);

    const jbatnozic::gridgoblin::cell::UserData userData = {.i64 = 0};
    for (hg::PZInteger y = 0; y < WORLD_CONFIG.cellsPerChunkX; ++y) {
        for (hg::PZInteger x = 0; x < WORLD_CONFIG.cellsPerChunkY; ++x) {
            aChunk.setCellDataAtUnchecked(aChunkMemLayout, {x, y}, &userData);
        }
    }
}

void InteriorWorld::willIntegrateLoadedChunk(grid::ChunkId                      aId,
                                             grid::Chunk&                       aChunk,
                                             const grid::ChunkMemoryLayoutInfo& aChunkMemLayout) {
    // TODO
}

void InteriorWorld::didIntegrateChunk(grid::ChunkId                      aId,
                                      const grid::Chunk&                 aChunk,
                                      const grid::ChunkMemoryLayoutInfo& aChunkMemLayout) {
    // TODO
}

void InteriorWorld::willSeparateChunk(grid::ChunkId                      aId,
                                      const grid::Chunk&                 aChunk,
                                      const grid::ChunkMemoryLayoutInfo& aChunkMemLayout) {
    // TODO
}

void InteriorWorld::didSeparateChunk(grid::ChunkId                      aId,
                                     grid::Chunk&                       aChunk,
                                     const grid::ChunkMemoryLayoutInfo& aChunkMemLayout) {
    // TODO
}

void InteriorWorld::didEditCells(const grid::CellEditInfos& aCellEditInfos) {
    // TODO: sync to clients
}

std::unique_ptr<grid::ChunkExtensionInterface> InteriorWorld::createChunkExtension() {
    return nullptr; // TODO
}

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Interior_world.hpp"

#include <CBR/QAOMessages/Sync_create_and_update_to_client.hpp>

#include <Hobgoblin/Common/Build_type.hpp>
#include <Hobgoblin/Utility/Compact_int.hpp>
#include <Hobgoblin/Utility/On_scope_exit.hpp>

#include "Cell_archs.hpp"
#include "Cell_props.hpp"

namespace cinnabar {

namespace grid = ::jbatnozic::gridgoblin;

namespace {
#define GRID_RESOLUTION InteriorWorld::CELL_RESOLUTION

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

static_assert(InteriorWorld::CELL_COUNT_X == WORLD_CONFIG.chunkCountX * CELLS_PER_CHUNK_X);
static_assert(InteriorWorld::CELL_COUNT_Y == WORLD_CONFIG.chunkCountY * CELLS_PER_CHUNK_Y);

static_assert(InteriorWorld::CENTER_OFFSET.x == WORLD_CONFIG.chunkCountX * GRID_RESOLUTION / 2.0);
static_assert(InteriorWorld::CENTER_OFFSET.y == WORLD_CONFIG.chunkCountY * GRID_RESOLUTION / 2.0);

// clang-format off
constexpr grid::FatCell DEFAULT_CELL = {
    .cellKindId  = interior::cell_archetype::SOLID_VOID.cellKindId,
    .floorSprite = interior::cell_archetype::SOLID_VOID.floorSprite,
    .wallSprite  = interior::cell_archetype::SOLID_VOID.wallSprite,
    .spatialInfo = interior::cell_archetype::SOLID_VOID.spatialInfo
};
// clang-format on

template <class taCallable>
void ForEachCellInChunkDo(jbatnozic::gridgoblin::World&  aWorld,
                          jbatnozic::gridgoblin::ChunkId aChunkId,
                          taCallable&&                   aCallable) {
    for (hg::PZInteger y = 0; y < CELLS_PER_CHUNK_Y; ++y) {
        for (hg::PZInteger x = 0; x < CELLS_PER_CHUNK_X; ++x) {
            const auto globalX = aChunkId.x * CELLS_PER_CHUNK_X + x;
            const auto globalY = aChunkId.y * CELLS_PER_CHUNK_Y + y;
            aCallable(globalX, globalY);
        }
    }
}
} // namespace

InteriorWorld::InteriorWorld(Mode aMode, spe::GameContext& aContext)
    : _mode{aMode}
    , _world{WORLD_CONFIG}
    , _internalRuntime{*this, &aContext, nullptr} //
{
    _world.attachBinder(this);
}

void InteriorWorld::onBeginUpdate() {
     _cellEditInfos.clear();
}

void InteriorWorld::onEndUpdate() {
     _cellEditInfos.clear();
}

const jbatnozic::gridgoblin::World& InteriorWorld::getUnderlying() const {
    return _world;
}

void InteriorWorld::writeWholeCellState(hg::util::OutputStream& aOStream) const {
    const auto& memLayout = _world.getChunkMemoryLayoutInfo();
    const auto  endIter   = _world.availableChunksEnd();

    for (auto iter = _world.availableChunksBegin(); iter != endIter; ++iter) {
        const auto chunkId = *iter;

        // Write Chunk ID (uint16 x/y)
        aOStream << static_cast<std::uint16_t>(chunkId.x);
        aOStream << static_cast<std::uint16_t>(chunkId.y);

        const auto* chunk = _world.getChunkAtId(chunkId);
        HG_ASSERT(chunk != nullptr);

        interior::CellProperties cellProps;
        for (hg::PZInteger y = 0; y < CELLS_PER_CHUNK_Y; ++y) {
            for (hg::PZInteger x = 0; x < CELLS_PER_CHUNK_X; ++x) {
                interior::CellProperties::readFromChunk(memLayout, *chunk, x, y, cellProps);
                interior::CellProperties::writeToStreamFully(cellProps, aOStream);
            }
        }
    }

    // Write end marker (as far as GridGoblin is concerned, 65535 is a valid X coordinate for a chunk,
    // but interior worlds can never be that large).
    aOStream << static_cast<std::uint16_t>(0xFFFF);
}

void InteriorWorld::readWholeCellState(hg::util::InputStream& aIStream) {
    _world.toggleEditJournaling(false);
    HG_ON_SCOPE_EXIT_CAP1(this, _world.toggleEditJournaling(_getPreferredWorldEditJournalingFlag()));

    _world.edit(
        *_world.getPermissionToEdit(),
        [this, &aIStream](jbatnozic::gridgoblin::World::Editor& aEditor) {
            interior::CellProperties cellProps;

            // First set all available chunks (if any) to their default values
            const auto endIter = _world.availableChunksEnd();
            for (auto iter = _world.availableChunksBegin(); iter != endIter; ++iter) {
                ForEachCellInChunkDo(_world, *iter, [&aEditor](hg::PZInteger aX, hg::PZInteger aY) {
                    aEditor.setCellDataAtUnchecked(aX,
                                                   aY,
                                                   &DEFAULT_CELL.cellKindId,
                                                   &DEFAULT_CELL.floorSprite,
                                                   &DEFAULT_CELL.wallSprite,
                                                   &DEFAULT_CELL.spatialInfo,
                                                   &DEFAULT_CELL.userData);
                });
            }

            // Then read cell data from the stream
            while (!aIStream.endOfStream()) {
                jbatnozic::gridgoblin::ChunkId chunkId;
                chunkId.x = aIStream.extract<std::uint16_t>();
                if (chunkId.x == static_cast<std::uint16_t>(0xFFFF)) {
                    break;
                }
                chunkId.y = aIStream.extract<std::uint16_t>();

                // clang-format off
                ForEachCellInChunkDo(
                    _world,
                    chunkId,
                    [&aEditor, &aIStream, &cellProps](hg::PZInteger aX, hg::PZInteger aY) {
                        interior::CellProperties::readFromStreamFully(aIStream, cellProps);
                        aEditor.setCellDataAtUnchecked(aX, aY,
                                                       &cellProps.cellKindId,
                                                       &cellProps.floorSprite,
                                                       &cellProps.wallSprite,
                                                       &cellProps.spatialInfo,
                                                       &cellProps.userData);
                });
                // clang-format on
            }
        });

    _world.toggleEditJournaling(_getPreferredWorldEditJournalingFlag());
}

bool InteriorWorld::writeCellStateDiff(hg::util::OutputStream& aOStream) const {
    if (_cellEditInfos.empty()) {
        return false;
    }

    aOStream.append<hg::util::CompactUInt28>(static_cast<std::uint32_t>(_cellEditInfos.size()));

    interior::CellProperties cellProps;
    for (const auto& [cellPos, cellEditInfo] : _cellEditInfos) {
        interior::CellProperties::readFromWorld(_world, cellPos.x, cellPos.y, cellProps);
        aOStream << static_cast<std::uint16_t>(cellPos.x) << static_cast<std::uint16_t>(cellPos.y);
        interior::CellProperties::writeToStreamWithBBMask(cellProps, aOStream, cellEditInfo.bbMask);
    }

    return true;
}

void InteriorWorld::readAndApplyCellStateDiff(hg::util::InputStream& aIStream) {
    using grid::BuildingBlock;

    const auto cellCount = aIStream.extract<hg::util::CompactUInt28>().value();

    _world.edit(*_world.getPermissionToEdit(), [&aIStream, cellCount](grid::World::Editor& aEditor) {
        interior::CellProperties cellProps;
        for (std::uint32_t i = 0; i < cellCount; ++i) {
            const hg::PZInteger x = aIStream.extract<std::uint16_t>();
            const hg::PZInteger y = aIStream.extract<std::uint16_t>();

            const auto mask = interior::CellProperties::readFromStreamWithBBMask(aIStream, cellProps);

            if ((mask & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
                aEditor.setCellDataAt(x, y, &cellProps.cellKindId);
            }
            if ((mask & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
                aEditor.setCellDataAt(x, y, &cellProps.floorSprite);
            }
            if ((mask & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
                aEditor.setCellDataAt(x, y, &cellProps.wallSprite);
            }
            if ((mask & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
                aEditor.setCellDataAt(x, y, &cellProps.spatialInfo);
            }
            if ((mask & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
                aEditor.setCellDataAt(x, y, &cellProps.userData);
            }
        }
    });
}

void InteriorWorld::doFullSyncOfCreateAndUpdateOfEntireInternalRuntimeForClient(
    hg::PZInteger aClientIndex) //
{
    // clang-format off
    const auto recepientInfo = SyncRecepientInfo{
        .clientIndex = aClientIndex,
        .syncFlags   = spe::SyncFlags::FULL_STATE
    };
    // clang-format on

    const auto endIter = _internalRuntime.end();
    for (auto iter = _internalRuntime.begin(); iter != endIter; ++iter) {
        const QAO_GenericHandle     handle = *iter;
        [[maybe_unused]] const bool didAnswer =
            QAO_SendMessage<SyncCreateAndUpdateToClient>(*handle, &recepientInfo);
#if HG_BUILD_TYPE == HG_DEBUG
        if (!didAnswer) {
            const auto* syncObj = dynamic_cast<spe::SynchronizedObjectBase*>(handle.ptr());
            if (syncObj) {
                HG_THROW_TRACED(
                    hg::TracedLogicError,
                    0,
                    "Object {} is a Synchronized Object but did not answer to QAO Message '{}'!",
                    syncObj->getDebugDescription(),
                    "SyncCreateAndUpdateToClient");
            }
        }
#endif
    }
}

bool InteriorWorld::_getPreferredWorldEditJournalingFlag() const {
    return (_mode == Mode::MASTER);
}

// MARK: GridGoblin Binder

void InteriorWorld::didPrepareChunk(grid::ChunkId aChunkId) { /* No implementation needed */ }

void InteriorWorld::willIntegrateNewChunk(grid::ChunkId                      aId,
                                          grid::Chunk&                       aChunk,
                                          const grid::ChunkMemoryLayoutInfo& aChunkMemLayout) {
    aChunk.setAll(aChunkMemLayout, DEFAULT_CELL);

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
    for (const auto& [cellPos, cellEditInfo] : aCellEditInfos) {
        // Note: CellEditInfo default-initializes to an empty (all-zero) mask
        auto& mask = this->_cellEditInfos[cellPos].bbMask;
        mask = mask | cellEditInfo.bbMask;
    }
}

std::unique_ptr<grid::ChunkExtensionInterface> InteriorWorld::createChunkExtension() {
    return nullptr; // TODO
}

} // namespace cinnabar

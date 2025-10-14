// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Private/Chunk_memory_layout_info.hpp>

#include <Hobgoblin/Math/Core.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

namespace {
void IncUntilAligned(std::size_t& aByteCounter, std::size_t aAlignment) {
    aByteCounter = hg::math::IntegralCeilDiv(aByteCounter, aAlignment) * aAlignment;
}
} // namespace

class ChunkExtensionInterface;

#define acc ChunkMemoryLayoutInfoAccessor

ChunkMemoryLayoutInfo CalcChunkMemoryLayoutInfo(hg::PZInteger     aChunkWidth,
                                                hg::PZInteger     aChunkHeight,
                                                BuildingBlockMask aBuildingBlocks) {
    ChunkMemoryLayoutInfo meminfo;
    acc::setChunkWidth(meminfo, aChunkWidth);
    acc::setChunkHeight(meminfo, aChunkHeight);

    const auto cellCount = static_cast<std::size_t>(aChunkWidth * aChunkHeight);

    std::size_t byteCounter = 0;

    // Extension
    {
        // IMPORTANT: Extension pointer has to always be at offset zero, so that it can be cleaned
        //            up in the destructor without having access to a `MemoryLayoutInfo` object.
        // meminfo.offset.extension = byteCounter;
        byteCounter += 1 * sizeof(ChunkExtensionInterface*);
    }

    // Cell kind ID
    if ((aBuildingBlocks & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::CellKindId));
        acc::setCellKindIdOffset(meminfo, byteCounter);
        byteCounter += cellCount * sizeof(cell::CellKindId);
    }

    // Floor sprite
    if ((aBuildingBlocks & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::FloorSprite));
        acc::setFloorSpriteOffset(meminfo, byteCounter);
        byteCounter += cellCount * sizeof(cell::FloorSprite);
    }

    // Wall sprite
    if ((aBuildingBlocks & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::WallSprite));
        acc::setWallSpriteOffset(meminfo, byteCounter);
        byteCounter += cellCount * sizeof(cell::WallSprite);
    }

    // Spatial info
    if ((aBuildingBlocks & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::SpatialInfo));
        acc::setSpatialInfoOffset(meminfo, byteCounter);
        byteCounter += cellCount * sizeof(cell::SpatialInfo);
    }

    // Renderer aux data
    if ((aBuildingBlocks & BuildingBlock::RENDERER_AUX_DATA) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::RendererAuxData));
        acc::setRendererAuxDataOffset(meminfo, byteCounter);
        byteCounter += cellCount * sizeof(cell::RendererAuxData);
    }

    // User data
    if ((aBuildingBlocks & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::UserData));
        acc::setUserDataOffset(meminfo, byteCounter);
        byteCounter += cellCount * sizeof(cell::UserData);
    }

    acc::setTotalSize(meminfo, byteCounter);

    return meminfo;
}

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic

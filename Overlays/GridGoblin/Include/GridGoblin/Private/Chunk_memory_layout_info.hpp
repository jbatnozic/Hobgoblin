// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Building_block.hpp>

#include <Hobgoblin/Common.hpp>

#include <cstddef>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

namespace detail {
class ChunkMemoryLayoutInfoAccessor;
} // namespace detail

struct ChunkMemoryLayoutInfo {
private:
    hg::PZInteger chunkWidth;  //!< Number of columns in the chunk
    hg::PZInteger chunkHeight; //!< Number of rows in the chunk

    static constexpr std::size_t INVALID_OFFSET = 999'999'999;

    //! Memory offsets of specific building blocks of a chunk, expressed in bytes
    //! (relative to the beginning of the chunk's memory block).
    struct Offsets {
        std::size_t cellKindId      = INVALID_OFFSET;
        std::size_t floorSprite     = INVALID_OFFSET;
        std::size_t wallSprite      = INVALID_OFFSET;
        std::size_t spatialInfo     = INVALID_OFFSET;
        std::size_t rendererAuxData = INVALID_OFFSET;
        std::size_t userData        = INVALID_OFFSET;
    };

    Offsets     offset;
    std::size_t totalSize;

    friend class detail::ChunkMemoryLayoutInfoAccessor;
};

namespace detail {
ChunkMemoryLayoutInfo CalcChunkMemoryLayoutInfo(hg::PZInteger     aChunkWidth,
                                                hg::PZInteger     aChunkHeight,
                                                BuildingBlockMask aBuildingBlocks);

class ChunkMemoryLayoutInfoAccessor {
public:
    static constexpr std::size_t INVALID_OFFSET = ChunkMemoryLayoutInfo::INVALID_OFFSET;

    // Getters

    static hg::PZInteger getChunkWidth(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.chunkWidth;
    }

    static hg::PZInteger getChunkHeight(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.chunkHeight;
    }

    static std::size_t getCellKindIdOffset(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.offset.cellKindId;
    }

    static std::size_t getFloorSpriteOffset(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.offset.floorSprite;
    }

    static std::size_t getWallSpriteOffset(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.offset.wallSprite;
    }

    static std::size_t getSpatialInfoOffset(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.offset.spatialInfo;
    }

    static std::size_t getRendererAuxDataOffset(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.offset.rendererAuxData;
    }

    static std::size_t getUserDataOffset(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.offset.userData;
    }

    static std::size_t getTotalSize(const ChunkMemoryLayoutInfo& aMemLayout) {
        return aMemLayout.totalSize;
    }

    // Setters

    static void setChunkWidth(ChunkMemoryLayoutInfo& aMemLayout, hg::PZInteger width) {
        aMemLayout.chunkWidth = width;
    }

    static void setChunkHeight(ChunkMemoryLayoutInfo& aMemLayout, hg::PZInteger height) {
        aMemLayout.chunkHeight = height;
    }

    static void setCellKindIdOffset(ChunkMemoryLayoutInfo& aMemLayout, std::size_t aOffset) {
        aMemLayout.offset.cellKindId = aOffset;
    }

    static void setFloorSpriteOffset(ChunkMemoryLayoutInfo& aMemLayout, std::size_t aOffset) {
        aMemLayout.offset.floorSprite = aOffset;
    }

    static void setWallSpriteOffset(ChunkMemoryLayoutInfo& aMemLayout, std::size_t aOffset) {
        aMemLayout.offset.wallSprite = aOffset;
    }

    static void setSpatialInfoOffset(ChunkMemoryLayoutInfo& aMemLayout, std::size_t aOffset) {
        aMemLayout.offset.spatialInfo = aOffset;
    }

    static void setRendererAuxDataOffset(ChunkMemoryLayoutInfo& aMemLayout, std::size_t aOffset) {
        aMemLayout.offset.rendererAuxData = aOffset;
    }

    static void setUserDataOffset(ChunkMemoryLayoutInfo& aMemLayout, std::size_t aOffset) {
        aMemLayout.offset.userData = aOffset;
    }

    static void setTotalSize(ChunkMemoryLayoutInfo& aMemLayout, std::size_t size) {
        aMemLayout.totalSize = size;
    }
};
} // namespace detail

} // namespace gridgoblin
} // namespace jbatnozic

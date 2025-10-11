// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Chunk.hpp>

#include <GridGoblin/Model/Chunk_extension.hpp>

#include <Hobgoblin/HGExcept.hpp>

namespace jbatnozic {
namespace gridgoblin {

Chunk::Chunk(const ChunkMemoryLayoutInfo& aMemLayout) {
    _impl.alloc(aMemLayout);
}

void Chunk::zeroOut(const ChunkMemoryLayoutInfo& aMemLayout) {
    _impl.zeroOut(aMemLayout);
}

void Chunk::setAll(const ChunkMemoryLayoutInfo& aMemLayout, const FatCell& aCell) {
    _impl.setAll(aMemLayout, aCell);
}

void Chunk::setExtension(std::unique_ptr<ChunkExtensionInterface> aChunkExtension) {
    _impl.setExtension(std::move(aChunkExtension));
}

std::unique_ptr<ChunkExtensionInterface> Chunk::releaseExtension() {
    return _impl.releaseExtension();
}

///////////////////////////////////////////////////////////////////////////
// FREE FUNCTIONS                                                        //
///////////////////////////////////////////////////////////////////////////

namespace detail {
#define acc ChunkMemoryLayoutInfoAccessor
bool ChunksContainIdenticalCellData(const ChunkMemoryLayoutInfo& aMemLayout,
                                    const Chunk&                 aChunk1,
                                    const Chunk&                 aChunk2) {
    HG_VALIDATE_PRECONDITION(!aChunk1.isEmpty());
    HG_VALIDATE_PRECONDITION(!aChunk2.isEmpty());

    const auto width  = acc::getChunkWidth(aMemLayout);
    const auto height = acc::getChunkHeight(aMemLayout);

    if (auto offset = acc::getCellKindIdOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                cell::CellKindId cellKindIds[2];
                aChunk1.getCellDataAtUnchecked(aMemLayout, {x, y}, &cellKindIds[0]);
                aChunk2.getCellDataAtUnchecked(aMemLayout, {x, y}, &cellKindIds[1]);
                if (cellKindIds[0] != cellKindIds[1]) {
                    return false;
                }
            }
        }
    }

    if (auto offset = acc::getFloorSpriteOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                cell::FloorSprite floorSprites[2];
                aChunk1.getCellDataAtUnchecked(aMemLayout, {x, y}, &floorSprites[0]);
                aChunk2.getCellDataAtUnchecked(aMemLayout, {x, y}, &floorSprites[1]);
                if (floorSprites[0] != floorSprites[1]) {
                    return false;
                }
            }
        }
    }

    if (auto offset = acc::getWallSpriteOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                cell::WallSprite wallSprites[2];
                aChunk1.getCellDataAtUnchecked(aMemLayout, {x, y}, &wallSprites[0]);
                aChunk2.getCellDataAtUnchecked(aMemLayout, {x, y}, &wallSprites[1]);
                if (wallSprites[0] != wallSprites[1]) {
                    return false;
                }
            }
        }
    }

    if (auto offset = acc::getSpatialInfoOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                cell::SpatialInfo spatialInfos[2];
                aChunk1.getCellDataAtUnchecked(aMemLayout, {x, y}, &spatialInfos[0]);
                aChunk2.getCellDataAtUnchecked(aMemLayout, {x, y}, &spatialInfos[1]);
                if (spatialInfos[0] != spatialInfos[1]) {
                    return false;
                }
            }
        }
    }

    if (auto offset = acc::getRendererAuxDataOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                cell::RendererAuxData rendererAuxData[2];
                aChunk1.getCellDataAtUnchecked(aMemLayout, {x, y}, &rendererAuxData[0]);
                aChunk2.getCellDataAtUnchecked(aMemLayout, {x, y}, &rendererAuxData[1]);
                if (rendererAuxData[0] != rendererAuxData[1]) {
                    return false;
                }
            }
        }
    }

    if (auto offset = acc::getUserDataOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                cell::UserData userData[2];
                aChunk1.getCellDataAtUnchecked(aMemLayout, {x, y}, &userData[0]);
                aChunk2.getCellDataAtUnchecked(aMemLayout, {x, y}, &userData[1]);
                if (userData[0] != userData[1]) {
                    return false;
                }
            }
        }
    }

    return true;
}
} // namespace detail

} // namespace gridgoblin
} // namespace jbatnozic

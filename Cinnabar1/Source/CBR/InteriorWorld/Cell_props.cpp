// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Cell_props.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <GridGoblin/World/World.hpp>

namespace cinnabar {
namespace interior {

void CellProperties::readFromWorld(const jbatnozic::gridgoblin::World& aWorld,
                                   hg::PZInteger                       aX,
                                   hg::PZInteger                       aY,
                                   CellProperties&                     aCellProps) {
    const bool success = aWorld.getCellDataAt(aX,
                                              aY,
                                              &aCellProps.cellKindId,
                                              &aCellProps.floorSprite,
                                              &aCellProps.wallSprite,
                                              &aCellProps.spatialInfo,
                                              &aCellProps.userData);
    HG_HARD_ASSERT(success);
}

void CellProperties::readFromChunk(const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aMemLayout,
                                   const jbatnozic::gridgoblin::Chunk&                 aChunk,
                                   hg::PZInteger                                       aX,
                                   hg::PZInteger                                       aY,
                                   CellProperties&                                     aCellProps) {
    aChunk.getCellDataAtUnchecked(aMemLayout,
                                  aX,
                                  aY,
                                  &aCellProps.cellKindId,
                                  &aCellProps.floorSprite,
                                  &aCellProps.wallSprite,
                                  &aCellProps.spatialInfo,
                                  &aCellProps.userData);
}

void CellProperties::writeToStreamFully(const CellProperties&   aCellProps,
                                        hg::util::OutputStream& aOStream) {
    aOStream << aCellProps.cellKindId << aCellProps.floorSprite << aCellProps.wallSprite
             << aCellProps.spatialInfo << aCellProps.userData;
}

void CellProperties::readFromStreamFully(hg::util::InputStream& aIStream, CellProperties& aCellProps) {
    aIStream >> aCellProps.cellKindId >> aCellProps.floorSprite >> aCellProps.wallSprite >>
        aCellProps.spatialInfo >> aCellProps.userData;
}

using ::jbatnozic::gridgoblin::BuildingBlock;
using ::jbatnozic::gridgoblin::BuildingBlockMask;

// Check that we can store the BBMask safely in 8 bits
static_assert(static_cast<std::int64_t>(BuildingBlockMask::SIGNIFICANT_BITS) <= 0xFFull);

void CellProperties::writeToStreamWithBBMask(const CellProperties&                    aCellProps,
                                             hg::util::OutputStream&                  aOStream,
                                             jbatnozic::gridgoblin::BuildingBlockMask aBBMask) {
    aOStream << static_cast<std::uint8_t>(aBBMask & BuildingBlockMask::SIGNIFICANT_BITS);

    if ((aBBMask & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
        aOStream << aCellProps.cellKindId;
    }
    if ((aBBMask & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
        aOStream << aCellProps.floorSprite;
    }
    if ((aBBMask & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
        aOStream << aCellProps.wallSprite;
    }
    if ((aBBMask & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
        aOStream << aCellProps.spatialInfo;
    }
    if ((aBBMask & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
        aOStream << aCellProps.userData;
    }
}

jbatnozic::gridgoblin::BuildingBlockMask CellProperties::readFromStreamWithBBMask(
    hg::util::InputStream& aIStream,
    CellProperties&        aCellProps) //
{
    const auto mask = static_cast<BuildingBlockMask>(aIStream.extractNoThrow<std::uint8_t>());

    if ((mask & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
        aIStream >> aCellProps.cellKindId;
    }
    if ((mask & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
        aIStream >> aCellProps.floorSprite;
    }
    if ((mask & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
        aIStream >> aCellProps.wallSprite;
    }
    if ((mask & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
        aIStream >> aCellProps.spatialInfo;
    }
    if ((mask & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
        aIStream >> aCellProps.userData;
    }

    return mask;
}

} // namespace interior
} // namespace cinnabar

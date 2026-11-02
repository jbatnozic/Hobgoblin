// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/InteriorWorld/Cell_archs.hpp>

#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_id.hpp>
#include <Hobgoblin/Utility/Grids.hpp>
#include <Hobgoblin/Utility/Stream_input.hpp>
#include <Hobgoblin/Utility/Stream_output.hpp>

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {
// Forward-declare
class World;
} // namespace gridgoblin
} // namespace jbatnozic

namespace cinnabar {
namespace interior {

struct CellProperties : CellArchProperties {
    jbatnozic::gridgoblin::cell::UserData userData;

    //! TODO: add description
    static void readFromWorld(const jbatnozic::gridgoblin::World& aWorld,
                              hg::PZInteger                       aX,
                              hg::PZInteger                       aY,
                              CellProperties&                     aCellProps);

    static void readFromChunk(const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aMemLayout,
                              const jbatnozic::gridgoblin::Chunk&                 aChunk,
                              hg::PZInteger                                       aX,
                              hg::PZInteger                                       aY,
                              CellProperties&                                     aCellProps);

    //! TODO: add description
    static void writeToStreamFully(const CellProperties& aCellProps, hg::util::OutputStream& aOStream);

    //! TODO: add description
    static void readFromStreamFully(hg::util::InputStream& aIStream, CellProperties& aCellProps);

    //! TODO: add description
    static void writeToStreamWithBBMask(const CellProperties&                    aCellProps,
                                        hg::util::OutputStream&                  aOStream,
                                        jbatnozic::gridgoblin::BuildingBlockMask aBBMask);

    //! TODO: add description
    static jbatnozic::gridgoblin::BuildingBlockMask readFromStreamWithBBMask(
        hg::util::InputStream& aIStream,
        CellProperties&        aCellProps);
};

using CellPropGrid = jbatnozic::hobgoblin::util::RowMajorGrid<CellProperties>;

// IW Cell User Data Bit Allocation:
// (available bits = 0..63)
//  0..10 [11 bits] = parent attachable ID
// 11..63 [53 bits] = reserved

inline std::int16_t UserData_GetParentAttachableId(jbatnozic::gridgoblin::cell::UserData& aUserData) {
    return static_cast<std::int16_t>(aUserData.i64 & 0x7FF);
}

inline void UserData_SetParentAttachableId(jbatnozic::gridgoblin::cell::UserData& aUserData,
                                           std::int16_t                           aParentAttachableId) {
    aUserData.i64 &= ~0x7FF;
    aUserData.i64 |= (aParentAttachableId & 0x7FF);
}

} // namespace interior
} // namespace cinnabar

// MARK: Stream compatibility

namespace jbatnozic {
namespace gridgoblin {

inline hg::util::OutputStream& operator<<(hg::util::OutputStreamExtender& aOStream, ChunkId aChunkId) {
    aOStream->noThrow() << static_cast<std::uint16_t>(aChunkId.x)
                        << static_cast<std::uint16_t>(aChunkId.y);
    return *aOStream;
}

inline hg::util::InputStream& operator>>(hg::util::InputStreamExtender& aIStream, ChunkId& aChunkId) {
    auto& istream = *aIStream;
    aChunkId.x    = istream.extractNoThrow<std::uint16_t>();
    aChunkId.y    = istream.extractNoThrow<std::uint16_t>();
    return istream;
}

namespace cell {

inline hg::util::OutputStream& operator<<(hg::util::OutputStreamExtender& aOStream,
                                          CellKindId                      aCellKindId) {
    aOStream->noThrow() << static_cast<std::uint16_t>(aCellKindId.value);
    return *aOStream;
}

inline hg::util::InputStream& operator>>(hg::util::InputStreamExtender& aIStream,
                                         CellKindId&                    aCellKindId) {
    auto& istream     = *aIStream;
    aCellKindId.value = istream.extractNoThrow<std::uint16_t>();
    return istream;
}

inline hg::util::OutputStream& operator<<(hg::util::OutputStreamExtender& aOStream,
                                          FloorSprite                     aFloorSprite) {
    aOStream->noThrow() << static_cast<std::uint16_t>(aFloorSprite.id);
    return *aOStream;
}

inline hg::util::InputStream& operator>>(hg::util::InputStreamExtender& aIStream,
                                         FloorSprite&                   aFloorSprite) {
    auto& istream   = *aIStream;
    aFloorSprite.id = static_cast<SpriteId>(istream.extractNoThrow<std::uint16_t>());
    return istream;
}

inline hg::util::OutputStream& operator<<(hg::util::OutputStreamExtender& aOStream,
                                          WallSprite                      aWallSprite) {
    aOStream->noThrow() << static_cast<std::uint16_t>(aWallSprite.id)
                        << static_cast<std::uint16_t>(aWallSprite.id_reduced);
    return *aOStream;
}

inline hg::util::InputStream& operator>>(hg::util::InputStreamExtender& aIStream,
                                         WallSprite&                    aWallSprite) {
    auto& istream          = *aIStream;
    aWallSprite.id         = static_cast<SpriteId>(istream.extractNoThrow<std::uint16_t>());
    aWallSprite.id_reduced = static_cast<SpriteId>(istream.extractNoThrow<std::uint16_t>());
    return istream;
}

inline hg::util::OutputStream& operator<<(hg::util::OutputStreamExtender& aOStream,
                                          SpatialInfo                     aSpatialInfo) {
    aOStream->noThrow() << static_cast<std::uint8_t>(aSpatialInfo.wallShape)
                        << static_cast<std::uint8_t>(aSpatialInfo.obFlags)
                        << static_cast<std::uint8_t>(aSpatialInfo.openness);
    return *aOStream;
}

inline hg::util::InputStream& operator>>(hg::util::InputStreamExtender& aIStream,
                                         SpatialInfo&                   aSpatialInfo) {
    auto& istream          = *aIStream;
    aSpatialInfo.wallShape = static_cast<Shape>(istream.extractNoThrow<std::uint8_t>());
    aSpatialInfo.obFlags   = istream.extractNoThrow<std::uint8_t>();
    aSpatialInfo.openness  = istream.extractNoThrow<std::uint8_t>();
    return istream;
}

inline hg::util::OutputStream& operator<<(hg::util::OutputStreamExtender& aOStream, UserData aUserData) {
    aOStream->noThrow() << static_cast<std::uint64_t>(aUserData.u64);
    return *aOStream;
}

inline hg::util::InputStream& operator>>(hg::util::InputStreamExtender& aIStream, UserData& aUserData) {
    auto& istream = *aIStream;
    aUserData.u64 = istream.extractNoThrow<std::uint64_t>();
    return istream;
}

} // namespace cell
} // namespace gridgoblin
} // namespace jbatnozic

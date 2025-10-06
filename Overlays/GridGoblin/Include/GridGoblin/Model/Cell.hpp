// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Shape.hpp>
#include <GridGoblin/Model/Sprites.hpp>

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

//! The `cell` namespace contains struct definitions for all building blocks that make up cells
//! of a Chunk. A single logical cell has one `CellKindId`, one `FloorSprite`, one `WallSprite`,
//! and so on (though a World can be configured to not allocate certain building blocks if they
//! are not needed).
//!
//! \see BuildingBlock
namespace cell {

struct CellKindId {
    std::uint16_t value = 0;
};

struct FloorSprite {
    SpriteId id = SPRITEID_NONE;
};

struct WallSprite {
    SpriteId id         = SPRITEID_NONE;
    SpriteId id_reduced = SPRITEID_NONE;
};

enum ObstructedByFlags : std::uint8_t {
    OBSTRUCTED_NONE = 0,
    // Obstructed-by flags:
    //     bits 0..3 -> any
    //     bits 4..7 -> full
    OBSTRUCTED_BY_NORTH_NEIGHBOR       = (1 << 0),
    OBSTRUCTED_FULLY_BY_NORTH_NEIGHBOR = (1 << 4),
    OBSTRUCTED_BY_WEST_NEIGHBOR        = (1 << 1),
    OBSTRUCTED_FULLY_BY_WEST_NEIGHBOR  = (1 << 5),
    OBSTRUCTED_BY_EAST_NEIGHBOR        = (1 << 2),
    OBSTRUCTED_FULLY_BY_EAST_NEIGHBOR  = (1 << 6),
    OBSTRUCTED_BY_SOUTH_NEIGHBOR       = (1 << 3),
    OBSTRUCTED_FULLY_BY_SOUTH_NEIGHBOR = (1 << 7),
};

struct SpatialInfo {
    Shape        wallShape = Shape::EMPTY;
    std::uint8_t obFlags   = OBSTRUCTED_NONE;
    std::uint8_t openness  = 0;

    bool hasNonEmptyWallShape() const {
        return ((wallShape & Shape::BASE_SHAPE_MASK) != Shape::EMPTY);
    }
};

struct RendererAuxData {
    std::uint16_t mask  = 0;
    std::uint16_t mask2 = 0;
};

struct UserData {
    union {
        std::int64_t  i64;
        std::uint64_t u64;
        double        f64;
        void*         ptr;
    };
};

} // namespace cell

struct FatCell {
    cell::CellKindId      id;
    cell::FloorSprite     floorSprite;
    cell::WallSprite      wallSprite;
    cell::SpatialInfo     spatialInfo;
    cell::RendererAuxData rendererAuxData;
    cell::UserData        userData;
};

} // namespace gridgoblin
} // namespace jbatnozic

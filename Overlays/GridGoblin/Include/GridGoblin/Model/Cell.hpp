// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Shape.hpp>
#include <GridGoblin/Model/Sprites.hpp>

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

namespace cell {

// Refreshing algo :
// - Openness algo    : needs wall shape, sets openness
// - Obstruction algo : needs wall shape, sets obstructed-by flags
//
// Rendering algo :
// - Visibility calc algo : needs wall shape, obstructed-by flags, and openness
// - Drawing algo : needs sprite IDs, renderer mask, and wall shape + whether the wall is initialized or
// not

struct CellKindId {
    std::uint16_t value;
};

struct FloorSprite {
    SpriteId id;
};

struct WallSprite {
    SpriteId id;
    SpriteId id_reduced;
};

enum ObstructedByFlags : std::uint8_t {
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
    Shape        wallShape;
    std::uint8_t obFlags;
    std::uint8_t openness;

    bool hasNonEmptyWallShape() const {
        return ((wallShape & Shape::BASE_SHAPE_MASK) != Shape::EMPTY);
    }
};

struct RendererAuxData {
    std::uint16_t mask;
    std::uint16_t mask2;
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

#if 0
//! CellModel::Floor equality operator.
inline bool operator==(const CellModel::Floor& aLhs, const CellModel::Floor& aRhs) {
    return aLhs.spriteId == aRhs.spriteId;
}

//! CellModel::Floor inequality operator.
inline bool operator!=(const CellModel::Floor& aLhs, const CellModel::Floor& aRhs) {
    return !(aLhs == aRhs);
}

//! CellModel::Wall equality operator.
inline bool operator==(const CellModel::Wall& aLhs, const CellModel::Wall& aRhs) {
    return std::tie(aLhs.spriteId, aLhs.spriteId_reduced, aLhs.shape) ==
           std::tie(aRhs.spriteId, aRhs.spriteId_reduced, aRhs.shape);
}

//! CellModel::Wall inequality operator.
inline bool operator!=(const CellModel::Wall& aLhs, const CellModel::Wall& aRhs) {
    return !(aLhs == aRhs);
}

//! CellModel equality operator.
inline bool operator==(const CellModel& aLhs, const CellModel& aRhs) {
    if (aLhs.getFlags() != aRhs.getFlags()) {
        return false;
    }

    if (aLhs.isFloorInitialized() && (aLhs.getFloor() != aRhs.getFloor())) {
        return false;
    }

    if (aLhs.isWallInitialized() && (aLhs.getWall() != aRhs.getWall())) {
        return false;
    }

    return true;
}

//! CellModel inequality operator.
inline bool operator!=(const CellModel& aLhs, const CellModel& aRhs) {
    return !(aLhs == aRhs);
}
#endif

} // namespace gridgoblin
} // namespace jbatnozic

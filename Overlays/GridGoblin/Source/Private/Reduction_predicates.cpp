// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Private/Reduction_predicates.hpp>

#include <array>
#include <cassert>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

namespace predicate {
namespace {

constexpr double Half(double aVal) {
    return aVal * 0.5;
}

bool IsPovCloserThanCell(PositionInWorld aCellTopLeft, PositionInWorld aPointOfView) {
    return ((aCellTopLeft->x - aCellTopLeft->y) > (aPointOfView->x - aPointOfView->y));
}

RecommendedDrawMode FullWhenPovIsCloserToCamera(double          aCellResolution,
                                                PositionInWorld aCellTopLeft,
                                                PositionInWorld aPointOfView) {
    if (IsPovCloserThanCell(aCellTopLeft, aPointOfView)) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode X1(double          aCellResolution,
                       PositionInWorld aCellTopLeft,
                       PositionInWorld aPointOfView) {
    if (aPointOfView->y <= aCellTopLeft->y || aPointOfView->x >= aCellTopLeft->x + aCellResolution) {
        return RecommendedDrawMode::REDUCED;
    }
    return RecommendedDrawMode::FULL;
}

RecommendedDrawMode FullWhenPovIsSouthOrCloserToCamera(double          aCellResolution,
                                                       PositionInWorld aCellTopLeft,
                                                       PositionInWorld aPointOfView) {
    if ((aPointOfView->y >= aCellTopLeft->y + aCellResolution) ||
        IsPovCloserThanCell(aCellTopLeft, aPointOfView)) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsWestOrCloserToCamera(double          aCellResolution,
                                                      PositionInWorld aCellTopLeft,
                                                      PositionInWorld aPointOfView) {
    if ((aPointOfView->x <= aCellTopLeft->x) || IsPovCloserThanCell(aCellTopLeft, aPointOfView)) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsWestOrSouth(double          aCellResolution,
                                             PositionInWorld aCellTopLeft,
                                             PositionInWorld aPointOfView) {
    if ((aPointOfView->x <= aCellTopLeft->x) || (aPointOfView->y >= aCellTopLeft->y + aCellResolution)) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsWest(double          aCellResolution,
                                      PositionInWorld aCellTopLeft,
                                      PositionInWorld aPointOfView) {
    if (aPointOfView->x <= aCellTopLeft->x) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsWest2(double          aCellResolution,
                                       PositionInWorld aCellTopLeft,
                                       PositionInWorld aPointOfView) {
    if (aPointOfView->x <= aCellTopLeft->x + aCellResolution) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsSouth(double          aCellResolution,
                                       PositionInWorld aCellTopLeft,
                                       PositionInWorld aPointOfView) {
    if (aPointOfView->y >= aCellTopLeft->y + aCellResolution) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsSouth2(double          aCellResolution,
                                        PositionInWorld aCellTopLeft,
                                        PositionInWorld aPointOfView) {
    if (aPointOfView->y >= aCellTopLeft->y + aCellResolution) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsSouthAndCloserToCamera(double          aCellResolution,
                                                        PositionInWorld aCellTopLeft,
                                                        PositionInWorld aPointOfView) {
    if ((aPointOfView->y >= aCellTopLeft->y + aCellResolution) &&
        IsPovCloserThanCell(aCellTopLeft, aPointOfView)) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode FullWhenPovIsWestAndCloserToCamera(double          aCellResolution,
                                                       PositionInWorld aCellTopLeft,
                                                       PositionInWorld aPointOfView) {
    if ((aPointOfView->x <= aCellTopLeft->x) && IsPovCloserThanCell(aCellTopLeft, aPointOfView)) {
        return RecommendedDrawMode::FULL;
    }
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode AlwaysLowered(double          aCellResolution,
                                  PositionInWorld aCellTopLeft,
                                  PositionInWorld aPointOfView) {
    return RecommendedDrawMode::REDUCED;
}

RecommendedDrawMode AlwaysSkip(double          aCellResolution,
                               PositionInWorld aCellTopLeft,
                               PositionInWorld aPointOfView) {
    return RecommendedDrawMode::NOT_DRAWN;
}

#if 0
std::array<ReductionPredicate, 16> SELECTION_TABLE_OLD = {
    // b0000
    //   _
    // _ O _
    //   _
    //&FullWhenPovIsCloserToCamera,
    &X1,

    // b0001
    //   _
    // _ O X
    //   _
    &FullWhenPovIsSouthOrCloserToCamera,

    // b0010
    //   X
    // _ O _
    //   _
    &FullWhenPovIsWestOrCloserToCamera,

    // b0011
    //   X
    // _ O X
    //   _
    &FullWhenPovIsWestOrSouth,

    // b0100
    //   _
    // X O _
    //   _
    &FullWhenPovIsSouth,

    // b0101
    //   _
    // X O X
    //   _
    //&FullWhenPovIsSouth,
    &FullWhenPovIsSouth2,

    // b0110
    //   X
    // X O _
    //   _
    &FullWhenPovIsSouthAndCloserToCamera,

    // b0111
    //   X
    // X O X
    //   _
    &FullWhenPovIsSouth,

    // b1000
    //   _
    // _ O _
    //   X
    &FullWhenPovIsWestAndCloserToCamera,

    // b1001
    //   _
    // _ O X
    //   X
    &FullWhenPovIsWestAndCloserToCamera,

    // b1010
    //   X
    // _ O _
    //   X
    //&FullWhenPovIsWest,
    &FullWhenPovIsWest2,

    // b1011
    //   X
    // _ O X
    //   X
    &FullWhenPovIsWest,

    // b1100
    //   _
    // X O _
    //   X
    &AlwaysLowered,

    // b1101
    //   _
    // X O X
    //   X
    &AlwaysLowered,

    // b1110
    //   X
    // X O _
    //   X
    &AlwaysLowered,

    // b1111
    //   X
    // X O X
    //   X
    &AlwaysSkip,
};
#endif

std::array<ReductionPredicate, 16> SELECTION_TABLE = {
    // b0000
    //   _
    // _ O _
    //   _
    //&FullWhenPovIsCloserToCamera,
    &X1,

    // b0001
    //   X
    // _ O _
    //   _
    &FullWhenPovIsWestOrCloserToCamera,

    // b0010
    //   _
    // X O _
    //   _
    &FullWhenPovIsSouth,

    // b0011
    //   X
    // X O _
    //   _
    &FullWhenPovIsSouthAndCloserToCamera,

    // b0100
    //   _
    // _ O X
    //   _
    &FullWhenPovIsSouthOrCloserToCamera,

    // b0101
    //   X
    // _ O X
    //   _
    &FullWhenPovIsWestOrSouth,

    // b0110
    //   _
    // X O X
    //   _
    &FullWhenPovIsSouth2,

    // b0111
    //   X
    // X O X
    //   _
    &FullWhenPovIsSouth,

    // b1000
    //   _
    // _ O _
    //   X
    &FullWhenPovIsWestAndCloserToCamera,

    // b1001
    //   X
    // _ O _
    //   X
    &FullWhenPovIsWest2,

    // b1010
    //   _
    // X O _
    //   X
    &AlwaysLowered,

    // b1011
    //   X
    // X O _
    //   X
    &AlwaysLowered,

    // b1100
    //   _
    // _ O X
    //   X
    &FullWhenPovIsWestAndCloserToCamera,

    // b1101
    //   X
    // _ O X
    //   X
    &FullWhenPovIsWest,

    // b1110
    //   _
    // X O X
    //   X
    &AlwaysLowered,

    // b1111
    //   X
    // X O X
    //   X
    &AlwaysSkip,
};
} // namespace
} // namespace predicate

ReductionPredicate GetReductionPredicate(cell::SpatialInfo aCellSpatialInfo) {
    // clang-format off
    static constexpr auto FLAG_MASK =
        (cell::OBSTRUCTED_FULLY_BY_NORTH_NEIGHBOR |
         cell::OBSTRUCTED_FULLY_BY_WEST_NEIGHBOR  |
         cell::OBSTRUCTED_FULLY_BY_EAST_NEIGHBOR  |
         cell::OBSTRUCTED_FULLY_BY_SOUTH_NEIGHBOR );
    // clang-format on

    static constexpr int FLAG_SHIFT = 4;

    static_assert(0xF << FLAG_SHIFT == FLAG_MASK);

    const auto selector = static_cast<std::size_t>((aCellSpatialInfo.obFlags & FLAG_MASK) >> FLAG_SHIFT);

    assert(selector < predicate::SELECTION_TABLE.size());

    return predicate::SELECTION_TABLE[selector];
}

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>

#include <Hobgoblin/HGExcept.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

enum class RecommendedDrawMode {
    NOT_DRAWN,
    REDUCED,
    FULL,
};

using ReductionPredicate = RecommendedDrawMode (*)(double          aCellResolution,
                                                   PositionInWorld aCellTopLeft,
                                                   PositionInWorld aPointOfView);

ReductionPredicate GetReductionPredicate(cell::SpatialInfo aCellSpatialInfo);

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

// In this context, a 'predicate' is a function which tells us something about how to draw a cell,
// based (predicated) on where it is relative to the current point of view.
// This file contains functions which determine these predicates based on the cells' surroundings.

enum class RecommendedDrawMode {
    NOT_DRAWN,
    REDUCED,
    FULL,
};

using RecDrawModePredicate = RecommendedDrawMode (*)(double          aCellResolution,
                                                     PositionInWorld aCellTopLeft,
                                                     PositionInWorld aPointOfView);

RecDrawModePredicate GetRecDrawModePredicate(cell::SpatialInfo aCellSpatialInfo);

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Math/Vector2.hpp>
#include <Hobgoblin/Utility/Grids.hpp>

namespace cinnabar {

namespace hg = ::jbatnozic::hobgoblin;

//! Structure which tells us which cells of the origin ship controller's interior world are
//! covered by this attachable ghost.
struct ProjectedCellPositions {
    enum CellBits : std::int8_t {
        EMPTY            = 0x0,
        INSIDE_SHAPE     = 0x1,
        COLLIDES_WITH_IW = 0x2,
        OUT_OF_BOUNDS    = 0x4
    };

    hg::util::RowMajorGrid<std::int8_t> cells;

    //! A bitmask that is equal to the result of bitwise OR-ing all elements of `cells`.
    std::int8_t totalBitmask = 0;

    //! X/Y position of the cell in the ship controller's interior world to which `cell[0][0]`
    //! of this projection corresponds.
    hg::math::Vector2i topLeftPos;
};

} // namespace cinnabar

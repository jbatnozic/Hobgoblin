// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Utility/Grids.hpp>

#include <InteriorWorld/Cell_archs.hpp>

namespace cinnabar {
namespace interior {

struct CellProperties : CellArchProperties {
    jbatnozic::gridgoblin::cell::UserData userData;
};

using CellPropGrid = jbatnozic::hobgoblin::util::RowMajorGrid<CellProperties>;

} // namespace interior
} // namespace cinnabar

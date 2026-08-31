// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Utility/Grids.hpp>

#include <InteriorWorld/Cell_archs.hpp>

#include <cstdint>

namespace cinnabar {
namespace interior {

struct CellProperties : CellArchProperties {
    jbatnozic::gridgoblin::cell::UserData userData;
};

using CellPropGrid = jbatnozic::hobgoblin::util::RowMajorGrid<CellProperties>;

// IW Cell User Data Bit Allocation:
// (available bits = 0..63)
//  0..10 [11 bits] = parent attachable ID
// 10..63 [53 bits] = reserved

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

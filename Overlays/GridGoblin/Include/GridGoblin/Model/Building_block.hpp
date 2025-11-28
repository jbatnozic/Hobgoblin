// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common/Enum_op.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace jbatnozic {
namespace gridgoblin {

//! A GriGoblin Chunk consists of several so called 'building blocks': the first one is an array
//! of all `cell::CellKindId` structures for all of the Chunk's Cells, the second one is an array
//! of all `cell::FloorSprite` structures for all of the Chunk's Cells, and so on (it's uses a
//! "Structure-of-Arrays" layout).
//! This is an enumeration of all the possible building blocks.
enum class BuildingBlock : std::uint32_t {
    NONE = 0x00,

    CELL_KIND_ID      = 0x01, //!< [Optional]
    FLOOR_SPRITE      = 0x02,
    RESERVED_1        = 0x04, //!< Currently unused.
    WALL_SPRITE       = 0x08,
    RESERVED_2        = 0x10, //!< Currently unused.
    SPATIAL_INFO      = 0x20,
    RENDERER_AUX_DATA = 0x40, //!< [Optional]
    USER_DATA         = 0x80, //!< [Optional]

    SIGNIFICANT_BITS = 0xEB,
    ALL              = SIGNIFICANT_BITS,
};

using BuildingBlockMask = BuildingBlock;

[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(BuildingBlock, &);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(BuildingBlock, |);

//! Construct a string from a building block mask.
//! Result examples: "NONE", "SPATIAL_INFO", "FLOOR_SPRITE|USER_DATA".
std::string BuildingBlockMaskToString(BuildingBlockMask aBbMask);

//! Reconstruct a building block mask from its string representation.
//! \see BuildingBlockMaskToString
BuildingBlockMask StringToBuildingBlockMask(const std::string_view& aString);

} // namespace gridgoblin
} // namespace jbatnozic

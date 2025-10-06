// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common/Enum_op.hpp>

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

enum class BuildingBlock : std::uint32_t {
    NONE              = 0x00,
    CELL_KIND_ID      = 0x01,
    FLOOR_SPRITE      = 0x02,
    RESERVED_1        = 0x04,
    WALL_SPRITE       = 0x08,
    RESERVED_2        = 0x10,
    SPATIAL_INFO      = 0x20,
    RENDERER_AUX_DATA = 0x40,
    USER_DATA         = 0x80,

    ALL = 0xFFFFFFFF,
};

using BuildingBlockMask = BuildingBlock;

[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(BuildingBlock, &);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(BuildingBlock, |);

} // namespace gridgoblin
} // namespace jbatnozic

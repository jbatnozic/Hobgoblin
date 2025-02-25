// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

//! Identifies a sprite within the GridGoblin system.
//!
//! \warning despite this being an alias for a 32-bit type, GridGoblin will use only the lower
//!          16 bits, so don't use IDs over 65,535!
using SpriteId = std::uint16_t;

constexpr SpriteId SPRITEID_IDENTIFIER_MASK = 0x0FFF;

constexpr SpriteId SPRITEID_HFLIP_BIT = 0x4000; //!< Horizontal flip (through the Y axis)
constexpr SpriteId SPRITEID_VFLIP_BIT = 0x8000; //!< Vertical flip (through the X axis)

constexpr SpriteId SPRITEID_FLIP_MASK = 0xC000;

constexpr SpriteId SPRITEID_PROJECTION_DIMETRIC = 0x0000;
constexpr SpriteId SPRITEID_PROJECTION_TOPDOWN  = 0x1000;
constexpr SpriteId SPRITEID_PROJECTION_UNUSED_1 = 0x2000;
constexpr SpriteId SPRITEID_PROJECTION_UNUSED_2 = 0x3000;

constexpr SpriteId SPRITEID_PROJECTION_MASK = 0x3000;

} // namespace gridgoblin
} // namespace jbatnozic

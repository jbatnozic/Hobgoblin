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

//! Bits 0..11
constexpr SpriteId SPRITEID_IDENTIFIER_MASK = 0x0FFF;

//! Flip the sprite horizontally (through the Y axis) when this bit is set to 1
constexpr SpriteId SPRITEID_HFLIP = 0x4000;
//! Flip the sprite vertically (through the X axis) when this bit is set to 1
constexpr SpriteId SPRITEID_VFLIP = 0x8000;

//! Bits 14..15
constexpr SpriteId SPRITEID_FLIP_MASK = SPRITEID_HFLIP | SPRITEID_VFLIP;

constexpr SpriteId SPRITEID_PROJECTION_DIMETRIC = 0x0000;
constexpr SpriteId SPRITEID_PROJECTION_TOPDOWN  = 0x1000;
constexpr SpriteId SPRITEID_PROJECTION_UNUSED_1 = 0x2000;
constexpr SpriteId SPRITEID_PROJECTION_UNUSED_2 = 0x3000;

//! Bits 12..13
constexpr SpriteId SPRITEID_PROJECTION_MASK = 0x3000;

} // namespace gridgoblin
} // namespace jbatnozic

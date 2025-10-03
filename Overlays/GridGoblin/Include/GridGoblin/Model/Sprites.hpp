// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

//! Identifies a sprite within the GridGoblin system.
//!
//! Used to look up sprites held by a `uwga::SpriteLoader`.
//!
//! \warning while `SpriteId`s in `UWGA` are 32-bit values, `GridGoblin` uses 16-bit values in order
//!          to conserve memory and thus the top 16 bits will always implicitly be zeros.
using SpriteId = std::uint16_t;

//! Bits 0..11
constexpr SpriteId SPRITEID_IDENTIFIER_MASK = 0x0FFF;

// MARK: Flip

//! Flip the sprite horizontally (through the Y axis) when this bit is set to 1
constexpr SpriteId SPRITEID_HFLIP = 0x4000;
//! Flip the sprite vertically (through the X axis) when this bit is set to 1
constexpr SpriteId SPRITEID_VFLIP = 0x8000;

//! Bits 14..15
constexpr SpriteId SPRITEID_FLIP_MASK = SPRITEID_HFLIP | SPRITEID_VFLIP;

// MARK: Projection

constexpr SpriteId SPRITEID_PROJECTION_DIMETRIC = 0x0000;
constexpr SpriteId SPRITEID_PROJECTION_TOPDOWN  = 0x1000;
constexpr SpriteId SPRITEID_PROJECTION_UNUSED_1 = 0x2000;
constexpr SpriteId SPRITEID_PROJECTION_UNUSED_2 = 0x3000;

//! Bits 12..13
constexpr SpriteId SPRITEID_PROJECTION_MASK = 0x3000;

// MARK: None

//! A `SpriteId` where all bits are set to 1 is treated as "no sprite, don't render".
constexpr SpriteId SPRITEID_NONE = 0xFFFF;

} // namespace gridgoblin
} // namespace jbatnozic

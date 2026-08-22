// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <GridGoblin/Model/Cell.hpp>

#include <cstdint>

namespace cinnabar {
namespace interior {

using ::jbatnozic::gridgoblin::FatCell;
using ::jbatnozic::gridgoblin::Shape;
using ::jbatnozic::gridgoblin::SPRITEID_NONE;

//! Enumeration of all cell archetypes (kinds).
enum class CellArchE : std::uint16_t {
    SOLID_VOID,
    METALLIC_FLOOR,
    METALLIC_WALL,
};

constexpr std::uint16_t ToU16(CellArchE aCellKind) {
    return static_cast<std::uint16_t>(aCellKind);
}

//! A structure containing all the static properties of a cell archetype.
struct CellArchProperties {
    jbatnozic::gridgoblin::cell::CellKindId  cellKindId;
    jbatnozic::gridgoblin::cell::FloorSprite floorSprite;
    jbatnozic::gridgoblin::cell::WallSprite  wallSprite;
    jbatnozic::gridgoblin::cell::SpatialInfo spatialInfo;
};

namespace cell_archetype {

constexpr CellArchProperties SOLID_VOID = {
    .cellKindId  = {.value = ToU16(CellArchE::SOLID_VOID)},
    .floorSprite = {.id = SPRITEID_NONE},
    .wallSprite  = {.id = SPRITEID_NONE, .id_reduced = SPRITEID_NONE},
    .spatialInfo = {.wallShape = Shape::FULL_SQUARE},
};

constexpr CellArchProperties METALLIC_FLOOR = {
    .cellKindId  = {.value = ToU16(CellArchE::SOLID_VOID)},
    .floorSprite = {.id = SPRITEID_NONE},
    .wallSprite  = {.id = SPRITEID_NONE, .id_reduced = SPRITEID_NONE},
    .spatialInfo = {.wallShape = Shape::EMPTY},
};

} // namespace cell_archetype

} // namespace interior
} // namespace cinnabar

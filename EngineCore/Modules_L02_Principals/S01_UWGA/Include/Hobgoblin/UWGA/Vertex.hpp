// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_VERTEX_HPP
#define UHOBGOBLIN_UWGA_VERTEX_HPP

#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! A vertex defines a point with color and coordinates on a texture.
struct Vertex {
    //! Position of the vertex in the world. While these positions are relative to an anchor,
    //! vertices don't store the anchor themselves, but it is always provided externally.
    math::Vector2f position = {0.f, 0.f};

    //! Color of the vertex.
    Color color = COLOR_WHITE;

    //! Coordinates of the texture's pixel to map to the vertex. Same as with the position and
    //! its anchor, the reference to a texture (if any) is always provided externally.
    math::Vector2f texCoords = {0.f, 0.f};
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_VERTEX_HPP

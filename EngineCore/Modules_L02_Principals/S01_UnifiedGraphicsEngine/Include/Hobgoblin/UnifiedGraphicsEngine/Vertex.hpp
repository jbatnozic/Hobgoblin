// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_VERTEX_HPP
#define UHOBGOBLIN_UGE_VERTEX_HPP

#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Color.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

//! A vertex defines a point with color and coordinates on a texture.
struct Vertex {
    math::Vector2f position;
    Color          color;
    math::Vector2f texCoords;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_VERTEX_HPP

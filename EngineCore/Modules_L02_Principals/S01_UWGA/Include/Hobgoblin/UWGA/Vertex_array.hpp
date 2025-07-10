// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_VERTEX_ARRAY_HPP
#define UHOBGOBLIN_UWGA_VERTEX_ARRAY_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Primitive_type.hpp>
#include <Hobgoblin/UWGA/Vertex.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class VertexArray : public Drawable {
public:
    virtual ~VertexArray() = default;

    // MARK: Primitive type 

    virtual void setPrimitiveType(PrimitiveType aPrimitiveType) = 0;

    virtual PrimitiveType getPrimitiveType() const = 0;

    // MARK: Length

    virtual void setLength(PZInteger aLength) = 0;

    virtual PZInteger getLength() const = 0;

    // MARK: Vertices

    virtual void setVertex(PZInteger aIndex, const Vertex& aVertex) = 0;

    virtual Vertex getVertex(PZInteger aIndex) const = 0;

    // MARK: Misc.

    virtual void setAnchor(math::Vector2d aAnchor) = 0;

    virtual math::Vector2d getAnchor() const = 0;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_VERTEX_ARRAY_HPP

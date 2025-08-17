// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_VERTEX_ARRAY_HPP
#define UHOBGOBLIN_UWGA_VERTEX_ARRAY_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Primitive_type.hpp>
#include <Hobgoblin/UWGA/Vertex.hpp>

#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! Wrapper for a dynamic array (vector) of vertices joined with a primitive type.
//! Together they define a set of one or more 2D primitives.
//! No transform is supported, except setting the anchor.
class VertexArray : public Drawable {
public:
    //! Construct the VertexArray with default parameters.
    VertexArray() = default;

    //! Construct the VertexArray with the given primitive type, size and anchor.
    //! If `aSize` is greater than 0, then that many vertices will be default-constructed.
    VertexArray(PrimitiveType aPrimitiveType, PZInteger aSize = 0, math::Vector2d aAnchor = {0.0, 0.0});

    //! Construct the VertexArray with the given primitive type and anchor, and copy the vertices
    //! over from a collection of vertices or other objects that are implicitly convertible to vertices.
    //! (passing the begin and end iterators to the collection)
    template <class taVerticesBegin, class taVerticesEnd>
    VertexArray(PrimitiveType   aPrimitiveType,
                taVerticesBegin aVerticesBegin,
                taVerticesEnd   aVerticesEnd,
                math::Vector2d  aAnchor = {0.0, 0.0});

    //! Construct the VertexArray with the given primitive type and anchor, and copy the vertices
    //! over from a collection of vertices or other objects that are implicitly convertible to vertices.
    //! (passing the reference to the whole collection)
    template <class taVertices>
    VertexArray(PrimitiveType     aPrimitiveType,
                const taVertices& aVertices,
                math::Vector2d    aAnchor = {0.0, 0.0});

    //! Vector of individual vertices.
    std::vector<Vertex> vertices;

    //! The type of primitives to draw. This field defines how the
    //! vertices must be interpreted when it's time to draw them.
    PrimitiveType primitiveType = PrimitiveType::POINTS;

    //! The anchor position relative to which the vertices will be drawn.
    math::Vector2d anchor = {0.0, 0.0};

    //! \brief Compute the bounding rectangle of the vertex array
    //!
    //! This function returns the minimal axis-aligned rectangle
    //! that contains all the vertices of the array.
    //!
    //! \return Bounding rectangle of the vertex array
    math::Rectangle<float> calculateBounds() const;

    //! Draw this vertex array on a canvas.
    void drawOnto(Canvas&             aCanvas,
                  const RenderStates& aRenderStates = RENDER_STATES_DEFAULT) const override;
};

// MARK: Inline implementations

inline VertexArray::VertexArray(PrimitiveType aPrimitiveType, PZInteger aSize, math::Vector2d aAnchor)
    : vertices(pztos(aSize))
    , primitiveType(aPrimitiveType)
    , anchor(aAnchor) {}

template <class taVerticesBegin, class taVerticesEnd>
inline VertexArray::VertexArray(PrimitiveType   aPrimitiveType,
                                taVerticesBegin aVerticesBegin,
                                taVerticesEnd   aVerticesEnd,
                                math::Vector2d  aAnchor)
    : primitiveType(aPrimitiveType)
    , anchor(aAnchor) //
{
    vertices.reserve(std::distance(aVerticesBegin, aVerticesEnd));
    auto iter = aVerticesBegin;
    for (; iter != aVerticesEnd; ++iter) {
        vertices.push_back(*iter);
    }
}

template <class taVertices>
inline VertexArray::VertexArray(PrimitiveType     aPrimitiveType,
                                const taVertices& aVertices,
                                math::Vector2d    aAnchor)
    : VertexArray(aPrimitiveType, std::begin(aVertices), std::end(aVertices), aAnchor) {}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_VERTEX_ARRAY_HPP

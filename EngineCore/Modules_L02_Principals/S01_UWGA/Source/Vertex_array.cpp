// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <Hobgoblin/UWGA/Canvas.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

math::Rectangle<float> VertexArray::calculateBounds() const {
    if (!vertices.empty()) {
        float left   = vertices[0].position.x;
        float top    = vertices[0].position.y;
        float right  = vertices[0].position.x;
        float bottom = vertices[0].position.y;

        for (const auto& vertex : vertices) {
            const auto position = vertex.position;

            // Update left and right
            if (position.x < left) {
                left = position.x;
            } else if (position.x > right) {
                right = position.x;
            }

            // Update top and bottom
            if (position.y < top) {
                top = position.y;
            } else if (position.y > bottom) {
                bottom = position.y;
            }
        }

        return {left, top, right - left, bottom - top};
    }

    // Array is empty
    return {};
}

void VertexArray::drawOnto(Canvas& aCanvas, const RenderStates& aStates) const {
    aCanvas.draw(vertices.data(), stopz(vertices.size()), primitiveType, anchor, aStates);
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

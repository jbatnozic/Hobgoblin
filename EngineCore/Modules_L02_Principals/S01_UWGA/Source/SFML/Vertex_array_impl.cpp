// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Vertex_array_impl.hpp"

#include <Hobgoblin/UWGA/Canvas.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

void SFMLVertexArrayImpl::drawOnto(Canvas& aCanvas, const RenderStates& aRenderStates) const {
    assert(&getSystem() == &aCanvas.getSystem());

    // auto* sfRenderTarget = static_cast<sf::RenderTarget*>(aCanvas.getRenderingBackend());
    // assert(sfRenderTarget != nullptr);

    // sfRenderTarget->draw(_vertices.data(), _vertices.size(), _primitiveType);
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Rendering/Render_context.hpp>
#include <GridGoblin/Rendering/Renderer.hpp>

#include <Hobgoblin/HGExcept.hpp>

namespace jbatnozic {
namespace gridgoblin {

void Reset(RenderContext& aRenderContext) {
    HG_VALIDATE_ARGUMENT(aRenderContext.impls.renderer != nullptr);
    aRenderContext.ephemeral.renderedObjects.clear();
    aRenderContext.impls.renderer->reset(aRenderContext);
}

void AddRenderedObject(RenderContext& aRenderContext, const RenderedObject& aRenderedObject) {
    aRenderContext.ephemeral.renderedObjects.push_back(&aRenderedObject);
}

void AddRenderedObject(RenderContext&                              aRenderContext,
                       hobgoblin::NeverNull<const RenderedObject*> aRenderedObject) {
    HG_VALIDATE_ARGUMENT(aRenderContext.impls.renderer != nullptr);
    aRenderContext.ephemeral.renderedObjects.push_back(aRenderedObject);
}

void PrepareToRender(RenderContext& aRenderContext) {
    HG_VALIDATE_ARGUMENT(aRenderContext.impls.renderer != nullptr);
    aRenderContext.impls.renderer->prepareToRender(aRenderContext);
}

void Render(const RenderContext&          aRenderContext,
            hobgoblin::uwga::Canvas&      aCanvas,
            const hg::uwga::RenderStates& aRenderStates) {
    HG_VALIDATE_ARGUMENT(aRenderContext.impls.renderer != nullptr);
    aRenderContext.impls.renderer->render(aRenderContext, aCanvas, aRenderStates);
}

} // namespace gridgoblin
} // namespace jbatnozic

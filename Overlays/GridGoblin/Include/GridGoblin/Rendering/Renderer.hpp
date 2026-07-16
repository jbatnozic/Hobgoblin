// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Rendering/Render_context.hpp>

#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/View.hpp>

namespace jbatnozic {
namespace gridgoblin {

class Renderer {
public:
    virtual ~Renderer() = default;

    virtual void startPrepareToRender(const RenderContext& aRenderCtx) = 0;

    virtual void finishPrepareToRender(const RenderContext& aRenderCtx) = 0;

    virtual void render(
        const RenderContext&          aRenderCtx,
        hg::uwga::Canvas&             aCanvas,
        const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) const = 0;
};

} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Rendering/Rendered_object.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/Spatial/Position_in_view.hpp>
#include <GridGoblin/Spatial/Position_in_world.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/Graphics.hpp>

namespace jbatnozic {
namespace gridgoblin {

class Renderer {
public:
    virtual ~Renderer() = default;

    enum RenderFlags : std::int32_t {
        REDUCE_WALLS_BASED_ON_POSITION   = 0x01,
        REDUCE_WALLS_BASED_ON_VISIBILITY = 0x02,
    };

    struct OverdrawAmounts { // FTODO
        float top    = 0.f;
        float bottom = 0.f;
        float left   = 0.f;
        float right  = 0.f;
    };

    virtual void startPrepareToRender(const hg::gr::View&       aView,
                                      const OverdrawAmounts&    aOverdrawAmounts,
                                      PositionInWorld           aPointOfView,
                                      std::int32_t              aRenderFlags,
                                      const VisibilityProvider* aVisProv) = 0;

    virtual void addObject(const RenderedObject& aObject) = 0;

    virtual void endPrepareToRender() = 0;

    virtual void render(hg::gr::Canvas& aCanvas) = 0;
};

} // namespace gridgoblin
} // namespace jbatnozic

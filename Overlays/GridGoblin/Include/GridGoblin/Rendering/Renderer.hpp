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

    struct RenderParameters {
        //! Position in the world corresponding to the center of the view.
        PositionInWorld viewCenter;

        //! Width and height of the view.
        hg::math::Vector2d viewSize;

        //! Position in the world corresponding to point of view of the player (or their player
        //! character, or similar). This is used by certain renderer implementations to provide
        //! a clearer view for the player - for example, the dimetric renderer may lower or
        //! make transparent some walls so the player can see what's behind them.
        PositionInWorld pointOfView;

        //! Most graphics libraries, including the Hobgoblin Graphics library, internally use
        //! 32-bit floats, whose precision starts to be unsatisfactory for rendering with values
        //! around 100,000 and above. GridGoblin is made to support very large worlds, potentially
        //! millions of cells wide and tall, and the rendering quality could suffer a lot when the
        //! view is far away from the origin.
        //! Thse offsets can be used to shift what's being rendered rather than the view itself,
        //! thus bringing it into ranges of values that are acceptable for the graphics library.
        double xOffset = 0.0;
        double yOffset = 0.0;
    };

    enum RenderFlags : std::int32_t {
        REDUCE_WALLS_BASED_ON_POSITION   = 0x01,
        REDUCE_WALLS_BASED_ON_VISIBILITY = 0x02,
    };

    virtual void startPrepareToRender(const RenderParameters&   aRenderParams,
                                      std::int32_t              aRenderFlags,
                                      const VisibilityProvider* aVisProv) = 0;

    virtual void addObject(const RenderedObject& aObject) = 0;

    virtual void endPrepareToRender() = 0;

    virtual void render(hg::gr::Canvas& aCanvas) = 0;
};

} // namespace gridgoblin
} // namespace jbatnozic

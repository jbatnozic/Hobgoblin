// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Positional/Position_in_view.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Rendered_object.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/View.hpp>

namespace jbatnozic {
namespace gridgoblin {

class Renderer {
public:
    virtual ~Renderer() = default;

    enum RenderFlags : std::int32_t {
        REDUCE_WALLS_BASED_ON_POSITION   = 0x01,
        REDUCE_WALLS_BASED_ON_VISIBILITY = 0x02,
    };

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

        //! TODO
        std::int32_t flags = 0;
    };

    virtual void startPrepareToRender(const RenderParameters&   aRenderParams,
                                      const VisibilityProvider* aVisProv = nullptr) = 0;

    virtual void addObject(const RenderedObject& aObject) = 0;

    virtual void endPrepareToRender() = 0;

    virtual void render(
        hg::uwga::Canvas&             aCanvas,
        const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) = 0;
};

} // namespace gridgoblin
} // namespace jbatnozic

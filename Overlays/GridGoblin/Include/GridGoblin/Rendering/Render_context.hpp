// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Positional/Position_in_view.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Rendered_object.hpp>

#include <Hobgoblin/Common/Nullability.hpp>
#include <Hobgoblin/Common/Enum_op.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/View.hpp>

#include <cstdint>
#include <vector>

namespace jbatnozic {
namespace gridgoblin {

//! TODO
enum class RenderFlags : std::uint32_t {
    NONE                             = 0,
    REDUCE_WALLS_BASED_ON_POSITION   = (1 << 0),
    REDUCE_WALLS_BASED_ON_VISIBILITY = (1 << 1),
};

[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(RenderFlags, |);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(RenderFlags, &);

class Renderer;
class VisibilityProvider;
class LightingProvider;

//! TODO
struct RenderContext {
    //! Pointers to objects implementing the `Renderer` and various `-Provider` and other interfaces
    //! which abstract the actual rendering algorithms.
    //! No rendering function will ever change these values. - this is up to the user (though it is
    //! very much NOT recommended to change them mid-rendering cycle).
    struct Implementations {
        //! TODO
        Renderer* renderer = nullptr;

        //! TODO
        VisibilityProvider* visibilityProvider = nullptr;

        //! TODO
        LightingProvider* lightingProvider = nullptr;
    } impls;

    //! Values and objects which describe the dynamic state of the rendering context.
    //! No rendering function will ever change these values - this is up to the user, and it is
    //! expected that these values will change often, even between every two rendering cycles (though
    //! it is very much NOT recommended to change them mid-rendering cycle).
    struct Dynamic {
        //! Position in the world corresponding to the center of the view.
        PositionInWorld viewCenter = {};

        //! Width and height of the view.
        hg::math::Vector2d viewSize = {};

        //! Position in the world corresponding to point of view of the player (or their player
        //! character, or similar). This is used by certain renderer implementations to provide
        //! a clearer view for the player - for example, the dimetric renderer may lower or
        //! make transparent some walls so the player can see what's behind them.
        PositionInWorld pointOfView = {};

        //! TODO
        RenderFlags flags = RenderFlags::NONE;
    } dynamic;

    //! Values and objects which are only valid for one rendering cycle.
    //! A `Reset()` call reverts them to their default values.
    struct Ephemeral {
        //! TODO
        std::vector<RenderedObject*> renderedObjects;
    } ephemeral;
};

//! TODO
void Reset(RenderContext& aRenderContext);

//! TODO
void AddRenderedObject(RenderContext& aRenderContext, RenderedObject& aRenderedObject);

//! TODO
void AddRenderedObject(RenderContext&                        aRenderContext,
                       hobgoblin::NeverNull<RenderedObject*> aRenderedObject);

//! TODO
void FinishPrepareToRender(RenderContext& aRenderContext);

//! TODO
void Render(const RenderContext&          aRenderContext,
            hobgoblin::uwga::Canvas&      aCanvas,
            const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT);

} // namespace gridgoblin
} // namespace jbatnozic

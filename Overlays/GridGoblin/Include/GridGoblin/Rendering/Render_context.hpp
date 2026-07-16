// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Positional/Position_in_view.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>

#include <Hobgoblin/Common/Enum_op.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/View.hpp>

#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

enum class RenderFlags : std::uint32_t {
    NONE                             = 0,
    REDUCE_WALLS_BASED_ON_POSITION   = (1 << 0),
    REDUCE_WALLS_BASED_ON_VISIBILITY = (1 << 1),
};

[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(RenderFlags, |);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(RenderFlags, &);

class Renderer;
class VisibilityProvider;

struct RenderContext {
    //! TODO
    Renderer* renderer = nullptr;

    //! TODO
    VisibilityProvider* visibilityProvider = nullptr;

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
};

//! TODO
void StartPrepareToRender(const RenderContext& aRenderContext);

//! TODO
void AddRenderedObject(const RenderContext& aRenderContext);

//! TODO
void FinishPrepareToRender(const RenderContext& aRenderContext);

//! TODO
void Render(const RenderContext&          aRenderContext,
            hobgoblin::uwga::Canvas&      aCanvas,
            const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT);

} // namespace gridgoblin
} // namespace jbatnozic

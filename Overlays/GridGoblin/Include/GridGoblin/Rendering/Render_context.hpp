// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Positional/Position_in_view.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Rendered_object.hpp>

#include <Hobgoblin/Common/Enum_op.hpp>
#include <Hobgoblin/Common/Nullability.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>

#include <cstdint>
#include <vector>

namespace jbatnozic {
namespace gridgoblin {

// MARK: Datatypes

//! TODO(add description)
enum class RenderFlags : std::uint32_t {
    NONE                             = 0,
    REDUCE_WALLS_BASED_ON_POSITION   = (1 << 0),
    REDUCE_WALLS_BASED_ON_VISIBILITY = (1 << 1),
};

[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(RenderFlags, |);
[[nodiscard]] inline HG_ENUM_DEFINE_ARITHMETIC_OP(RenderFlags, &);

// Forward declarations
class LightingProvider;
class Renderer;
class VisibilityProvider;
class World;

//! TODO(add description)
struct RenderContext {
    //! The `World` instance to render.
    //! \warning you MUST set this to a valid, non-null value before using the context!
    World* world = nullptr;

    //! Pointers to objects implementing the `Renderer` and various `-Provider` and other interfaces
    //! which abstract the actual rendering algorithms.
    //! No rendering function will ever change these values. - this is up to the user (though it is
    //! very much NOT recommended to change them mid-rendering cycle).
    struct Implementations {
        //! TODO(add description)
        //! \warning you MUST set this to a valid, non-null value before using the context!
        Renderer* renderer = nullptr;

        //! TODO(add description)
        VisibilityProvider* visibilityProvider = nullptr;

        //! TODO(add description)
        LightingProvider* lightingProvider = nullptr;
    } impls;

    //! Objects and values which describe the mostly-static rendering configuration,
    //! meaning that you most likely won't change them once initially set (but you can,
    //! though it is not recommended to do so mid-rendering cycle).
    struct Config {
        //! TODO(add description)
        struct WallReductionConfig {
            static constexpr std::uint16_t MIN_VALUE = 0;    //!< Minimal reduction
            static constexpr std::uint16_t MAX_VALUE = 1023; //!< Maximal reduction

            std::uint16_t delta        = 15;
            std::uint16_t lowerBound   = 100; //! Below this value, the wall is at fully rendered
            std::uint16_t upperBound   = 900; //! Above this value, the wall is at fully reduced
            float         maxReduction = 1.f; //! Normalized to range [0.f, 1.f]

            double reductionDistanceLimit = 640.f;

            // TODO: boolean choice - fade or lower
        } wallReductionConfig;
    } config;

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

        //! TODO(add description)
        RenderFlags flags = RenderFlags::NONE;
    } dynamic;

    //! Values and objects which are only valid for one rendering cycle.
    //! A `Reset()` call reverts them to their default values.
    struct Ephemeral {
        //! TODO(add description)
        std::vector<const RenderedObject*> renderedObjects;
    } ephemeral;
};

// MARK: Render cycle functions

// GridGoblin rendering is intended to work as follows:
//   Preamble:
//      1.1 Instantiate a `RenderContext` object (recommended to reuse for as long as possible).
//      1.2 (assuming the World and all the Renderer etc. objects are already instantiated) Set the
//          `world` field and all the `impls` sub-fields. You may change these later, but you likely
//          won't. Also set all relevant `config` sub-fields.
//
//   Render cycle:
//     2.1 Set all `dynamic` sub-fields of the context.
//     2.2 Call `Reset()` on the context.
//     2.3 Set all `ephemeral` fields of the context, including adding rendered objects, which you can
//         do directly, or using the provided `AddRenderedObject()` convenience functions.
//     2.4 Call `PrepareToRender()` on the context.
//     2.5 Do the actual rendering by calling `Render()`. You can do this as many times as you like,
//         including on different canvases and using different render states, but once you want to
//         change anything about the scene, you must enter a new render cycle (by going back to
//         step 2.1).

//! Reset all `ephemeral` fields of a rendering context to their default values.
//! \note marks the beginning of a new rendering cycle.
void Reset(RenderContext& aRenderContext);

//! Add an object to be rendered to the context.
//! \warning the added object must be kept alive for all subsequent render cycle function calls,
//!          until `Reset()` is called on the context.
//! \note this is a convenience function; you can add objects directly using
//!       `renderContext.ephemeral.renderedObjects.push_back(...)` (or similar).
void AddRenderedObject(RenderContext& aRenderContext, const RenderedObject& aRenderedObject);

//! Add an object to be rendered to the context.
//! \warning the added object must be kept alive for all subsequent render cycle function calls,
//!          until `Reset()` is called on the context.
//! \note this is a convenience function; you can add objects directly using
//!       `renderContext.ephemeral.renderedObjects.push_back(...)` (or similar).
void AddRenderedObject(RenderContext&                              aRenderContext,
                       hobgoblin::NeverNull<const RenderedObject*> aRenderedObject);

//! TODO(add description)
void PrepareToRender(RenderContext& aRenderContext);

//! TODO(add description)
void Render(const RenderContext&          aRenderContext,
            hobgoblin::uwga::Canvas&      aCanvas,
            const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT);

} // namespace gridgoblin
} // namespace jbatnozic

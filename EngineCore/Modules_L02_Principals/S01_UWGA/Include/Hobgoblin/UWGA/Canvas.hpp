// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_CANVAS_HPP
#define UHOBGOBLIN_UWGA_CANVAS_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Primitive_type.hpp>
#include <Hobgoblin/UWGA/Render_states.hpp>
#include <Hobgoblin/UWGA/Vertex.hpp>
#include <Hobgoblin/UWGA/View.hpp>

#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! Abstracts away an object (such as a window or a texture) onto which 2D entities can be drawn.
//! It makes it possible to draw 2D entities like sprites, shapes, text without using any OpenGL
//! command directly.
//!
//! \see RenderWindow, RenderTexture
class Canvas : virtual public Element {
public:
    //! Virtual destructor.
    virtual ~Canvas() = default;

    //! \brief Return the size of the rendering region of the canvas.
    //!
    //! \return Size in pixels.
    virtual math::Vector2pz getSize() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: VIEWS                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! Change the current active view.
    //!
    //! The view is like a 2D camera, it controls which part of the 2D scene is visible,
    //! and how it is viewed in the canvas.
    //! The new view will affect everything that is drawn, until another view is set.
    //!
    //! \note The canvas keeps its own copy of the view object, so it is not necessary to keep
    //!       the original one alive after calling this function.
    //!
    //! To restore the original view of the canvas, you can call `setDefaultView()`.
    virtual void setView(const View& aView) = 0;

    //! Get a reference to the current active view.
    virtual const View& getView() const = 0;

    //! Change the current active view back to the default view of this canvas.
    virtual void setDefaultView() = 0;

    //! Create and return the default view of this canvas.
    virtual std::unique_ptr<View> createDefaultView() const = 0;

    //! \brief Get the viewport of a view, applied to this canvas.
    //!
    //! The viewport is defined in the view as a ratio, this function
    //! simply applies this ratio to the current dimensions of the
    //! canvas to calculate the pixels rectangle that the viewport
    //! actually covers in the target.
    //!
    //! \param view The view for which we want to compute the viewport.
    //!
    //! \return Viewport rectangle, expressed in pixels
    virtual math::Rectangle<int> viewportToPixels(const View& aView) const = 0;

    //! \brief Convert a point from canvas coordinates to world coordinates, using the given view.
    //!
    //! This function finds the 2D position that matches the given pixel of the canvas.
    //!
    //! Initially, both coordinate systems (world units and canvas pixels) match perfectly. But if
    //! you define a custom view or resize your canvas, this assertion is not true anymore, i.e. a
    //! point located at (10, 50) in your canvas may map to the point (150, 75) in your 2D world --
    //! if the view is translated by (140, 25).
    //!
    //! For `RenderWindow`s, this function is typically used to find which point (or object) is
    //! located below the mouse cursor.
    //!
    //! This version uses a custom view for calculations, see the other overload of the function if
    //! you want to use the current view of the canvas.
    //!
    //! \param aPixel Pixel to convert.
    //! \param aView The view to use for converting the point.
    //!
    //! \return The converted point, in "world" units.
    math::Vector2d mapPixelToCoords(math::Vector2f aPixel, const View& aView) const;

    //! \brief Convert a point from target coordinates to world coordinates, using the current view.
    //!
    //! \note This function is an overload of the `mapPixelToCoords` function that implicitly uses
    //!       the current view.
    math::Vector2d mapPixelToCoords(math::Vector2f aPixel) const;

    //! \brief Convert a point from world coordinates to canvas coordinates.
    //!
    //! This function finds the pixel of the canvas that matches the given 2D point.
    //!
    //! Initially, both coordinate systems (world units and canvas pixels) match perfectly. But if
    //! you define a custom view or resize your canvas, this assertion is not true anymore, i.e. a
    //! point located at (150, 75) in your 2D world may map to the pixel (10, 50) of your canvas --
    //! if the view is translated by (140, 25).
    //!
    //! This version uses a custom view for calculations, see the other overload of the function if
    //! you want to use the current view of the canvas.
    //!
    //! \param aCoords Point to convert.
    //! \param aView The view to use for converting the point.
    //!
    //! \return The converted point, in canvas coordinates (pixels).
    math::Vector2f mapCoordsToPixel(math::Vector2d aCoords, const View& aView) const;

    //! \brief Convert a point from world coordinates to canvas coordinates, using the current view.
    //!
    //! This function is an overload of the `mapCoordsToPixel` function that implicitly uses
    //! the current view.
    math::Vector2f mapCoordsToPixel(math::Vector2d aCoords) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: DRAWING                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Clear the canvas with a single color.
    //!
    //! This function is usually called once every frame,
    //! to clear the previous contents of the canvas.
    //!
    //! \param aColor Fill color to use to clear the canvas.
    virtual void clear(Color aColor = COLOR_BLACK) = 0;

    //! Draw an array of vertices onto the canvas.
    //! \note does nothing if `aVertices` is NULL or `aVertexCount` is 0.
    virtual void draw(const Vertex*       aVertices,
                      PZInteger           aVertexCount,
                      PrimitiveType       aPrimitiveType,
                      math::Vector2d      aAnchor,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;

    //! Helper function to draw any class implementing the `Drawable` interface onto the canvas,
    //! in the same way as you'd draw an array of vertices (provided for consitency).
    template <class taDrawable,
              T_ENABLE_IF(std::is_base_of_v<Drawable, std::remove_reference_t<taDrawable>>)>
    void draw(const taDrawable& aDrawable, const RenderStates& aStates = RENDER_STATES_DEFAULT) {
        aDrawable.drawOnto(SELF, aStates);
    }

#if UHOBGOBLIN_FUTURE
    virtual void draw(const VertexBuffer& aVertexBuffer,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;

    virtual void draw(const VertexBuffer& aVertexBuffer,
                      PZInteger           aFirstVertex,
                      PZInteger           aVertexCount,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;
#endif

    //! If the underlying object supports draw batching, various draw() calls may
    //! be deferred until later. Calling flush() will make sure that every draw()
    //! call thus far is resolved.
    virtual void flush() = 0;

    //! All fields count occurrences of events since the last time the counters were reset
    //! (see `resetPerformanceCounters` and `getAndResetPerformanceCounters`), or since
    //! the Canvas instance was created - which ever is the most recent.
    struct PerformanceCounters {
        //! Number of calls to any of the virtual `draw()` methods of the Canvas itself.
        PZInteger vcallCount = 0;

        //! Number of draw calls to the underlying graphics API.
        //! \note a good batching setup will aim to minimize this value.
        PZInteger ucallCount = 0;

        //! Max number of vcalls that were aggregated into a single ucall.
        //! \note a good batching setup will aim to maximize this value.
        PZInteger maxAggregation = 0;
    };

    //! Get the current state of the performance counters.
    //!
    //! \note to get the most accurrate results, the best time to call this
    //!       function is immediately after flushing the canvas.
    virtual const PerformanceCounters& getPerformanceCounters() const = 0;

    //! Reset all the performance counters back to their zero states (as if
    //! the Canvas was just created).
    virtual void resetPerformanceCounters() = 0;

    //! Return the values of performance counters and reset them internally.
    //!
    //! \note to get the most accurrate results, the best time to call this
    //!       function is immediately after flushing the canvas.
    virtual PerformanceCounters getAndResetPerformanceCounters() = 0;
};

///////////////////////////////////////////////////////////////////////////
// MARK: INLINE DEFINITIONS                                              //
///////////////////////////////////////////////////////////////////////////

inline math::Vector2d Canvas::mapPixelToCoords(math::Vector2f aPixel, const View& aView) const {
    const auto viewport = viewportToPixels(aView);
    const auto anchor   = aView.getAnchor();

    math::Vector2f normalized; // In range -1 .. +1
    normalized.x = -1.f + 2.f * (aPixel.x - (float)viewport.x) / (float)viewport.w;
    normalized.y = +1.f - 2.f * (aPixel.y - (float)viewport.y) / (float)viewport.h;

    return math::VectorCast<double>(aView.inverseTransformPoint(normalized)) + anchor;
}

inline math::Vector2d Canvas::mapPixelToCoords(math::Vector2f aPixel) const {
    return mapPixelToCoords(aPixel, getView());
}

inline math::Vector2f Canvas::mapCoordsToPixel(math::Vector2d aCoords, const View& aView) const {
    const auto fpoint     = math::VectorCast<float>(aCoords - aView.getAnchor());
    const auto normalized = aView.transformPoint(fpoint); // In range -1 .. +1

    const auto     viewport = viewportToPixels(aView);
    math::Vector2f pixel;
    pixel.x = (+normalized.x + 1.f) / 2.f * (float)viewport.w + (float)viewport.x;
    pixel.y = (-normalized.y + 1.f) / 2.f * (float)viewport.h + (float)viewport.y;

    return pixel;
}

inline math::Vector2f Canvas::mapCoordsToPixel(math::Vector2d aCoords) const {
    return mapCoordsToPixel(aCoords, getView());
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_CANVAS_HPP

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_CANVAS_HPP
#define UHOBGOBLIN_UWGA_CANVAS_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Primitive_type.hpp>
#include <Hobgoblin/UWGA/Render_states.hpp>
#include <Hobgoblin/UWGA/Vertex.hpp>
#include <Hobgoblin/UWGA/View.hpp>

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

    //! Change the current active view back to the default view of this canvas.
    virtual void setDefaultView() = 0;

    //! Get a reference to the current active view.
    virtual const View& getView() const = 0;

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

    virtual void draw(const Vertex*       aVertices,
                      PZInteger           aVertexCount,
                      PrimitiveType       aPrimitiveType,
                      math::Vector2d      aAnchor,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;

#if UHOBGOBLIN_FUTURE
    virtual void draw(const Drawable&     aDrawable,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;

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
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_CANVAS_HPP

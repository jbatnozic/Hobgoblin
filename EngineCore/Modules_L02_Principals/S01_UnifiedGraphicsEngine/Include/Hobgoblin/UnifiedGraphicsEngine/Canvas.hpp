// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_CANVAS_HPP
#define UHOBGOBLIN_UGE_CANVAS_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Element.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Color.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Drawable.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Primitive_type.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Render_states.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Vertex.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class Canvas : public Element {
public:
    //! Virtual destructor.
    virtual ~Canvas() = default;

    //! \brief Return the size of the rendering region of the canvas.
    //!
    //! \return Size in pixels.
    virtual math::Vector2pz getSize() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // DRAWING                                                               //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Clear the canvas with a single color.
    //!
    //! This function is usually called once every frame,
    //! to clear the previous contents of the canvas.
    //!
    //! \param aColor Fill color to use to clear the canvas.
    virtual void clear(const Color& aColor = COLOR_BLACK) = 0;

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

    ///////////////////////////////////////////////////////////////////////////
    // TODO                                                                  //
    ///////////////////////////////////////////////////////////////////////////

    virtual void* getRenderingBackend() = 0;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_CANVAS_HPP

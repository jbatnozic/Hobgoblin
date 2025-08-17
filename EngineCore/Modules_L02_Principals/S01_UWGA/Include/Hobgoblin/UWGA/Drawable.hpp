// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_DRAWABLE_HPP
#define UHOBGOBLIN_UWGA_DRAWABLE_HPP

#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Render_states.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Canvas;
class RenderStates;

class Drawable {
public:
    virtual ~Drawable() = default;

    //! TODO(add description)
    //!
    //! \note if the implementation of the Drawable cares which System the Canvas belongs to, it
    //!       should check it itself at the start of this function (recommended to use the standard
    //!       `assert` macro). The Canvas will NOT check it.
    virtual void drawOnto(Canvas&             aCanvas,
                          const RenderStates& aRenderStates = RENDER_STATES_DEFAULT) const = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_DRAWABLE_HPP

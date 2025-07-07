// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_DRAWABLE_HPP
#define UHOBGOBLIN_UGE_DRAWABLE_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/Element.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class Canvas;
class RenderStates;

class Drawable : public Element {
public:
    virtual ~Drawable() = default;

private:
    virtual void _drawOnto(Canvas& aCanvas, const RenderStates& aStates) const = 0;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_DRAWABLE_HPP

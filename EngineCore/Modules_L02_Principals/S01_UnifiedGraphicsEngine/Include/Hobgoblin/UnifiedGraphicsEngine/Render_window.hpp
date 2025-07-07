// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_RENDER_WINDOW_HPP
#define UHOBGOBLIN_UGE_RENDER_WINDOW_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/Canvas.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Window_event.hpp> // TODO: Inherit from Window instead

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class RenderWindow : public Canvas {
public:
    virtual bool pollEvent(WindowEvent& aEvent) = 0;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_RENDER_WINDOW_HPP


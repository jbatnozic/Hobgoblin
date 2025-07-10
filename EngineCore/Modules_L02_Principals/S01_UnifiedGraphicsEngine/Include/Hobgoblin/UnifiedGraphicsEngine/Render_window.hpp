// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_RENDER_WINDOW_HPP
#define UHOBGOBLIN_UGE_RENDER_WINDOW_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/Canvas.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Window.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

//! A Window that can also serve as a target for 2D drawing.
class RenderWindow : public Window, public Canvas {
public:
    virtual ~RenderWindow() = default;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_RENDER_WINDOW_HPP


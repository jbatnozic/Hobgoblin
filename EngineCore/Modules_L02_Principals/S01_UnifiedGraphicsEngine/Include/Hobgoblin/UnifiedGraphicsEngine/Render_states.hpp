// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_RENDER_STATES_HPP
#define UHOBGOBLIN_UGE_RENDER_STATES_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class RenderStates {
public:
    // const Texture* texture = nullptr;
    // const Shader*  shader = nullptr;
    // Transform      transform{};
    // BlendMode      blendMode = BLEND_ALPHA;
};

extern const RenderStates RENDER_STATES_DEFAULT;

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_RENDER_STATES_HPP


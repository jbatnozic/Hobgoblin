// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_RENDER_STATES_HPP
#define UHOBGOBLIN_UGE_RENDER_STATES_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/Shader.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Blend_mode.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class RenderStates {
public:
    // const Texture* texture = nullptr;
    const Shader*  shader = nullptr;
    // Transform      transform{};
    BlendMode      blendMode = BLEND_NORMAL;
};

class RenderStatesOptRef {
public:
    RenderStatesOptRef() = default;

    RenderStatesOptRef(const RenderStates& aRenderStates)
        : _renderStates{&aRenderStates} {}

    operator bool() const {
        return _renderStates != nullptr;
    }

    const RenderStates* ptr() const {
        return _renderStates;
    }

private:
    const RenderStates* _renderStates = nullptr;
};

inline const RenderStatesOptRef RENDER_STATES_DEFAULT = {};

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_RENDER_STATES_HPP

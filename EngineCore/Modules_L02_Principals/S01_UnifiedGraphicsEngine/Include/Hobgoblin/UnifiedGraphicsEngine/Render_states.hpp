// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_RENDER_STATES_HPP
#define UHOBGOBLIN_UGE_RENDER_STATES_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/Blend_mode.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Shader.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Transform.hpp>

#include <optional>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class Texture {
    // TODO: temporary
};

class RenderStates {
public:
    const Texture*           texture   = nullptr;
    const Shader*            shader    = nullptr;
    const Transform*         transform = nullptr;
    std::optional<BlendMode> blendMode = std::nullopt;
};

inline const RenderStates RENDER_STATES_DEFAULT = {};

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_RENDER_STATES_HPP

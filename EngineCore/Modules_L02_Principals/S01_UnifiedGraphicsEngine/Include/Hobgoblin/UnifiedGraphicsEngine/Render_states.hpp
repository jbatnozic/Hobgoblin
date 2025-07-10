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

//! Defines the states used to draw to a Canvas.
class RenderStates {
public:
    //! The texture to use: what image is mapped to the object.
    //! If nullptr, no texture will be used.
    //!
    //! \warning some high level objects, like text, sprites and multisprites will ignore
    //!          this value and use their own textures.
    const Texture* texture = nullptr;

    //! The shader to use: what custom effect is applied to the object.
    //! If nullptr, the default shader will be used.
    const Shader* shader = nullptr;

    //! The transform to use: how the object is positioned/rotated/scaled/etc.
    //! If nullptr, the identity transform will be used.
    //!
    //! \note the built-in drawables: text, sprites, multisprites and shapes will combine their
    //!       internal transforms with the one passed here. It is recommended for custom drawables
    //!       to do the same.
    const Transform* transform = nullptr;

    //! The blend mode to use: how pixels of the objects are blended with the background.
    //! If not set, normal (alpha) blending will be used.
    std::optional<BlendMode> blendMode = std::nullopt;
};

inline const RenderStates RENDER_STATES_DEFAULT = {};

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_RENDER_STATES_HPP

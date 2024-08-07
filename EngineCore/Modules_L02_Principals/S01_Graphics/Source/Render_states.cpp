// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off


#include <Hobgoblin/Graphics/Render_states.hpp>

#include <Hobgoblin/Graphics/Texture.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

const RenderStates RenderStates::DEFAULT{BlendMode{}};

RenderStates::RenderStates() = default;

RenderStates::RenderStates(const BlendMode& aBlendMode)
    : blendMode{aBlendMode}
{
}

RenderStates::RenderStates(const Transform& aTransform)
    : transform{aTransform}
{
}

RenderStates::RenderStates(const Texture* aTexture)
    : texture{aTexture}
{
}

RenderStates::RenderStates(const Shader* aShader)
    : shader{aShader}
{
}

RenderStates::RenderStates(
    const Texture* aTexture,
    const Shader* aShader,
    const Transform& aTransform,
    const BlendMode& aBlendMode)
    : texture{aTexture}
    , shader{aShader}
    , transform{aTransform}
    , blendMode{aBlendMode}
{
}

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// clang-format on

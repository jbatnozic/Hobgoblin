// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_DRAWING_ADAPTER_HPP
#define UHOBGOBLIN_UWGA_SFML_DRAWING_ADAPTER_HPP

#include <Hobgoblin/UWGA/Primitive_type.hpp>
#include <Hobgoblin/UWGA/Render_states.hpp>
#include <Hobgoblin/UWGA/System.hpp>
#include <Hobgoblin/UWGA/Vertex.hpp>

#include <Hobgoblin/Math/Vector.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include "SFML/Glsl_shader_impl.hpp"
#include "SFML/SFML_conversions.hpp"
#include "SFML/SFML_vertices.hpp"
#include "SFML/Transform_impl.hpp"

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLDrawingAdapter {
public:
    SFMLDrawingAdapter(const System&     aSystem,
                       sf::RenderTarget& aRenderTarget,
                       sf::RenderStates& aDefaultRenderStates,
                       math::Vector2d    aActiveViewAnchor)
        : _system{aSystem}
        , _renderTarget{aRenderTarget}
        , _defaultRenderStates{aDefaultRenderStates}
        , _activeViewAnchor{aActiveViewAnchor} {}

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aRenderStates) {
        if (aVertices == nullptr || aVertexCount == 0) {
            return;
        }

        SFMLVertices sfVertices{aVertices, pztos(aVertexCount)};

        assert(aRenderStates.transform == nullptr || &aRenderStates.transform->getSystem() == &_system);

        // Determine which render states object to use
        sf::RenderStates& sfmlRenderStates =
            (aRenderStates.transform) ? static_cast<const SFMLTransformImpl*>(aRenderStates.transform)
                                            ->getUnderlyingRenderStates()
                                      : _defaultRenderStates;

        // Set simple fields of render states
        sfmlRenderStates.texture   = _getSfmlTexture(aRenderStates.texture);
        sfmlRenderStates.shader    = _getSfmlShader(aRenderStates.shader);
        sfmlRenderStates.blendMode = _getSfmlBlendMode(aRenderStates.blendMode);

        sf::Transform& sfmlTransform   = sfmlRenderStates.transform;
        auto*          transformMatrix = const_cast<float*>(sfmlTransform.getMatrix());

        // Save original translation values
        float tm12 = transformMatrix[12];
        float tm13 = transformMatrix[13];

        // Adjust translation based on anchors
        transformMatrix[12] += static_cast<float>(aAnchor.x - _activeViewAnchor.x);
        transformMatrix[13] += static_cast<float>(aAnchor.y - _activeViewAnchor.y);

        // Draw
        _renderTarget.draw(sfVertices.getVertices(),
                           sfVertices.getVertexCount(),
                           ToSf(aPrimitiveType),
                           sfmlRenderStates);

        // Restore original translation values
        transformMatrix[12] = tm12;
        transformMatrix[13] = tm13;
    }

#if UHOBGOBLIN_FUTURE
    virtual void draw(const VertexBuffer& aVertexBuffer,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;

    virtual void draw(const VertexBuffer& aVertexBuffer,
                      PZInteger           aFirstVertex,
                      PZInteger           aVertexCount,
                      const RenderStates& aStates = RENDER_STATES_DEFAULT) = 0;
#endif

private:
    const System&     _system;
    sf::RenderTarget& _renderTarget;
    sf::RenderStates& _defaultRenderStates;
    math::Vector2d    _activeViewAnchor;

    const sf::Texture* _getSfmlTexture(const Texture* aTexture) const {
        assert(aTexture == nullptr || &aTexture->getSystem() == &_system);

        return ToSf(aTexture);
    }

    const sf::Shader* _getSfmlShader(const Shader* aShader) const {
        assert(aShader == nullptr || &aShader->getSystem() == &_system);

        if (aShader) {
            // We can do a shortcut without calling `aShader->getLanguage()` because this
            // is the only shader implementation supported by the SFML graphics system
            assert(typeid(*aShader) == typeid(SFMLGLSLShaderImpl));
            return &static_cast<const SFMLGLSLShaderImpl*>(aShader)->getUnderlyingShader();
        } else {
            return nullptr;
        }
    }

    const sf::BlendMode _getSfmlBlendMode(std::optional<BlendMode> aBlendMode) {
        if (aBlendMode) {
            return ToSf(*aBlendMode);
        } else {
            return sf::BlendAlpha;
        }
    }
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_DRAWING_ADAPTER_HPP

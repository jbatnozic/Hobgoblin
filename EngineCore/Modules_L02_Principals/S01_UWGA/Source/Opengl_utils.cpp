// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Opengl_utils.hpp>
#include <Hobgoblin/UWGA/System.hpp>

#include "SFML/Glsl_shader_impl.hpp"
#include "SFML/Render_texture_impl.hpp"
#include "SFML/Render_window_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {
namespace opengl {

void PushStates(Canvas& aCanvas) {
    switch (aCanvas.getSystem().getProvider()) {
    case System::Provider::SFML:
        if (auto* rt = dynamic_cast<SFMLRenderTextureImpl*>(&aCanvas); rt) {
            rt->pushGLStates();
        } else if (auto* rw = dynamic_cast<SFMLRenderWindowImpl*>(&aCanvas)) {
            rw->getUnderlyingRenderWindow().pushGLStates();
        } else {
            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "Unknown SFML Canvas implementation '{}'.",
                            typeid(aCanvas).name());
        }
        break;

    default:
        HG_THROW_TRACED(TracedLogicError,
                        0,
                        "Canvas is from a graphics system provider which doesn't support "
                        "opengl::PushStates - '{}' (graphics API = '{}').",
                        aCanvas.getSystem().getProviderName(),
                        aCanvas.getSystem().getGraphicsAPIName());
    }
}

void PopStates(Canvas& aCanvas) {
    switch (aCanvas.getSystem().getProvider()) {
    case System::Provider::SFML:
        if (auto* rt = dynamic_cast<SFMLRenderTextureImpl*>(&aCanvas); rt) {
            rt->popGLStates();
        } else if (auto* rw = dynamic_cast<SFMLRenderWindowImpl*>(&aCanvas)) {
            rw->getUnderlyingRenderWindow().popGLStates();
        } else {
            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "Unknown SFML Canvas implementation '{}'.",
                            typeid(aCanvas).name());
        }
        break;

    default:
        HG_THROW_TRACED(TracedLogicError,
                        0,
                        "Canvas is from a graphics system provider which doesn't support "
                        "opengl::PopStates - '{}' (graphics API = '{}').",
                        aCanvas.getSystem().getProviderName(),
                        aCanvas.getSystem().getGraphicsAPIName());
    }
}

void ResetStates(Canvas& aCanvas) {
    switch (aCanvas.getSystem().getProvider()) {
    case System::Provider::SFML:
        if (auto* rt = dynamic_cast<SFMLRenderTextureImpl*>(&aCanvas); rt) {
            rt->resetGLStates();
        } else if (auto* rw = dynamic_cast<SFMLRenderWindowImpl*>(&aCanvas)) {
            rw->getUnderlyingRenderWindow().resetGLStates();
        } else {
            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "Unknown SFML Canvas implementation '{}'.",
                            typeid(aCanvas).name());
        }
        break;

    default:
        HG_THROW_TRACED(TracedLogicError,
                        0,
                        "Canvas is from a graphics system provider which doesn't support "
                        "opengl::ResetStates - '{}' (graphics API = '{}').",
                        aCanvas.getSystem().getProviderName(),
                        aCanvas.getSystem().getGraphicsAPIName());
    }
}

bool SetActive(Canvas& aCanvas, bool aActive) {
    switch (aCanvas.getSystem().getProvider()) {
    case System::Provider::SFML:
        if (auto* rt = dynamic_cast<SFMLRenderTextureImpl*>(&aCanvas); rt) {
            return rt->setActive(aActive);
        } else if (auto* rw = dynamic_cast<SFMLRenderWindowImpl*>(&aCanvas)) {
            return rw->getUnderlyingRenderWindow().setActive(aActive);
        } else {
            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "Unknown SFML Canvas implementation '{}'.",
                            typeid(aCanvas).name());
        }
        break;

    default:
        HG_THROW_TRACED(TracedLogicError,
                        0,
                        "Canvas is from a graphics system provider which doesn't support "
                        "opengl::SetActive - '{}' (graphics API = '{}').",
                        aCanvas.getSystem().getProviderName(),
                        aCanvas.getSystem().getGraphicsAPIName());
    }
}

void Bind(const GLSLShader* aShader) {
    if (aShader == nullptr) {
        sf::Shader::bind(nullptr);
    }

    switch (aShader->getSystem().getProvider()) {
    case System::Provider::SFML:
        if (auto* sh = dynamic_cast<const SFMLGLSLShaderImpl*>(aShader); sh) {
            sf::Shader::bind(&(sh->getUnderlyingShader()));
        } else {
            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "Unknown SFML GLSLShader implementation '{}'.",
                            typeid(*aShader).name());
        }
        break;

    default:
        HG_THROW_TRACED(TracedLogicError,
                        0,
                        "Shader is from a graphics system provider which doesn't support "
                        "opengl::Bind - '{}' (graphics API = '{}').",
                        aShader->getSystem().getProviderName(),
                        aShader->getSystem().getGraphicsAPIName());
    }
}

} // namespace opengl
} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

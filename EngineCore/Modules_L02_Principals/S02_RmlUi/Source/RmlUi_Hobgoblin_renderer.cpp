// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/RmlUi/Private/RmlUi_Hobgoblin_renderer.hpp>
#include <Hobgoblin/UWGA/Opengl_utils.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>

#include <RmlUi/Core.h>

#include <GL/glew.h>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace rml {
namespace detail {

namespace {
constexpr auto LOG_ID = "Hobgoblin.RmlUi";

struct GeometryView {
    Rml::Span<const Rml::Vertex> vertices;
    Rml::Span<const int>         indices;
};
} // namespace

RmlUiHobgoblinRenderer::RmlUiHobgoblinRenderer(const uwga::System& aSystem)
    : _graphicsSystem{aSystem} {}

void RmlUiHobgoblinRenderer::setCanvas(uwga::Canvas* aCanvas) {
    _canvas = aCanvas;
}

uwga::Canvas* RmlUiHobgoblinRenderer::getCanvas() const {
    return _canvas;
}

///////////////////////////////////////////////////////////////////////////
// INHERITED FROM RML::RENDERINTERFACE                                   //
///////////////////////////////////////////////////////////////////////////

// MARK: Geometry

Rml::CompiledGeometryHandle RmlUiHobgoblinRenderer::CompileGeometry(
    Rml::Span<const Rml::Vertex> aVertices,
    Rml::Span<const int>         aIndices) {
    GeometryView* data = new GeometryView{aVertices, aIndices};
    return reinterpret_cast<Rml::CompiledGeometryHandle>(data);
}

void RmlUiHobgoblinRenderer::RenderGeometry(Rml::CompiledGeometryHandle aGeometry,
                                            Rml::Vector2f               aTranslation,
                                            Rml::TextureHandle          aTexture) {
    if (!_canvas) {
        return;
    }

    const GeometryView* geometry    = reinterpret_cast<GeometryView*>(aGeometry);
    const Rml::Vertex*  vertices    = geometry->vertices.data();
    const int*          indices     = geometry->indices.data();
    const int           num_indices = (int)geometry->indices.size();

    uwga::opengl::PushStates(*_canvas);
    _initViewport();

    glTranslatef(aTranslation.x, aTranslation.y, 0);

    glVertexPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].position);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rml::Vertex), &vertices[0].colour);

    if (!aTexture) {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    } else {
        glEnable(GL_TEXTURE_2D);

        auto uwgaTexture = reinterpret_cast<uwga::Texture*>(aTexture);
        uwga::opengl::Bind(uwgaTexture);

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].tex_coord);
    }

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

    uwga::opengl::PopStates(*_canvas);
}

void RmlUiHobgoblinRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle aGeometryHandle) {
    delete (reinterpret_cast<GeometryView*>(aGeometryHandle));
}

// MARK: Texture

// TODO: check correctness by comparing to
// https://github.com/mikke89/RmlUi/blob/2cd28864ae25ed345b70598751703a5433b12356/Backends/RmlUi_Renderer_GL2.cpp
// (specifically in regards to premultiplied alpha)
Rml::TextureHandle RmlUiHobgoblinRenderer::LoadTexture(Rml::Vector2i&     aTextureDimensions,
                                                       const Rml::String& aSource) {
    std::unique_ptr<uwga::Texture> uwgaTexture;

    try {
        uwgaTexture = _graphicsSystem.createTexture(aSource);
    } catch (const std::exception& ex) {
        HG_LOG_ERROR(LOG_ID, "Failed to load texture from memory; details: {}", ex.what());
        return 0;
    }

    const auto size    = uwgaTexture->getSize();
    aTextureDimensions = {size.x, size.y};

    return reinterpret_cast<Rml::TextureHandle>(uwgaTexture.release());
}

// TODO: check correctness by comparing to
// https://github.com/mikke89/RmlUi/blob/2cd28864ae25ed345b70598751703a5433b12356/Backends/RmlUi_Renderer_GL2.cpp
// (specifically in regards to premultiplied alpha)
Rml::TextureHandle RmlUiHobgoblinRenderer::GenerateTexture(Rml::Span<const Rml::byte> aSource,
                                                           Rml::Vector2i aSourceDimensions) {
    std::unique_ptr<uwga::Texture> uwgaTexture;

    try {
        uwgaTexture = _graphicsSystem.createTexture(aSourceDimensions.x, aSourceDimensions.y);
        uwgaTexture->update(aSource.data(), stopz(aSource.size()));
    } catch (const std::exception& ex) {
        HG_LOG_ERROR(LOG_ID, "Failed to load texture from memory; details: {}", ex.what());
        return 0;
    }

    return reinterpret_cast<Rml::TextureHandle>(uwgaTexture.release());
}

void RmlUiHobgoblinRenderer::ReleaseTexture(Rml::TextureHandle aTextureHandle) {
    delete (reinterpret_cast<uwga::Texture*>(aTextureHandle));
}

// MARK: ScissorRegion

void RmlUiHobgoblinRenderer::EnableScissorRegion(bool aEnable) {
    if (aEnable) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void RmlUiHobgoblinRenderer::SetScissorRegion(Rml::Rectanglei aRegion) {
    if (!_canvas) {
        return;
    }
    glScissor(aRegion.Left(),
              _canvas->getSize().y - aRegion.Bottom(),
              aRegion.Width(),
              aRegion.Height());
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE                                                               //
///////////////////////////////////////////////////////////////////////////

void RmlUiHobgoblinRenderer::_initViewport() {
    if (!_canvas) {
        return;
    }

    const auto size = _canvas->getSize();

    glViewport(0, 0, size.x, size.y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, size.x, size.y, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

} // namespace detail
} // namespace rml
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

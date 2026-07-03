// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_HOBGOBLIN_RENDERER_HPP
#define UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_HOBGOBLIN_RENDERER_HPP

#include <RmlUi/Core/RenderInterface.h>

#include <Hobgoblin/RmlUi/Context_driver.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/System.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>
HOBGOBLIN_NAMESPACE_BEGIN
namespace rml {
namespace detail {

class RmlUiHobgoblinRenderer : public Rml::RenderInterface {
public:
    RmlUiHobgoblinRenderer(const uwga::System& aSystem);

    //! Sets the canvas
    void setCanvas(uwga::Canvas* aCanvas);

    //! Returns the currently set canvas
    uwga::Canvas* getCanvas() const;

    ///////////////////////////////////////////////////////////////////////////
    // INHERITED FROM RML::RENDERINTERFACE                                   //
    ///////////////////////////////////////////////////////////////////////////

    // Geometry

    //! Called by RmlUi when it wants to compile geometry to be rendered later.
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> aVertices,
                                                Rml::Span<const int>         aIndices) override;

    //! Called by RmlUi when it wants to render geometry.
    void RenderGeometry(Rml::CompiledGeometryHandle aGeometryHandle,
                        Rml::Vector2f               aTranslation,
                        Rml::TextureHandle          aTextureHandle) override;

    //! Called by RmlUi when it wants to release geometry.
    void ReleaseGeometry(Rml::CompiledGeometryHandle aGeometryHandle) override;

    // Texture

    //! Called by RmlUi when a texture is required by the library.
    Rml::TextureHandle LoadTexture(Rml::Vector2i&     aTextureDimensions,
                                   const Rml::String& aSource) override;

    //! Called by RmlUi when a texture is required to be built from an
    //! internally-generated sequence of pixels.
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> aSource,
                                       Rml::Vector2i              aSourceDimensions) override;

    //! Called by RmlUi when a loaded texture is no longer required.
    void ReleaseTexture(Rml::TextureHandle aTextureHandle) override;

    // ScissorRegion

    //! Called by RmlUi when it wants to enable or disable scissoring to clip content.
    void EnableScissorRegion(bool aEnable) override;

    //! Called by RmlUi when it wants to change the scissor region.
    void SetScissorRegion(Rml::Rectanglei aRegion) override;

private:
    void _initViewport();

    const uwga::System& _graphicsSystem;
    uwga::Canvas*       _canvas = nullptr;
};

} // namespace detail
} // namespace rml
HOBGOBLIN_NAMESPACE_END
#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_HOBGOBLIN_RENDERER_HPP

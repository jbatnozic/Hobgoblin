// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP
#define UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP

#include <Hobgoblin/UWGA/System.hpp>

#include "Image_impl.hpp"
#include "Render_texture_impl.hpp"
#include "Render_window_impl.hpp"
#include "Texture_impl.hpp"
#include "Transform_impl.hpp"
#include "Vertex_array_impl.hpp"
#include "View_impl.hpp"

#include <SFML/Graphics/Texture.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLSystemImpl : public System {
public:
    ~SFMLSystemImpl() override = default;

    // MARK: RenderWindow

    std::unique_ptr<RenderWindow> createRenderWindow(PZInteger            aWidth,
                                                     PZInteger            aHeight,
                                                     WindowStyle          aStyle,
                                                     const UnicodeString& aTitle) const override {
        return std::make_unique<SFMLRenderWindowImpl>(SELF, aWidth, aHeight, aStyle, aTitle);
    }

    // MARK: Image

    std::unique_ptr<Image> createImage() const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset();
        return image;
    }

    std::unique_ptr<Image> createImage(PZInteger aWidth,
                                       PZInteger aHeight,
                                       Color     aColor) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aWidth, aHeight, aColor);
        return image;
    }

    std::unique_ptr<Image> createImage(math::Vector2pz aSize, Color aColor) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aSize, aColor);
        return image;
    }

    std::unique_ptr<Image> createImage(PZInteger                      aWidth,
                                       PZInteger                      aHeight,
                                       NeverNull<const std::uint8_t*> aPixels) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aWidth, aHeight, aPixels);
        return image;
    }

    std::unique_ptr<Image> createImage(math::Vector2pz                aSize,
                                       NeverNull<const std::uint8_t*> aPixels) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aSize, aPixels);
        return image;
    }

    std::unique_ptr<Image> createImage(const std::filesystem::path& aImagePath) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aImagePath);
        return image;
    }

    // MARK: Texture

    PZInteger getMaximumTextureSize() const override {
        return ToPz(sf::Texture::getMaximumSize());
    }

    std::unique_ptr<Texture> createTexture() const override {
        return std::make_unique<SFMLTextureImpl>(SELF);
    }

    std::unique_ptr<Texture> createTexture(PZInteger aWidth,
                                           PZInteger aHeight,
                                           bool      aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aWidth, aHeight, aEnableSRgb);
    }

    std::unique_ptr<Texture> createTexture(math::Vector2pz aSize, bool aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aSize.x, aSize.y, aEnableSRgb);
    }

    std::unique_ptr<Texture> createTexture(const std::filesystem::path& aImagePath,
                                           TextureRect                  aArea,
                                           bool                         aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aImagePath, aArea, aEnableSRgb);
    }

    std::unique_ptr<Texture> createTexture(const Image&      aImage,
                                           const TextureRect aArea,
                                           bool              aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aImage, aArea, aEnableSRgb);
    }

    // MARK: RenderTexture

    std::unique_ptr<RenderTexture> createRenderTexture() const override {
        return std::make_unique<SFMLRenderTextureImpl>(SELF);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(PZInteger aWidth,
                                                       PZInteger aHeight,
                                                       bool      aEnableSRgb) const override {
        return std::make_unique<SFMLRenderTextureImpl>(SELF, aWidth, aHeight, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(math::Vector2pz aSize,
                                                       bool            aEnableSRgb) const override {
        return std::make_unique<SFMLRenderTextureImpl>(SELF, aSize.x, aSize.y, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const std::filesystem::path& aImagePath,
                                                       TextureRect                  aArea,
                                                       bool aEnableSRgb) const override {
        return std::make_unique<SFMLRenderTextureImpl>(SELF, aImagePath, aArea, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const Image&      aImage,
                                                       const TextureRect aArea,
                                                       bool              aEnableSRgb) const override {
        return std::make_unique<SFMLRenderTextureImpl>(SELF, aImage, aArea, aEnableSRgb);
    }

    // MARK: View

    std::unique_ptr<View> createView() const override {
        return std::make_unique<SFMLViewImpl>(SELF);
    }

    std::unique_ptr<View> createDefaultView(const RenderWindow& aRenderWindow) const override {
        assert(&aRenderWindow.getSystem() == this);
        return std::make_unique<SFMLViewImpl>(
            SELF,
            static_cast<const SFMLRenderWindowImpl&>(aRenderWindow).getDefaultView(),
            math::Vector2d{0.0, 0.0});
    }

    // MARK: Transform

    std::unique_ptr<Transform> createTransform() const override {
        return std::make_unique<SFMLTransformImpl>(SELF);
    }

    // MARK: VertexArray

    std::unique_ptr<VertexArray> createVertexArray(PZInteger aSize) const override {
        // return std::make_unique
        return {};
    }
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TEXTURE_WRAPPER_HPP
#define UHOBGOBLIN_UWGA_SFML_TEXTURE_WRAPPER_HPP

#include "Texture_provider.hpp"

namespace sf {
class Texture;
} // namespace sf

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

//! This class is used to pass a sf::Texture as an uwga::Texture, or,
//! more precisely, as a SFMLTextureProvider.
class SFMLTextureWrapper : public SFMLTextureProvider {
public:
    SFMLTextureWrapper(const System& aSystem, const sf::Texture& aTexture)
        : _system{aSystem}
        , _texture{aTexture} {}

    ~SFMLTextureWrapper() override = default;

    sf::Texture& getUnderlyingTexture() override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    const sf::Texture& getUnderlyingTexture() const override {
        return _texture;
    }

    const System& getSystem() const override {
        return _system;
    }

    // MARK: Texture/Reset

    void reset(PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void reset(const std::filesystem::path& aPath, TextureRect aArea, bool aEnableSRgb) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void reset(const Image& aImage, TextureRect aArea, bool aEnableSRgb) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    // MARK: Texture/Updating

    void update(const std::uint8_t* aPixels, PZInteger aPixelsByteCount) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const std::uint8_t* aPixels,
                PZInteger           aWidth,
                PZInteger           aHeight,
                PZInteger           aX,
                PZInteger           aY) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const Texture& aTexture) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const Texture& aTexture, PZInteger aX, PZInteger aY) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const Image& aImage) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const Image& aImage, PZInteger aX, PZInteger aY) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const Window& aWindow) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void update(const Window& aWindow, PZInteger aX, PZInteger aY) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    // MARK: Texture/Miscellaneous

    math::Vector2pz getSize() const override {
        HG_NOT_IMPLEMENTED();
    }

    void setSmooth(bool aSmooth) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    bool isSmooth() const override {
        HG_NOT_IMPLEMENTED();
    }

    bool isSrgb() const override {
        HG_NOT_IMPLEMENTED();
    }

    void setRepeated(bool aRepeated) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    bool isRepeated() const override {
        HG_NOT_IMPLEMENTED();
    }

    [[nodiscard]] bool generateMipmap() override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    void swap(Texture& aOther) override {
        HG_UNREACHABLE("Operation forbidden.");
    }

    [[nodiscard]] std::unique_ptr<Image> copyToImage() const override {
        HG_NOT_IMPLEMENTED();
    }

    [[nodiscard]] std::optional<std::int64_t> getNativeHandle() const override {
        HG_NOT_IMPLEMENTED();
    }

    // MARK: Canvas/Views

    void setView(const View& aView) override {
        HG_UNREACHABLE("Operation not supported.");
    }

    const View& getView() const override {
        HG_UNREACHABLE("Operation not supported.");
    }

    void setDefaultView() override {
        HG_UNREACHABLE("Operation not supported.");
    }

    std::unique_ptr<View> createDefaultView() const override {
        HG_UNREACHABLE("Operation not supported.");
    }

    math::Rectangle<int> viewportToPixels(const View& aView) const override {
        HG_UNREACHABLE("Operation not supported.");
    }

    // MARK: Canvas/Drawing

    void clear(Color aColor) override {
        HG_UNREACHABLE("Operation not supported.");
    }

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates) override {
        HG_UNREACHABLE("Operation not supported.");
    }

    void flush() override {
        HG_UNREACHABLE("Operation not supported.");
    }

    void display() override {
        HG_UNREACHABLE("Operation not supported.");
    }

private:
    const System&      _system;
    const sf::Texture& _texture;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TEXTURE_WRAPPER_HPP

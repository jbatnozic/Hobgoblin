// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TEXTURE_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_TEXTURE_IMPL_HPP

#include <Hobgoblin/UWGA/Texture.hpp>

#include <SFML/Graphics/Texture.hpp>

#include "SFML/SFML_conversions.hpp"
#include "SFML/SFML_err.hpp"
#include "SFML/Texture_common_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLTextureImpl : public SFMLTextureCommonImpl<sf::Texture> {
public:
    SFMLTextureImpl(const System& aSystem, PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb)
        : SFMLTextureCommonImpl<sf::Texture>{aSystem} //
    {
        reset(aWidth, aHeight, aEnableSRgb);
    }

    SFMLTextureImpl(const System& aSystem, math::Vector2pz aSize, bool aEnableSRgb)
        : SFMLTextureCommonImpl<sf::Texture>{aSystem} //
    {
        reset(aSize.x, aSize.y, aEnableSRgb);
    }

    SFMLTextureImpl(const System&                aSystem,
                    const std::filesystem::path& aImagePath,
                    TextureRect                  aArea,
                    bool                         aEnableSRgb)
        : SFMLTextureCommonImpl<sf::Texture>{aSystem} //
    {
        reset(aImagePath, aArea, aEnableSRgb);
    }

    SFMLTextureImpl(const System&     aSystem,
                    const Image&      aImage,
                    const TextureRect aArea,
                    bool              aEnableSRgb)
        : SFMLTextureCommonImpl<sf::Texture>{aSystem} //
    {
        reset(aImage, aArea, aEnableSRgb);
    }

    void display() override {
        HG_UNREACHABLE("Illegal operation!");
    }

    // MARK: Reset

    void reset(PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb) override {
        _texture.emplace();
        _texture->setSrgb(aEnableSRgb);

        SFMLErrorCatcher sfErr;
        if (!_texture->create(static_cast<unsigned>(aWidth), static_cast<unsigned>(aHeight))) {
            HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
        }
    }

    void reset(const std::filesystem::path& aPath, TextureRect aArea, bool aEnableSRgb) override {
        _texture.emplace();
        _texture->setSrgb(aEnableSRgb);

        SFMLErrorCatcher sfErr;
        if (!_texture->loadFromFile(FilesystemPathToSfPath(aPath), ConvertTextureRect(aArea))) {
            HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
        }
    }

    void reset(const Image& aImage, TextureRect aArea, bool aEnableSRgb) override {
        assert(&aImage.getSystem() == &_system);

        const auto& imageImpl = static_cast<const SFMLImageImpl&>(aImage);

        _texture.emplace();
        _texture->setSrgb(aEnableSRgb);

        SFMLErrorCatcher sfErr;
        if (!_texture->loadFromImage(imageImpl.getUnderlyingImage(), ConvertTextureRect(aArea))) {
            HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
        }
    }

    // MARK: Updating

    void update(const std::uint8_t* aPixels, PZInteger aPixelsByteCount) override {
        if (!aPixels) {
            return;
        }

        const auto size = getSize();
        HG_VALIDATE_ARGUMENT(aPixelsByteCount == size.x * size.y * 4);

        _texture->update(aPixels);
    }

    void update(const std::uint8_t* aPixels,
                PZInteger           aWidth,
                PZInteger           aHeight,
                PZInteger           aX,
                PZInteger           aY) override {
        if (!aPixels) {
            return;
        }

        _texture->update(aPixels,
                         static_cast<unsigned>(aWidth),
                         static_cast<unsigned>(aHeight),
                         static_cast<unsigned>(aX),
                         static_cast<unsigned>(aY));
    }

    void update(const Texture& aTexture) override {
        update(aTexture, 0, 0);
    }

    void update(const Texture& aTexture, PZInteger aX, PZInteger aY) override {
        assert(&aTexture.getSystem() == &_system);

        const auto& textureProvider = static_cast<const SFMLTextureProvider&>(aTexture);
        _texture->update(textureProvider.getUnderlyingTexture(),
                         static_cast<unsigned>(aX),
                         static_cast<unsigned>(aY));
    }

    void update(const Image& aImage) override {
        update(aImage, 0, 0);
    }

    void update(const Image& aImage, PZInteger aX, PZInteger aY) override {
        assert(&aImage.getSystem() == &_system);

        const auto& imageImpl = static_cast<const SFMLImageImpl&>(aImage);
        _texture->update(imageImpl.getUnderlyingImage(),
                         static_cast<unsigned>(aX),
                         static_cast<unsigned>(aY));
    }

    void update(const Window& aWindow) override {
        update(aWindow, 0, 0);
    }

    void update(const Window& aWindow, PZInteger aX, PZInteger aY) override {
        assert(&aWindow.getSystem() == &_system);

        if (typeid(aWindow) == typeid(SFMLRenderWindowImpl)) {
            const auto& windowImpl = static_cast<const SFMLRenderWindowImpl&>(aWindow);
            _texture->update(windowImpl.getUnderlyingRenderWindow(),
                             static_cast<unsigned>(aX),
                             static_cast<unsigned>(aY));
        }

        HG_UNREACHABLE("Incompatible Window implementation provided.");
    }

    // MARK: Miscellaneous

    [[nodiscard]] std::unique_ptr<Image> copyToImage() const override {
        return std::make_unique<SFMLImageImpl>(_system, *_texture);
    }

    [[nodiscard]] std::optional<std::int64_t> getNativeHandle() const override {
        const auto openglHandle = _texture->getNativeHandle();
        if (openglHandle == 0) {
            return std::nullopt;
        }
        return static_cast<std::int64_t>(openglHandle);
    }
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TEXTURE_IMPL_HPP

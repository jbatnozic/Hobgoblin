// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TEXTURE_COMMON_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_TEXTURE_COMMON_IMPL_HPP

#include <Hobgoblin/UWGA/Texture.hpp>

#include <SFML/Graphics/Texture.hpp>

#include "Image_impl.hpp"
#include "Render_window_impl.hpp"
#include "Texture_provider.hpp"

#include "SFML_conversions.hpp"
#include "SFML_err.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

/*
 * Class hierarchy:
 *
 *    Texture        Canvas
 *       \             /
 *        \           /
 *         \         /
 *        RenderTexture
 *              |
 *              |
 *      SFMLTextureProvider
 *              |
 *              |
 *     SFMLTextureCommonImpl<>
 *        /            \
 *       /              \
 *      /                \
 * TextureImpl      RenderTextureImpl
 */
template <class taUnderlying>
class SFMLTextureCommonImpl : public SFMLTextureProvider {
public:
    SFMLTextureCommonImpl(const System& aSystem);

    SFMLTextureCommonImpl(const System& aSystem, PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb);

    SFMLTextureCommonImpl(const System& aSystem, math::Vector2pz aSize, bool aEnableSRgb);

    SFMLTextureCommonImpl(const System&                aSystem,
                          const std::filesystem::path& aImagePath,
                          TextureRect                  aArea,
                          bool                         aEnableSRgb);

    SFMLTextureCommonImpl(const System&     aSystem,
                          const Image&      aImage,
                          const TextureRect aArea,
                          bool              aEnableSRgb);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SFMLTextureProvider                                             //
    ///////////////////////////////////////////////////////////////////////////

    sf::Texture& getUnderlyingTexture() override;

    const sf::Texture& getUnderlyingTexture() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Texture                                                         //
    ///////////////////////////////////////////////////////////////////////////

    // Reset

    void reset(PZInteger aWidth, PZInteger aHeight) override;

    void reset(const std::filesystem::path& aPath, TextureRect aArea) override;

    void reset(const Image& aImage, TextureRect aArea) override;

    // Updating

    void update(const std::uint8_t* aPixels, PZInteger aPixelsByteCount) override;

    void update(const std::uint8_t* aPixels,
                PZInteger           aWidth,
                PZInteger           aHeight,
                PZInteger           aX,
                PZInteger           aY) override;

    void update(const Texture& aTexture) override;

    void update(const Texture& aTexture, PZInteger aX, PZInteger aY) override;

    void update(const Image& aImage) override;

    void update(const Image& aImage, PZInteger aX, PZInteger aY) override;

    void update(const Window& aWindow) override;

    void update(const Window& aWindow, PZInteger aX, PZInteger aY) override;

    // Miscellaneous

    math::Vector2pz getSize() const override;

    void setSmooth(bool aSmooth) override;

    bool isSmooth() const override;

    void setSrgb(bool aSRgb) override;

    bool isSrgb() const override;

    void setRepeated(bool aRepeated) override;

    bool isRepeated() const override;

    [[nodiscard]] bool generateMipmap() override;

    void swap(Texture& aOther) override;

    [[nodiscard]] std::unique_ptr<Image> copyToImage() const override;

    [[nodiscard]] std::optional<std::int64_t> getNativeHandle() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Canvas                                                          //
    ///////////////////////////////////////////////////////////////////////////

    // math::Vector2pz getSize() const override; -- see section 'Window'

    // Viwes

    void setView(const View& aView) override;

    void setDefaultView() override;

    const View& getView() const override;

    const sf::View& getDefaultView() const;

    // Drawing

    void clear(Color aColor) override;

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates) override;

    void flush() override;

private:
    const System& _system;
    sf::Texture   _texture;
};

///////////////////////////////////////////////////////////////////////////
// MARK: Method definitions                                              //
///////////////////////////////////////////////////////////////////////////

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::SFMLTextureCommonImpl(const System& aSystem)
    : _system{aSystem} {}

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::SFMLTextureCommonImpl(const System& aSystem,
                                                           PZInteger     aWidth,
                                                           PZInteger     aHeight,
                                                           bool          aEnableSRgb)
    : _system{aSystem} //
{
    reset(aWidth, aHeight);
    _texture.setSrgb(aEnableSRgb);
}

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::SFMLTextureCommonImpl(const System&   aSystem,
                                                           math::Vector2pz aSize,
                                                           bool            aEnableSRgb)
    : _system{aSystem} //
{
    reset(aSize.x, aSize.y);
    _texture.setSrgb(aEnableSRgb);
}

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::SFMLTextureCommonImpl(const System&                aSystem,
                                                           const std::filesystem::path& aImagePath,
                                                           TextureRect                  aArea,
                                                           bool                         aEnableSRgb)
    : _system{aSystem} //
{
    reset(aImagePath, aArea);
    _texture.setSrgb(aEnableSRgb);
}

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::SFMLTextureCommonImpl(const System&     aSystem,
                                                           const Image&      aImage,
                                                           const TextureRect aArea,
                                                           bool              aEnableSRgb)
    : _system{aSystem} //
{
    reset(aImage, aArea);
    _texture.setSrgb(aEnableSRgb);
}

template <class taUnderlying>
sf::Texture& SFMLTextureCommonImpl<taUnderlying>::getUnderlyingTexture() {
    return _texture;
}

template <class taUnderlying>
const sf::Texture& SFMLTextureCommonImpl<taUnderlying>::getUnderlyingTexture() const {
    return _texture;
}

// ===== Element =====

template <class taUnderlying>
const System& SFMLTextureCommonImpl<taUnderlying>::getSystem() const {
    return _system;
}

// ===== Texture =====

// Reset

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::reset(PZInteger aWidth, PZInteger aHeight) {
    SFMLErrorCatcher sfErr;
    if (!_texture.create(static_cast<unsigned>(aWidth), static_cast<unsigned>(aHeight))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::reset(const std::filesystem::path& aPath, TextureRect aArea) {
    SFMLErrorCatcher sfErr;
    if (!_texture.loadFromFile(FilesystemPathToSfPath(aPath), ConvertTextureRect(aArea))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::reset(const Image& aImage, TextureRect aArea) {
    assert(&aImage.getSystem() == &_system);

    const auto& imageImpl = static_cast<const SFMLImageImpl&>(aImage);

    SFMLErrorCatcher sfErr;
    if (!_texture.loadFromImage(imageImpl.getUnderlyingImage(), ConvertTextureRect(aArea))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

// Updating

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const std::uint8_t* aPixels,
                                                 PZInteger           aPixelsByteCount) {
    if (!aPixels) {
        return;
    }

    const auto size = getSize();
    HG_VALIDATE_ARGUMENT(aPixelsByteCount == size.x * size.y * 4);

    _texture.update(aPixels);
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const std::uint8_t* aPixels,
                                                 PZInteger           aWidth,
                                                 PZInteger           aHeight,
                                                 PZInteger           aX,
                                                 PZInteger           aY) {
    if (!aPixels) {
        return;
    }

    _texture.update(aPixels,
                    static_cast<unsigned>(aWidth),
                    static_cast<unsigned>(aHeight),
                    static_cast<unsigned>(aX),
                    static_cast<unsigned>(aY));
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const Texture& aTexture) {
    update(aTexture, 0, 0);
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const Texture& aTexture, PZInteger aX, PZInteger aY) {
    assert(&aTexture.getSystem() == &_system);

    const auto& textureProvider = static_cast<const SFMLTextureProvider&>(aTexture);
    _texture.update(textureProvider.getUnderlyingTexture(),
                    static_cast<unsigned>(aX),
                    static_cast<unsigned>(aY));
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const Image& aImage) {
    update(aImage, 0, 0);
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const Image& aImage, PZInteger aX, PZInteger aY) {
    assert(&aImage.getSystem() == &_system);

    const auto& imageImpl = static_cast<const SFMLImageImpl&>(aImage);
    _texture.update(imageImpl.getUnderlyingImage(),
                    static_cast<unsigned>(aX),
                    static_cast<unsigned>(aY));
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const Window& aWindow) {
    update(aWindow, 0, 0);
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::update(const Window& aWindow, PZInteger aX, PZInteger aY) {
    assert(&aWindow.getSystem() == &_system);

    if (typeid(aWindow) == typeid(SFMLRenderWindowImpl)) {
        const auto& windowImpl = static_cast<const SFMLRenderWindowImpl&>(aWindow);
        _texture.update(windowImpl.getUnderlyingRenderWindow(),
                        static_cast<unsigned>(aX),
                        static_cast<unsigned>(aY));
    }

    HG_UNREACHABLE("Incompatible Window implementation provided.");
}

// Miscellaneous

template <class taUnderlying>
math::Vector2pz SFMLTextureCommonImpl<taUnderlying>::getSize() const {
    const auto size = _texture.getSize();
    return {ToPz(size.x), ToPz(size.y)};
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setSmooth(bool aSmooth) {
    _texture.setSmooth(aSmooth);
}

template <class taUnderlying>
bool SFMLTextureCommonImpl<taUnderlying>::isSmooth() const {
    return _texture.isSmooth();
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setSrgb(bool aSRgb) {
    _texture.setSrgb(aSRgb);
}

template <class taUnderlying>
bool SFMLTextureCommonImpl<taUnderlying>::isSrgb() const {
    return _texture.isSrgb();
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setRepeated(bool aRepeated) {
    _texture.setRepeated(aRepeated);
}

template <class taUnderlying>
bool SFMLTextureCommonImpl<taUnderlying>::isRepeated() const {
    return _texture.isRepeated();
}

template <class taUnderlying>
[[nodiscard]] bool SFMLTextureCommonImpl<taUnderlying>::generateMipmap() {
    return _texture.generateMipmap();
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::swap(Texture& aOther) {
    assert(&aOther.getSystem() == &_system);

    auto& textureProvider = static_cast<SFMLTextureProvider&>(aOther);

    _texture.swap(textureProvider.getUnderlyingTexture());
}

template <class taUnderlying>
[[nodiscard]] std::unique_ptr<Image> SFMLTextureCommonImpl<taUnderlying>::copyToImage() const {
    return std::make_unique<SFMLImageImpl>(_system, _texture);
}

template <class taUnderlying>
[[nodiscard]] std::optional<std::int64_t> SFMLTextureCommonImpl<taUnderlying>::getNativeHandle() const {
    const auto openglHandle = _texture.getNativeHandle();
    if (openglHandle == 0) {
        return std::nullopt;
    }
    return static_cast<std::int64_t>(openglHandle);
}

// ===== Canvas =====

// Viwes

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setView(const View& aView) {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setDefaultView() {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
const View& SFMLTextureCommonImpl<taUnderlying>::getView() const {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
const sf::View& SFMLTextureCommonImpl<taUnderlying>::getDefaultView() const {
    HG_UNREACHABLE("Illegal operation!");
}

// Drawing

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::clear(Color aColor) {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::draw(const Vertex*       aVertices,
                                               PZInteger           aVertexCount,
                                               PrimitiveType       aPrimitiveType,
                                               math::Vector2d      aAnchor,
                                               const RenderStates& aStates) {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::flush() {
    HG_UNREACHABLE("Illegal operation!");
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TEXTURE_COMMON_IMPL_HPP

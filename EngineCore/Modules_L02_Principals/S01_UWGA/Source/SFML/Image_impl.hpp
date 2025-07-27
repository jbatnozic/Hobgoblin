// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_IMAGE_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_IMAGE_IMPL_HPP

#include <Hobgoblin/UWGA/Image.hpp>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

class SFMLImageImpl : public Image {
public:
    SFMLImageImpl(const System& aSystem);

    SFMLImageImpl(const System& aSystem, const sf::Texture& aSfmlTexture);

    const sf::Image& getUnderlyingImage() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Image                                                           //
    ///////////////////////////////////////////////////////////////////////////

    // Reset

    void reset() override;

    void reset(PZInteger aWidth, PZInteger aHeight, Color aColor) override;

    void reset(math::Vector2pz aSize, Color aColor) override;

    void reset(PZInteger aWidth, PZInteger aHeight, NeverNull<const std::uint8_t*> aPixels) override;

    void reset(math::Vector2pz aSize, NeverNull<const std::uint8_t*> aPixels) override;

    void reset(const std::filesystem::path& aImagePath) override;

    // Saving

    void saveToFile(const std::filesystem::path& aPath) const override;

    // Pixel access

    void setPixel(PZInteger aX, PZInteger aY, Color aColor) override;

    Color getPixel(PZInteger aX, PZInteger aY) const override;

    const std::uint8_t* getPixelsPtr() const override;

    // Miscellaneous

    math::Vector2pz getSize() const override;

    void copy(const Image& aSource,
              PZInteger    aDestX,
              PZInteger    aDestY,
              TextureRect  aSourceRect = {0, 0, 0, 0},
              bool         aApplyAlpha = false) override;

    void createMaskFromColor(Color aColor, std::uint8_t aAlpha = 0) override;

    void flipHorizontally() override;
    void flipVertically() override;

private:
    const System& _system;
    sf::Image     _image;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_IMAGE_IMPL_HPP

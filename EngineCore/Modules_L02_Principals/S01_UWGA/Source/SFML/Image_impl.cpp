// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Image_impl.hpp"

#include "SFML_conversions.hpp"

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

///////////////////////////////////////////////////////////////////////////
// MARK: Element                                                         //
///////////////////////////////////////////////////////////////////////////

const System& SFMLImageImpl::getSystem() const {
    return _system;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Image                                                           //
///////////////////////////////////////////////////////////////////////////

// Reset

void SFMLImageImpl::reset() {
    _image.create(0u, 0u, sf::Color::Black);
}

void SFMLImageImpl::reset(PZInteger aWidth, PZInteger aHeight, Color aColor) {
    _image.create(static_cast<unsigned>(aWidth), static_cast<unsigned>(aHeight), ToSf(aColor));
}

void SFMLImageImpl::reset(math::Vector2pz aSize, Color aColor) {
    _image.create(static_cast<unsigned>(aSize.x), static_cast<unsigned>(aSize.y), ToSf(aColor));
}

void SFMLImageImpl::reset(PZInteger                      aWidth,
                          PZInteger                      aHeight,
                          NeverNull<const std::uint8_t*> aPixels) {
    _image.create(static_cast<unsigned>(aWidth), static_cast<unsigned>(aHeight), aPixels);
}

void SFMLImageImpl::reset(math::Vector2pz aSize, NeverNull<const std::uint8_t*> aPixels) {
    _image.create(static_cast<unsigned>(aSize.x), static_cast<unsigned>(aSize.y), aPixels);
}

void SFMLImageImpl::reset(const std::filesystem::path& aImagePath) {
    _image.loadFromFile(FilesystemPathToSfPath(aImagePath));
}

// Saving

void SFMLImageImpl::saveToFile(const std::filesystem::path& aPath) const {
    _image.saveToFile(FilesystemPathToSfPath(aPath));
}

// Pixel access

void SFMLImageImpl::setPixel(PZInteger aX, PZInteger aY, Color aColor) {
    _image.setPixel(static_cast<unsigned>(aX), static_cast<unsigned>(aY), ToSf(aColor));
}

Color SFMLImageImpl::getPixel(PZInteger aX, PZInteger aY) const {
    return ToHg(_image.getPixel(static_cast<unsigned>(aX), static_cast<unsigned>(aY)));
}

const std::uint8_t* SFMLImageImpl::getPixelsPtr() const {
    return _image.getPixelsPtr();
}

// Miscellaneous

math::Vector2pz SFMLImageImpl::getSize() const {
    const auto size = _image.getSize();
    return {static_cast<PZInteger>(size.x), static_cast<PZInteger>(size.y)};
}

void SFMLImageImpl::copy(const Image& aSource,
                         PZInteger    aDestX,
                         PZInteger    aDestY,
                         TextureRect  aSourceRect,
                         bool         aApplyAlpha) {
    assert(&aSource.getSystem() == &_system);
    assert(typeid(aSource) == typeid(SFMLImageImpl));

    const auto& sourceImpl = static_cast<const SFMLImageImpl&>(aSource);

    // clang-format off
    _image.copy(sourceImpl._image,
                static_cast<unsigned>(aDestX),
                static_cast<unsigned>(aDestY),
                {
                    static_cast<int>(aSourceRect.x),
                    static_cast<int>(aSourceRect.y),
                    static_cast<int>(aSourceRect.w),
                    static_cast<int>(aSourceRect.h)
                },
                aApplyAlpha);
    // clang-format on
}

void SFMLImageImpl::createMaskFromColor(Color aColor, std::uint8_t aAlpha) {
    _image.createMaskFromColor(ToSf(aColor), aAlpha);
}

void SFMLImageImpl::flipHorizontally() {
    _image.flipHorizontally();
}

void SFMLImageImpl::flipVertically() {
    _image.flipVertically();
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

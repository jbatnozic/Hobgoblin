// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SYSTEM_HPP
#define UHOBGOBLIN_UWGA_SYSTEM_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>
#include <Hobgoblin/UWGA/Window_style.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Image;
class RenderWindow;
class Transform;
class Texture;
class RenderTexture;
class View;

class System {
public:
    virtual ~System() = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RenderWindow                                                    //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<RenderWindow> createRenderWindow(
        PZInteger            aWidth      = 640,
        PZInteger            aHeight     = 480,
        WindowStyle          aStyle      = WindowStyle::DEFAULT,
        const UnicodeString& aTitle      = HG_UNILIT("Hobgoblin"),
        bool                 aEnableSRgb = false) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Image                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! Create an image of unspecified size and contents.
    virtual std::unique_ptr<Image> createImage() const = 0;

    //! Create an image of size `aWidth` x `aHeight` (in pixels) and fill it with color `aColor`.
    virtual std::unique_ptr<Image> createImage(PZInteger aWidth,
                                               PZInteger aHeight,
                                               Color     aColor = COLOR_BLACK) const = 0;

    //! Create an image of size `aSize.x` x `aSize.y` (in pixels) and fill it with color `aColor`.
    virtual std::unique_ptr<Image> createImage(math::Vector2pz aSize,
                                               Color           aColor = COLOR_BLACK) const = 0;

    //! Create an image of size `aWidth` x `aHeight` (in pixels) and fill it with pixels read from
    //! `aPixels`.
    //! \warning Calling this function triggers Undefined Behaviour if the `aPixels` pointer does
    //!          not point to a sufficiently-sized array of 32-bit RGBA pixels.
    virtual std::unique_ptr<Image> createImage(PZInteger                      aWidth,
                                               PZInteger                      aHeight,
                                               NeverNull<const std::uint8_t*> aPixels) const = 0;

    //! Create an image of size `aSize.x` x `aSize.y` (in pixels) and fill it with pixels read from
    //! `aPixels`.
    //! \warning Calling this function triggers Undefined Behaviour if the `aPixels` pointer does
    //!          not point to a sufficiently-sized array of 32-bit RGBA pixels.
    virtual std::unique_ptr<Image> createImage(math::Vector2pz                aSize,
                                               NeverNull<const std::uint8_t*> aPixels) const = 0;

    //! Create an image by loading it from a file on disk.
    //! The supported image formats are bmp, png, tga, jpg, gif, psd, hdr, pic and pnm.
    //! Some format options are not supported, like jpeg with arithmetic coding or ASCII pnm.
    //!
    //! \throws TracedRuntimeError on failure
    virtual std::unique_ptr<Image> createImage(const std::filesystem::path& aImagePath) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Texture                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Get the maximum texture size allowed
    //!
    //! This maximum size is defined by the graphics driver.
    //! You can expect a value of 512 pixels for low-end graphics
    //! card, and up to 8192 pixels or more for newer hardware.
    //!
    //! \return Maximum size allowed for textures, in pixels
    virtual PZInteger getMaximumTextureSize() const = 0;

    //! Create a default texture with unspecified size and settings.
    virtual std::unique_ptr<Texture> createTexture() const = 0;

    //! Create a texture of size `aWidth` x `aHeight` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    virtual std::unique_ptr<Texture> createTexture(PZInteger aWidth,
                                                   PZInteger aHeight,
                                                   bool      aEnableSRgb = false) const = 0;

    //! Create a texture of size `aSize.x` x `aSize.y` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    virtual std::unique_ptr<Texture> createTexture(math::Vector2pz aSize,
                                                   bool            aEnableSRgb = false) const = 0;

    //! Create a texture by loading an image file from the disk.
    //!
    //! The `aArea` argument can be used to load only a sub-rectangle of the whole image. If you want
    //! the entire image then leave the default value (which is an empty `TextureRect`). If the `aArea`
    //! rectangle crosses the bounds of the image, it is adjusted to fit the image size.
    //!
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<Texture> createTexture(const std::filesystem::path& aImagePath,
                                                   TextureRect                  aArea = {},
                                                   bool aEnableSRgb                   = false) const = 0;

    //! Create a texture by loading it from an `Image` object.
    //!
    //! The `aArea` argument can be used to load only a sub-rectangle of the whole image. If you want
    //! the entire image then leave the default value (which is an empty `TextureRect`). If the `aArea`
    //! rectangle crosses the bounds of the image, it is adjusted to fit the image size.
    //!
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<Texture> createTexture(const Image&      image,
                                                   const TextureRect aArea       = {},
                                                   bool              aEnableSRgb = false) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RenderTexture                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! Create a default render texture with unspecified size and settings.
    virtual std::unique_ptr<RenderTexture> createRenderTexture() const = 0;

    //! Create a render texture of size `aWidth` x `aHeight` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(PZInteger aWidth,
                                                               PZInteger aHeight,
                                                               bool      aEnableSRgb = false) const = 0;

    //! Create a render texture of size `aSize.x` x `aSize.y` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(math::Vector2pz aSize,
                                                               bool aEnableSRgb = false) const = 0;

    //! Create a render texture by loading an image file from the disk.
    //!
    //! The `aArea` argument can be used to load only a sub-rectangle of the whole image. If you want
    //! the entire image then leave the default value (which is an empty `TextureRect`). If the `aArea`
    //! rectangle crosses the bounds of the image, it is adjusted to fit the image size.
    //!
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(const std::filesystem::path& aImagePath,
                                                               TextureRect                  aArea = {},
                                                               bool aEnableSRgb = false) const = 0;

    //! Create a render texture by loading it from an `Image` object.
    //!
    //! The `aArea` argument can be used to load only a sub-rectangle of the whole image. If you want
    //! the entire image then leave the default value (which is an empty `TextureRect`). If the `aArea`
    //! rectangle crosses the bounds of the image, it is adjusted to fit the image size.
    //!
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(const Image&      image,
                                                               const TextureRect aArea = {},
                                                               bool aEnableSRgb = false) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: View                                                            //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<View> createView() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Transform                                                       //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<Transform> createTransform() const = 0;
};

//! Create a new rendering system.
//!
//! \param aSystemProviderName name of the rendering system provider. Must be one of the following:
//!                            - "SFML": Use SFML (OpenGL underneath).
//!
//! \throws TracedLogicError if an invalid provider name is passed.
std::unique_ptr<System> CreateRenderSystem(const char* aSystemProviderName);

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SYSTEM_HPP

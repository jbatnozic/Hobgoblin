// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2018 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef UHOBGOBLIN_UWGA_TEXTURE_HPP
#define UHOBGOBLIN_UWGA_TEXTURE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>
#include <Hobgoblin/UWGA/Window.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Image;

//! \brief Image living on the graphics card that can be used for drawing.
class Texture : virtual public Element {
public:
    //! \brief Virtual destructor.
    virtual ~Texture() = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RESET                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Reset the texture by changing its size.
    //!
    //! If this function fails, the texture is left unchanged.
    //! Otherwise, its contents are left in a valid but unspecified state.
    //!
    //! The maximum size for a texture depends on the graphics driver and can be
    //! retrieved with the `System::getMaximumTextureSize` function.
    //!
    //! \param aWidth  New width of the texture.
    //! \param aHeight New height of the texture.
    //! \param aEnableSRgb Whether to enable sRGB conversion or not.
    //!
    //! \throws TracedRuntimeError on failure.
    virtual void reset(PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb = false) = 0;

    //! \brief Reset the texture by loading new contents from a file on disk.
    //!
    //! The supported image formats are bmp, png, tga, jpg, gif, psd, hdr, pic and pnm.
    //! Some format options are not supported, like jpeg with arithmetic coding or ASCII pnm.
    //!
    //! The `aArea` argument can be used to load only a sub-rectangle of the whole image.
    //! If you want the entire image then leave the default value.
    //! If the `aArea` rectangle crosses the bounds of the image, it is adjusted to fit the image size.
    //!
    //! The maximum size for a texture depends on the graphics driver and can be
    //! retrieved with the `System::getMaximumTextureSize` function.
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param aPath Path of the image file to load.
    //! \param aArea Area of the image to load.
    //! \param aEnableSRgb Whether to enable sRGB conversion or not.
    //!
    //! \throws TracedRuntimeError on failure.
    virtual void reset(const std::filesystem::path& aPath,
                       TextureRect                  aArea       = {},
                       bool                         aEnableSRgb = false) = 0;

    //! \brief Reset the texture by loading new contents from an Image object.
    //!
    //! The `aArea` argument can be used to load only a sub-rectangle of the whole image.
    //! If you want the entire image then leave the default value.
    //! If the `aArea` rectangle crosses the bounds of the image, it is adjusted to fit the image size.
    //!
    //! The maximum size for a texture depends on the graphics driver and can be
    //! retrieved with the `System::getMaximumTextureSize` function.
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param aImage Image to load into the texture.
    //! \param aArea  Area of the image to load.
    //! \param aEnableSRgb Whether to enable sRGB conversion or not.
    //!
    //! \throws TracedRuntimeError on failure.
    virtual void reset(const Image& aImage, TextureRect aArea = {}, bool aEnableSRgb = false) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: UPDATING                                                        //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Update the whole texture from an array of pixels.
    //!
    //! \param aPixels array of 32-bit RGBA pixels. The function does nothing if this parameter is NULL.
    //! \param aPixelsByteCount byte size of the pixels array (first parameter).
    //!
    //! \throws InvalidArgumentError if `aPixelsByteCount` is not equal to size.x * size.y * 4.
    virtual void update(const std::uint8_t* aPixels, PZInteger aPixelsByteCount) = 0;

    //! \brief Update a part of the texture from an array of pixels.
    //!
    //! The size of the \a aPixels array must match the \a aWidth and
    //! \a aHeight arguments, and it must contain 32-bits RGBA pixels.
    //!
    //! This function does nothing if \a pixels is null or if the
    //! texture was not previously created.
    //!
    //! \param aPixels array of 32-bit RGBA pixels. The function does nothing if this parameter is NULL.
    //! \param aWidth  Width of the pixel region contained in \a aPixels.
    //! \param aHeight Height of the pixel region contained in \a aPixels.
    //! \param aX      X offset in the texture where to copy the source pixels.
    //! \param aY      Y offset in the texture where to copy the source pixels.
    virtual void update(const std::uint8_t* aPixels,
                        PZInteger           aWidth,
                        PZInteger           aHeight,
                        PZInteger           aX,
                        PZInteger           aY) = 0;

    //! \brief Update a part of this texture from another texture
    //!
    //! Although the source texture can be smaller than this texture,
    //! this function is usually used for updating the whole texture.
    //! The other overload, which has (x, y) additional arguments,
    //! is more convenient for updating a sub-area of this texture.
    //!
    //! No additional check is performed on the size of the passed
    //! texture, passing a texture bigger than this texture
    //! will lead to an undefined behavior.
    //!
    //! \param aTexture Source texture to copy to this texture.
    virtual void update(const Texture& aTexture) = 0;

    //! \brief Update a part of this texture from another texture.
    //!
    //! No additional check is performed on the size of the texture,
    //! passing an invalid combination of texture size and offset
    //! will lead to an undefined behavior.
    //!
    //! \param aTexture Source texture to copy to this texture.
    //! \param aX       X offset in this texture where to copy the source texture.
    //! \param aY       Y offset in this texture where to copy the source texture.
    virtual void update(const Texture& aTexture, PZInteger aX, PZInteger aY) = 0;

    //! \brief Update the texture from an image.
    //!
    //! Although the source image can be smaller than the texture,
    //! this function is usually used for updating the whole texture.
    //! The other overload, which has (x, y) additional arguments,
    //! is more convenient for updating a sub-area of the texture.
    //!
    //! No additional check is performed on the size of the image,
    //! passing an image bigger than the texture will lead to an
    //! undefined behavior.
    //!
    //! \param aImage Image to copy to the texture.
    virtual void update(const Image& aImage) = 0;

    //! \brief Update a part of the texture from an image.
    //!
    //! No additional check is performed on the size of the image,
    //! passing an invalid combination of image size and offset
    //! will lead to an undefined behavior.
    //!
    //! \param aImage Image to copy to the texture.
    //! \param aX     X offset in the texture where to copy the source image.
    //! \param aY     Y offset in the texture where to copy the source image.
    virtual void update(const Image& aImage, PZInteger aX, PZInteger aY) = 0;

    //! \brief Update the texture from the contents of a window.
    //!
    //! Although the source window can be smaller than the texture,
    //! this function is usually used for updating the whole texture.
    //! The other overload, which has (x, y) additional arguments,
    //! is more convenient for updating a sub-area of the texture.
    //!
    //! No additional check is performed on the size of the window,
    //! passing a window bigger than the texture will lead to an
    //! undefined behavior.
    //!
    //! \param aWindow Window to copy to the texture.
    virtual void update(const Window& aWindow) = 0;

    //! \brief Update a part of the texture from the contents of a window.
    //!
    //! No additional check is performed on the size of the window,
    //! passing an invalid combination of window size and offset
    //! will lead to an undefined behavior.
    //!
    //! \param aWindow Window to copy to the texture.
    //! \param aX      X offset in the texture where to copy the source window.
    //! \param aY      Y offset in the texture where to copy the source window.
    virtual void update(const Window& aWindow, PZInteger aX, PZInteger aY) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: MISCELLANEOUS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Return the size of the texture
    //!
    //! \return Size in pixels
    virtual math::Vector2pz getSize() const = 0;

    //! \brief Enable or disable the smooth filter
    //!
    //! When the filter is activated, the texture appears smoother
    //! so that pixels are less noticeable. However if you want
    //! the texture to look exactly the same as its source file,
    //! you should leave it disabled.
    //! The smooth filter is disabled by default.
    //!
    //! \param smooth True to enable smoothing, false to disable it
    virtual void setSmooth(bool aSmooth) = 0;

    //! \brief Tell whether the smooth filter is enabled or not
    //!
    //! \return True if smoothing is enabled, false if it is disabled
    virtual bool isSmooth() const = 0;

    //! \brief Tell whether the texture source is converted from sRGB or not
    //!
    //! \return True if the texture source is converted from sRGB, false if not
    virtual bool isSrgb() const = 0;

    //! \brief Enable or disable repeating
    //!
    //! Repeating is involved when using texture coordinates
    //! outside the texture rectangle [0, 0, width, height].
    //! In this case, if repeat mode is enabled, the whole texture
    //! will be repeated as many times as needed to reach the
    //! coordinate (for example, if the X texture coordinate is
    //! 3 * width, the texture will be repeated 3 times).
    //! If repeat mode is disabled, the "extra space" will instead
    //! be filled with border pixels.
    //! Warning: on very old graphics cards, white pixels may appear
    //! when the texture is repeated. With such cards, repeat mode
    //! can be used reliably only if the texture has power-of-two
    //! dimensions (such as 256x128).
    //! Repeating is disabled by default.
    //!
    //! \param repeated True to repeat the texture, false to disable repeating
    //!
    //! \see isRepeated
    virtual void setRepeated(bool aRepeated) = 0;

    //! \brief Tell whether the texture is repeated or not
    //!
    //! \return True if repeat mode is enabled, false if it is disabled
    //!
    //! \see setRepeated
    virtual bool isRepeated() const = 0;

    //! \brief Generate a mipmap using the current texture data
    //!
    //! Mipmaps are pre-computed chains of optimized textures. Each
    //! level of texture in a mipmap is generated by halving each of
    //! the previous level's dimensions. This is done until the final
    //! level has the size of 1x1. The textures generated in this process may
    //! make use of more advanced filters which might improve the visual quality
    //! of textures when they are applied to objects much smaller than they are.
    //! This is known as minification. Because fewer texels (texture elements)
    //! have to be sampled from when heavily minified, usage of mipmaps
    //! can also improve rendering performance in certain scenarios.
    //!
    //! Mipmap generation relies on the necessary OpenGL extension being
    //! available. If it is unavailable or generation fails due to another
    //! reason, this function will return false. Mipmap data is only valid from
    //! the time it is generated until the next time the base level image is
    //! modified, at which point this function will have to be called again to
    //! regenerate it.
    //!
    //! \return True if mipmap generation was successful, false if unsuccessful
    [[nodiscard]] virtual bool generateMipmap() = 0;

    //! \brief Swap the contents of this texture with those of another
    //!
    //! \param right Instance to swap with
    virtual void swap(Texture& aOther) = 0;

    //! \brief Copy the texture pixels to an image
    //!
    //! This function performs a slow operation that downloads
    //! the texture's pixels from the graphics card and copies
    //! them to a new image, potentially applying transformations
    //! to pixels if necessary (texture may be padded or flipped).
    //!
    //! \return Image containing the texture's pixels
    //!
    //! \see loadFromImage
    [[nodiscard]] virtual std::unique_ptr<Image> copyToImage() const = 0;

    //! \brief Get the underlying OpenGL handle of the texture.
    //!
    //! You shouldn't need to use this function, unless you have
    //! very specific stuff to implement that SFML doesn't support,
    //! or implement a temporary workaround until a bug is fixed.
    //!
    //! \return OpenGL handle of the texture or OPENGL_HANDLE_NONE if not yet created
    [[nodiscard]] virtual std::optional<std::int64_t> getNativeHandle() const = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TEXTURE_HPP

////////////////////////////////////////////////////////////
/// \class sf::Texture
/// \ingroup graphics
///
/// sf::Texture stores pixels that can be drawn, with a sprite
/// for example. A texture lives in the graphics card memory,
/// therefore it is very fast to draw a texture to a render target,
/// or copy a render target to a texture (the graphics card can
/// access both directly).
///
/// Being stored in the graphics card memory has some drawbacks.
/// A texture cannot be manipulated as freely as a sf::Image,
/// you need to prepare the pixels first and then upload them
/// to the texture in a single operation (see Texture::update).
///
/// sf::Texture makes it easy to convert from/to sf::Image, but
/// keep in mind that these calls require transfers between
/// the graphics card and the central memory, therefore they are
/// slow operations.
///
/// A texture can be loaded from an image, but also directly
/// from a file/memory/stream. The necessary shortcuts are defined
/// so that you don't need an image first for the most common cases.
/// However, if you want to perform some modifications on the pixels
/// before creating the final texture, you can load your file to a
/// sf::Image, do whatever you need with the pixels, and then call
/// Texture::loadFromImage.
///
/// Since they live in the graphics card memory, the pixels of a texture
/// cannot be accessed without a slow copy first. And they cannot be
/// accessed individually. Therefore, if you need to read the texture's
/// pixels (like for pixel-perfect collisions), it is recommended to
/// store the collision information separately, for example in an array
/// of booleans.
///
/// Like sf::Image, sf::Texture can handle a unique internal
/// representation of pixels, which is RGBA 32 bits. This means
/// that a pixel must be composed of 8 bits red, green, blue and
/// alpha channels -- just like a sf::Color.
///
/// Usage example:
/// \code
/// // This example shows the most common use of sf::Texture:
/// // drawing a sprite
///
/// // Load a texture from a file
/// sf::Texture texture;
/// if (!texture.loadFromFile("texture.png"))
///     return -1;
///
/// // Assign it to a sprite
/// sf::Sprite sprite;
/// sprite.setTexture(texture);
///
/// // Draw the textured sprite
/// window.draw(sprite);
/// \endcode
///
/// \code
/// // This example shows another common use of sf::Texture:
/// // streaming real-time data, like video frames
///
/// // Create an empty texture
/// sf::Texture texture;
/// if (!texture.create(640, 480))
///     return -1;
///
/// // Create a sprite that will display the texture
/// sf::Sprite sprite(texture);
///
/// while (...) // the main loop
/// {
///     ...
///
///     // update the texture
///     sf::Uint8* pixels = ...; // get a fresh chunk of pixels (the next frame of a movie, for example)
///     texture.update(pixels);
///
///     // draw it
///     window.draw(sprite);
///
///     ...
/// }
///
/// \endcode
///
/// Like sf::Shader that can be used as a raw OpenGL shader,
/// sf::Texture can also be used directly as a raw texture for
/// custom OpenGL geometry.
/// \code
/// sf::Texture::bind(&texture);
/// ... render OpenGL geometry ...
/// sf::Texture::bind(NULL);
/// \endcode
///
/// \see sf::Sprite, sf::Image, sf::RenderTexture
///
////////////////////////////////////////////////////////////

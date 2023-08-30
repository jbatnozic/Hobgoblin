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

#ifndef UHOBGOBLIN_GRAPHICS_TEXTURE_HPP
#define UHOBGOBLIN_GRAPHICS_TEXTURE_HPP

#include <Hobgoblin/Graphics/OpenGL_handle.hpp>
#include <Hobgoblin/Graphics/Image.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <cstdint>
#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

namespace detail {
class GraphicsImplAccessor;
} // namespace detail

class InputStream;
class RenderTarget;
class RenderTexture;
class Text;
class Window;

//! \brief Image living on the graphics card that can be used for drawing.
class Texture {
public:
    //! \brief Types of texture coordinates that can be used for rendering
    enum class CoordinateType {
        Normalized, //!< Texture coordinates in range [0 .. 1]
        Pixels      //!< Texture coordinates in range [0 .. size]
    };

    //! \brief Default constructor
    //!
    //! Creates an empty texture.
    Texture();

    //! \brief Copy constructor
    //!
    //! \param aOther instance to copy
    Texture(const Texture& aOther);

    //! \brief Destructor
    ~Texture();

    //! \brief Create the texture
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param width  Width of the texture
    //! \param height Height of the texture
    //!
    //! \return True if creation was successful
    bool create(PZInteger aWidth, PZInteger aHeight);

    //! \brief Load the texture from a file on disk
    //!
    //! This function is a shortcut for the following code:
    //! \code
    //! sf::Image image;
    //! image.loadFromFile(filename);
    //! texture.loadFromImage(image, area);
    //! \endcode
    //!
    //! The \a area argument can be used to load only a sub-rectangle
    //! of the whole image. If you want the entire image then leave
    //! the default value (which is an empty IntRect).
    //! If the \a area rectangle crosses the bounds of the image, it
    //! is adjusted to fit the image size.
    //!
    //! The maximum size for a texture depends on the graphics
    //! driver and can be retrieved with the getMaximumSize function.
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param filename Path of the image file to load
    //! \param area     Area of the image to load
    //!
    //! \return True if loading was successful
    //!
    //! \see loadFromMemory, loadFromStream, loadFromImage
    bool loadFromFile(const std::string& aFilename, const math::Rectangle<int>& aArea = {});

    //! \brief Load the texture from a file in memory
    //!
    //! This function is a shortcut for the following code:
    //! \code
    //! sf::Image image;
    //! image.loadFromMemory(data, size);
    //! texture.loadFromImage(image, area);
    //! \endcode
    //!
    //! The \a area argument can be used to load only a sub-rectangle
    //! of the whole image. If you want the entire image then leave
    //! the default value (which is an empty IntRect).
    //! If the \a area rectangle crosses the bounds of the image, it
    //! is adjusted to fit the image size.
    //!
    //! The maximum size for a texture depends on the graphics
    //! driver and can be retrieved with the getMaximumSize function.
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param data Pointer to the file data in memory
    //! \param size Size of the data to load, in bytes
    //! \param area Area of the image to load
    //!
    //! \return True if loading was successful
    //!
    //! \see loadFromFile, loadFromStream, loadFromImage
    bool loadFromMemory(const void* aData, PZInteger aSize, const math::Rectangle<int>& aArea = {});

    //! \brief Load the texture from a custom stream
    //!
    //! This function is a shortcut for the following code:
    //! \code
    //! sf::Image image;
    //! image.loadFromStream(stream);
    //! texture.loadFromImage(image, area);
    //! \endcode
    //!
    //! The \a area argument can be used to load only a sub-rectangle
    //! of the whole image. If you want the entire image then leave
    //! the default value (which is an empty IntRect).
    //! If the \a area rectangle crosses the bounds of the image, it
    //! is adjusted to fit the image size.
    //!
    //! The maximum size for a texture depends on the graphics
    //! driver and can be retrieved with the getMaximumSize function.
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param stream Source stream to read from
    //! \param area   Area of the image to load
    //!
    //! \return True if loading was successful
    //!
    //! \see loadFromFile, loadFromMemory, loadFromImage
    // bool loadFromStream(InputStream& stream, const math::Rectangle<int>& aArea = {}); TODO

    //! \brief Load the texture from an image
    //!
    //! The \a area argument can be used to load only a sub-rectangle
    //! of the whole image. If you want the entire image then leave
    //! the default value (which is an empty IntRect).
    //! If the \a area rectangle crosses the bounds of the image, it
    //! is adjusted to fit the image size.
    //!
    //! The maximum size for a texture depends on the graphics
    //! driver and can be retrieved with the getMaximumSize function.
    //!
    //! If this function fails, the texture is left unchanged.
    //!
    //! \param image Image to load into the texture
    //! \param area  Area of the image to load
    //!
    //! \return True if loading was successful
    //!
    //! \see loadFromFile, loadFromMemory
    bool loadFromImage(const Image& aImage, const const math::Rectangle<int>& aArea = {});

    //! \brief Return the size of the texture
    //!
    //! \return Size in pixels
    math::Vector2pz getSize() const;

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
    Image copyToImage() const;

    //! \brief Update the whole texture from an array of pixels
    //!
    //! The \a pixel array is assumed to have the same size as
    //! the \a area rectangle, and to contain 32-bits RGBA pixels.
    //!
    //! No additional check is performed on the size of the pixel
    //! array, passing invalid arguments will lead to an undefined
    //! behavior.
    //!
    //! This function does nothing if \a pixels is null or if the
    //! texture was not previously created.
    //!
    //! \param pixels Array of pixels to copy to the texture
    void update(const std::uint8_t* aPixels);

    //! \brief Update a part of the texture from an array of pixels
    //!
    //! The size of the \a pixel array must match the \a width and
    //! \a height arguments, and it must contain 32-bits RGBA pixels.
    //!
    //! No additional check is performed on the size of the pixel
    //! array or the bounds of the area to update, passing invalid
    //! arguments will lead to an undefined behavior.
    //!
    //! This function does nothing if \a pixels is null or if the
    //! texture was not previously created.
    //!
    //! \param pixels Array of pixels to copy to the texture
    //! \param width  Width of the pixel region contained in \a pixels
    //! \param height Height of the pixel region contained in \a pixels
    //! \param x      X offset in the texture where to copy the source pixels
    //! \param y      Y offset in the texture where to copy the source pixels
    void update(
        const std::uint8_t* aPixels,
        PZInteger aWidth,
        PZInteger aHeight,
        PZInteger aX,
        PZInteger aY
    );

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
    //! This function does nothing if either texture was not
    //! previously created.
    //!
    //! \param texture Source texture to copy to this texture
    void update(const Texture& aTexture);

    //! \brief Update a part of this texture from another texture
    //!
    //! No additional check is performed on the size of the texture,
    //! passing an invalid combination of texture size and offset
    //! will lead to an undefined behavior.
    //!
    //! This function does nothing if either texture was not
    //! previously created.
    //!
    //! \param texture Source texture to copy to this texture
    //! \param x       X offset in this texture where to copy the source texture
    //! \param y       Y offset in this texture where to copy the source texture
    void update(const Texture& aTexture, PZInteger aX, PZInteger aY);

    //! \brief Update the texture from an image
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
    //! This function does nothing if the texture was not
    //! previously created.
    //!
    //! \param image Image to copy to the texture
    // void update(const Image& aImage); TODO

    //! \brief Update a part of the texture from an image
    //!
    //! No additional check is performed on the size of the image,
    //! passing an invalid combination of image size and offset
    //! will lead to an undefined behavior.
    //!
    //! This function does nothing if the texture was not
    //! previously created.
    //!
    //! \param image Image to copy to the texture
    //! \param x     X offset in the texture where to copy the source image
    //! \param y     Y offset in the texture where to copy the source image
    // void update(const Image& aImage, unsigned int aX, unsigned int aY); TODO

    //! \brief Update the texture from the contents of a window
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
    //! This function does nothing if either the texture or the window
    //! was not previously created.
    //!
    //! \param window Window to copy to the texture
    // void update(const Window& Window); TODO

    //! \brief Update a part of the texture from the contents of a window
    //!
    //! No additional check is performed on the size of the window,
    //! passing an invalid combination of window size and offset
    //! will lead to an undefined behavior.
    //!
    //! This function does nothing if either the texture or the window
    //! was not previously created.
    //!
    //! \param window Window to copy to the texture
    //! \param x      X offset in the texture where to copy the source window
    //! \param y      Y offset in the texture where to copy the source window
    // void update(const Window& aWindow, PZInteger aX, PZInteger aY); TODO

    //! \brief Enable or disable the smooth filter
    //!
    //! When the filter is activated, the texture appears smoother
    //! so that pixels are less noticeable. However if you want
    //! the texture to look exactly the same as its source file,
    //! you should leave it disabled.
    //! The smooth filter is disabled by default.
    //!
    //! \param smooth True to enable smoothing, false to disable it
    //!
    //! \see isSmooth
    void setSmooth(bool aSmooth);

    //! \brief Tell whether the smooth filter is enabled or not
    //!
    //! \return True if smoothing is enabled, false if it is disabled
    //!
    //! \see setSmooth
    bool isSmooth() const;

    //! \brief Enable or disable conversion from sRGB
    //!
    //! When providing texture data from an image file or memory, it can
    //! either be stored in a linear color space or an sRGB color space.
    //! Most digital images account for gamma correction already, so they
    //! would need to be "uncorrected" back to linear color space before
    //! being processed by the hardware. The hardware can automatically
    //! convert it from the sRGB color space to a linear color space when
    //! it gets sampled. When the rendered image gets output to the final
    //! framebuffer, it gets converted back to sRGB.
    //!
    //! After enabling or disabling sRGB conversion, make sure to reload
    //! the texture data in order for the setting to take effect.
    //!
    //! This option is only useful in conjunction with an sRGB capable
    //! framebuffer. This can be requested during window creation.
    //!
    //! \param sRgb True to enable sRGB conversion, false to disable it
    //!
    //! \see isSrgb
    void setSrgb(bool aSRgb);

    //! \brief Tell whether the texture source is converted from sRGB or not
    //!
    //! \return True if the texture source is converted from sRGB, false if not
    //!
    //! \see setSrgb
    bool isSrgb() const;

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
    void setRepeated(bool aRepeated);

    //! \brief Tell whether the texture is repeated or not
    //!
    //! \return True if repeat mode is enabled, false if it is disabled
    //!
    //! \see setRepeated
    bool isRepeated() const;

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
    bool generateMipmap();

    //! \brief Overload of assignment operator
    //!
    //! \param right Instance to assign
    //!
    //! \return Reference to self
    Texture& operator=(const Texture& aRhs);

    //! \brief Swap the contents of this texture with those of another
    //!
    //! \param right Instance to swap with
    void swap(Texture& aOther);

    //! \brief Get the underlying OpenGL handle of the texture.
    //!
    //! You shouldn't need to use this function, unless you have
    //! very specific stuff to implement that SFML doesn't support,
    //! or implement a temporary workaround until a bug is fixed.
    //!
    //! \return OpenGL handle of the texture or OPENGL_HANDLE_NONE if not yet created
    OpenGLHandle getNativeHandle() const;

    //! \brief Bind a texture for rendering
    //!
    //! This function is not part of the graphics API, it mustn't be
    //! used when drawing SFML entities. It must be used only if you
    //! mix sf::Texture with OpenGL code.
    //!
    //! \code
    //! sf::Texture t1, t2;
    //! ...
    //! sf::Texture::bind(&t1);
    //! // draw OpenGL stuff that use t1...
    //! sf::Texture::bind(&t2);
    //! // draw OpenGL stuff that use t2...
    //! sf::Texture::bind(NULL);
    //! // draw OpenGL stuff that use no texture...
    //! \endcode
    //!
    //! The \a coordinateType argument controls how texture
    //! coordinates will be interpreted. If Normalized (the default), they
    //! must be in range [0 .. 1], which is the default way of handling
    //! texture coordinates with OpenGL. If Pixels, they must be given
    //! in pixels (range [0 .. size]). This mode is used internally by
    //! the graphics classes of SFML, it makes the definition of texture
    //! coordinates more intuitive for the high-level API, users don't need
    //! to compute normalized values.
    //!
    //! \param texture Pointer to the texture to bind, can be null to use no texture
    //! \param coordinateType Type of texture coordinates to use
    static void bind(const Texture* aTexture, CoordinateType aCoordinateType = CoordinateType::Normalized);

    //! \brief Get the maximum texture size allowed
    //!
    //! This maximum size is defined by the graphics driver.
    //! You can expect a value of 512 pixels for low-end graphics
    //! card, and up to 8192 pixels or more for newer hardware.
    //!
    //! \return Maximum size allowed for textures, in pixels
    static PZInteger getMaximumSize();

private:
    friend class detail::GraphicsImplAccessor;

    void* _getSFMLImpl();
    const void* _getSFMLImpl() const;

    static constexpr ::std::size_t STORAGE_SIZE  = 40;
    static constexpr ::std::size_t STORAGE_ALIGN =  8;
    ::std::aligned_storage<STORAGE_SIZE, STORAGE_ALIGN>::type _storage;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_GRAPHICS_TEXTURE_HPP

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
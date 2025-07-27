// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef UHOBGOBLIN_UWGA_IMAGE_HPP
#define UHOBGOBLIN_UWGA_IMAGE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>

#include <cstdint>
#include <filesystem>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! Class for loading, manipulating and saving images.
class Image : virtual public Element {
public:
    //! Virtual destructor.
    virtual ~Image() = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RESET                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! Reset the image to an empty image of size 0x0.
    virtual void reset() = 0;

    //! Reset the image to an image of size `aWidth` x `aHeight` (in pixels)
    //! and fill it with color `aColor`.
    virtual void reset(PZInteger aWidth, PZInteger aHeight, Color aColor = COLOR_BLACK) = 0;

    //! Reset the image to an image of size `aSize.x` x `aSize.y` (in pixels)
    //! and fill it with color `aColor`.
    virtual void reset(math::Vector2pz aSize, Color aColor = COLOR_BLACK) = 0;

    //! Reset the image to an image of size `aWidth` x `aHeight` (in pixels)
    //! and fill it with pixels read from `aPixels`.
    //! \warning Calling this function triggers Undefined Behaviour if the `aPixels` pointer does
    //!          not point to a sufficiently-sized array of 32-bit RGBA pixels.
    virtual void reset(PZInteger aWidth, PZInteger aHeight, NeverNull<const std::uint8_t*> aPixels) = 0;

    //! Reset the image to an image of size `aSize.x` x `aSize.y` (in pixels)
    //! and fill it with pixels read from `aPixels`.
    //! \warning Calling this function triggers Undefined Behaviour if the `aPixels` pointer does
    //!          not point to a sufficiently-sized array of 32-bit RGBA pixels.
    virtual void reset(math::Vector2pz aSize, NeverNull<const std::uint8_t*> aPixels) = 0;

    //! Reset the image by loading new contents from a file on disk.
    //! The supported image formats are bmp, png, tga, jpg, gif, psd, hdr, pic and pnm.
    //! Some format options are not supported, like jpeg with arithmetic coding or ASCII pnm.
    //!
    //! \throws TracedRuntimeError on failure
    virtual void reset(const std::filesystem::path& aImagePath) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SAVING                                                          //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Save the image to a file on disk
    //!
    //! The format of the image is automatically deduced from
    //! the extension. The supported image formats are bmp, png,
    //! tga and jpg. The destination file is overwritten
    //! if it already exists. This function fails if the image is empty.
    //!
    //! \param filename Path of the file to save
    //!
    //! \return True if saving was successful
    //!
    //! \see create, loadFromFile, loadFromMemory
    virtual void saveToFile(const std::filesystem::path& aPath) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: PIXEL ACCESS                                                    //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Change the color of a pixel
    //!
    //! This function doesn't check the validity of the pixel
    //! coordinates, using out-of-range values will result in
    //! an undefined behavior.
    //!
    //! \param x     X coordinate of pixel to change
    //! \param y     Y coordinate of pixel to change
    //! \param color New color of the pixel
    //!
    //! \see getPixel
    virtual void setPixel(PZInteger aX, PZInteger aY, Color aColor) = 0;

    //! \brief Get the color of a pixel
    //!
    //! This function doesn't check the validity of the pixel
    //! coordinates, using out-of-range values will result in
    //! an undefined behavior.
    //!
    //! \param x X coordinate of pixel to get
    //! \param y Y coordinate of pixel to get
    //!
    //! \return Color of the pixel at coordinates (x, y)
    //!
    //! \see setPixel
    virtual Color getPixel(PZInteger aX, PZInteger aY) const = 0;

    //! \brief Get a read-only pointer to the array of pixels
    //!
    //! The returned value points to an array of RGBA pixels made of
    //! 8 bits integers components. The size of the array is
    //! width * height * 4 (getSize().x * getSize().y * 4).
    //! Warning: the returned pointer may become invalid if you
    //! modify the image, so you should never store it for too long.
    //! If the image is empty, a null pointer is returned.
    //!
    //! \return Read-only pointer to the array of pixels
    virtual const std::uint8_t* getPixelsPtr() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: MISCELLANEOUS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Return the size (width and height) of the image
    //!
    //! \return Size of the image, in pixels
    virtual math::Vector2pz getSize() const = 0;

    //! \brief Copy pixels from another image onto this one
    //!
    //! This function does a slow pixel copy and should not be
    //! used intensively. It can be used to prepare a complex
    //! static image from several others, but if you need this
    //! kind of feature in real-time you'd better use sf::RenderTexture.
    //!
    //! If \a sourceRect is empty, the whole image is copied.
    //! If \a applyAlpha is set to true, the transparency of
    //! source pixels is applied. If it is false, the pixels are
    //! copied unchanged with their alpha value.
    //!
    //! \param aSource     Source image to copy
    //! \param aDestX      X coordinate of the destination position
    //! \param aDestY      Y coordinate of the destination position
    //! \param aSourceRect Sub-rectangle of the source image to copy
    //! \param aApplyAlpha Should the copy take into account the source transparency?
    virtual void copy(const Image& aSource,
                      PZInteger    aDestX,
                      PZInteger    aDestY,
                      TextureRect  aSourceRect = {0, 0, 0, 0},
                      bool         aApplyAlpha = false) = 0;

    //! \brief Create a transparency mask from a specified color-key
    //!
    //! This function sets the alpha value of every pixel matching
    //! the given color to \a alpha (0 by default), so that they
    //! become transparent.
    //!
    //! \param aColor Color to make transparent
    //! \param aAlpha Alpha value to assign to transparent pixels
    virtual void createMaskFromColor(Color aColor, std::uint8_t aAlpha = 0) = 0;

    //! \brief Flip the image horizontally (left <-> right)
    virtual void flipHorizontally() = 0;

    //! \brief Flip the image vertically (top <-> bottom)
    virtual void flipVertically() = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_IMAGE_HPP

////////////////////////////////////////////////////////////
/// \class sf::Image
/// \ingroup graphics
///
/// sf::Image is an abstraction to manipulate images
/// as bidimensional arrays of pixels. The class provides
/// functions to load, read, write and save pixels, as well
/// as many other useful functions.
///
/// sf::Image can handle a unique internal representation of
/// pixels, which is RGBA 32 bits. This means that a pixel
/// must be composed of 8 bits red, green, blue and alpha
/// channels -- just like a sf::Color.
/// All the functions that return an array of pixels follow
/// this rule, and all parameters that you pass to sf::Image
/// functions (such as loadFromMemory) must use this
/// representation as well.
///
/// A sf::Image can be copied, but it is a heavy resource and
/// if possible you should always use [const] references to
/// pass or return them to avoid useless copies.
///
/// Usage example:
/// \code
/// // Load an image file from a file
/// sf::Image background;
/// if (!background.loadFromFile("background.jpg"))
///     return -1;
///
/// // Create a 20x20 image filled with black color
/// sf::Image image;
/// image.create(20, 20, sf::Color::Black);
///
/// // Copy image1 on image2 at position (10, 10)
/// image.copy(background, 10, 10);
///
/// // Make the top-left pixel transparent
/// sf::Color color = image.getPixel(0, 0);
/// color.a = 0;
/// image.setPixel(0, 0, color);
///
/// // Save the image to a file
/// if (!image.saveToFile("result.png"))
///     return -1;
/// \endcode
///
/// \see sf::Texture
///
////////////////////////////////////////////////////////////

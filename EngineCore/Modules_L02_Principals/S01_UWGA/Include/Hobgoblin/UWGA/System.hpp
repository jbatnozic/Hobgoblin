// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SYSTEM_HPP
#define UHOBGOBLIN_UWGA_SYSTEM_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Window_style.hpp>
#include <Hobgoblin/Math/Vector.hpp>
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
class VertexArray;
class View;

class System {
public:
    virtual ~System() = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RenderWindow                                                    //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<RenderWindow> createRenderWindow(
        PZInteger            aWidth  = 640,
        PZInteger            aHeight = 480,
        WindowStyle          aStyle  = WindowStyle::DEFAULT,
        const UnicodeString& aTitle  = HG_UNILIT("Hobgoblin")) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Image                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! Create an empty image of size 0x0.
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

    // TODO

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RenderTexture                                                   //
    ///////////////////////////////////////////////////////////////////////////

    // TODO

    ///////////////////////////////////////////////////////////////////////////
    // MARK: View                                                            //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<View> createView() const = 0;

    virtual std::unique_ptr<View> createDefaultView(const RenderWindow& aRenderWindow) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Transform                                                       //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<Transform> createTransform() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: VertexArray                                                     //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<VertexArray> createVertexArray(PZInteger aSize) const = 0;
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

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SYSTEM_HPP
#define UHOBGOBLIN_UWGA_SYSTEM_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Batching_config.hpp>
#include <Hobgoblin/UWGA/Builtin_fonts.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Shader.hpp>
#include <Hobgoblin/UWGA/Text.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>
#include <Hobgoblin/UWGA/Window_style.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Font;
class GLSLShader;
class Image;
class RenderWindow;
class Transform;
class Texture;
class RenderTexture;
class View;

//! \brief Virtual interface for a graphics system.
//!
//! It is the entry point for all windowing and graphics/rendering operations, and provides
//! functions to inspect the capabilities of the system and create various graphics elements.
class System {
public:
    //! \brief Virtual destructor.
    virtual ~System() = default;

    //! Graphics system provider designator.
    enum class Provider {
        SFML
    };

    //! Get the designator of the graphics system provider.
    //!
    //! In general this will be the same value that you pass to `CreateGraphicsSystem(System::Provider)`
    //! when creating the system.
    virtual Provider getProvider() const = 0;

    //! Get the name of the graphics system provider.
    //! Depending on the value returned by `getProvider()`, the returned string can be:
    //! - SFML -> "SFML"
    //!
    //! In general this will be the same string that you pass to `CreateGraphicsSystem(const char*)`
    //! when creating the system.
    virtual const char* getProviderName() const = 0;

    //! Graphics API designator.
    enum class GraphicsAPI {
        OPENGL,   //!< OpenGL.
        DIRECT3D, //!< Direct3D (part of DirectX). WARNING: Not yet supported.
        VULKAN,   //!< Vulkan (cross-platform successor to OpenGL). WARNING: Not yet supported.
        METAL     //!< Metal (replacement for OpenGL on Apple platforms). WARNING: Not yet supported.
    };

    //! Get the designator of the underlying graphics API used by the system.
    virtual GraphicsAPI getGraphicsAPI() const = 0;

    //! Get the name of the underlying graphics API used by the system.
    //! Depending on the value returned by `getGraphicsAPI()`, the returned string can be:
    //! - OPENGL   -> "OpenGL"
    //! - DIRECT3D -> "Direct3D"
    //! - VULKAN   -> "Vulkan"
    //! - METAL    -> "Metal"
    virtual const char* getGraphicsAPIName() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RenderWindow                                                    //
    ///////////////////////////////////////////////////////////////////////////

    virtual std::unique_ptr<RenderWindow> createRenderWindow(
        const BatchingConfig& aBatchingConfig,
        PZInteger             aWidth      = 640,
        PZInteger             aHeight     = 480,
        WindowStyle           aStyle      = WindowStyle::DEFAULT,
        const UnicodeString&  aTitle      = HG_UNILIT("Hobgoblin"),
        bool                  aEnableSRgb = false) const = 0;

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
    virtual std::unique_ptr<RenderTexture> createRenderTexture(
        const BatchingConfig& aBatchingConfig) const = 0;

    //! Create a default render texture with unspecified size and settings.
    //!
    //! \note this overload of the function takes no `BatchingConfig` parameter,
    //!       so the default is assumed (batching disabled).
    virtual std::unique_ptr<RenderTexture> createRenderTexture() const = 0;

    //! Create a render texture of size `aWidth` x `aHeight` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                               PZInteger             aWidth,
                                                               PZInteger             aHeight,
                                                               bool aEnableSRgb = false) const = 0;

    //! Create a render texture of size `aWidth` x `aHeight` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note this overload of the function takes no `BatchingConfig` parameter,
    //!       so the default is assumed (batching disabled).
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(PZInteger aWidth,
                                                               PZInteger aHeight,
                                                               bool      aEnableSRgb = false) const = 0;

    //! Create a render texture of size `aSize.x` x `aSize.y` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
    virtual std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                               math::Vector2pz       aSize,
                                                               bool aEnableSRgb = false) const = 0;

    //! Create a render texture of size `aSize.x` x `aSize.y` (in pixels).
    //! Set `aEnableSRgb` to `true` to enable sRGB conversion, or to `false ` to disable it.
    //!
    //! \note this overload of the function takes no `BatchingConfig` parameter,
    //!       so the default is assumed (batching disabled).
    //!
    //! \note The maximum size for a texture depends on the graphics driver and can be retrieved
    //!       with the `getMaximumTextureSize` function.
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
    virtual std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                               const std::filesystem::path& aImagePath,
                                                               TextureRect                  aArea = {},
                                                               bool aEnableSRgb = false) const = 0;

    //! Create a render texture by loading an image file from the disk.
    //!
    //! \note this overload of the function takes no `BatchingConfig` parameter,
    //!       so the default is assumed (batching disabled).
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
    virtual std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                               const Image&          image,
                                                               const TextureRect     aArea = {},
                                                               bool aEnableSRgb = false) const = 0;

    //! Create a render texture by loading it from an `Image` object.
    //!
    //! \note this overload of the function takes no `BatchingConfig` parameter,
    //!       so the default is assumed (batching disabled).
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

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Font                                                            //
    ///////////////////////////////////////////////////////////////////////////

    //! Create an empty font (it must be reset before it can be used).
    virtual std::unique_ptr<Font> createFont() const = 0;

    //! Create a font and load its data from a file on disk.
    //!
    //! The supported font formats are: TrueType, Type 1, CFF,
    //! OpenType, SFNT, X11 PCF, Windows FNT, BDF, PFR and Type 42.
    //! Note that this function knows nothing about the standard
    //! fonts installed on the user's system, thus you can't
    //! load them directly.
    //!
    //! \warning we cannot preload all the font data in this
    //! function, so the file has to remain accessible until
    //! the sf::Font object loads a new font or is destroyed.
    //!
    //! \throws on failure
    virtual std::unique_ptr<Font> createFont(const std::filesystem::path& aFilePath) const = 0;

    //! Create a font and load its data from a file in memory.
    //!
    //! The supported font formats are: TrueType, Type 1, CFF,
    //! OpenType, SFNT, X11 PCF, Windows FNT, BDF, PFR and Type 42.
    //! Note that this function knows nothing about the standard
    //! fonts installed on the user's system, thus you can't
    //! load them directly.
    //!
    //! \warning we cannot preload all the font data in this
    //! function, so the file has to remain accessible until
    //! the sf::Font object loads a new font or is destroyed.
    //!
    //! \throws on failure
    virtual std::unique_ptr<Font> createFont(const void* aData, PZInteger aByteCount) const = 0;

    //! Create a copy of one of the builtin fonts and return it.
    //! \note it's recommended to use `getBuiltinFont` instead, to share the font object between
    //!       components.
    virtual std::unique_ptr<Font> createBuiltinFont(BuiltInFont aFontChoice) const = 0;

    //! Get a reference to one of the builtin font objects that's managed by the system.
    virtual const Font& getBuiltinFont(BuiltInFont aFontChoice) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Text                                                            //
    ///////////////////////////////////////////////////////////////////////////

    //! Create a text with the given font but with an empty string.
    virtual std::unique_ptr<Text> createText(const Font& aFont) const = 0;

    //! Create a text with the given font, string and character size.
    virtual std::unique_ptr<Text> createText(
        const Font&        aFont,
        const std::string& aString,
        PZInteger          aCharacterSize = Text::DEFAULT_CHARACTER_SIZE) const = 0;

    //! Create a text with the given font, string and character size.
    virtual std::unique_ptr<Text> createText(
        const Font&          aFont,
        const UnicodeString& aString,
        PZInteger            aCharacterSize = Text::DEFAULT_CHARACTER_SIZE) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Shaders                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Tell whether or not the system supports certain shaders
    //!
    //! The returned value is a bitmask where each bit denotes whether the system
    //! supports the shader language related to that bit (see `Shader::Language`).
    //! If all bits are 0 (`Shader::Language::NONE`), the system doesn't support
    //! shaders.
    //!
    //! This function should always be called before using the shader features.
    //! Attempting to use unsupported shader features will always fail.
    virtual Shader::Language getAvailableShaders() const = 0;

    //! \brief Tell whether or not the system supports certain geometry shaders
    //!
    //! The returned value is a bitmask where each bit denotes whether the system
    //! supports geometry shaders in a shader language related to that bit (see
    //! `Shader::Language`). If all bits are 0 (`Shader::Language::NONE`), the system
    //! doesn't support geometry shaders.
    //! \note a bit in the value returned by this function can only be set to 1 if the
    //!       corresponding bit in the value returned by `getAvailableShaders` is also
    //!       set to 1, since shaders in general have to be supported in order for geometry
    //!       shaders to be supported as well.
    //!
    //! This function should always be called before using the geometry shader features.
    //! Attempting to use unsupported shader features will always fail.
    virtual Shader::Language getAvailableGeometryShaders() const = 0;

    //! \brief Create an empty GLSL shader
    //!
    //! You must `reset()` the shader manually before using it.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`,
    virtual std::unique_ptr<GLSLShader> createGLSLShader() const = 0;

    //! \brief Create a new GLSL shader and load its source code from a file
    //!
    //! This function loads only one of: vertex shader, fragment shader, or geometry shader
    //! (identified by the second argument). Its data will be loaded from a text file
    //! which must contain a valid shader in GLSL language.
    //!
    //! \param aFile Path of the text file containing the shader.
    //! \param aKind Kind of shader to load (vertex, geometry or fragment)
    //!
    //! \throws on failure.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`, `getAvailableGeometryShaders()`).
    virtual std::unique_ptr<GLSLShader> createGLSLShader(const std::filesystem::path& aFile,
                                                         Shader::Kind                 aKind) const = 0;

    //! \brief Create a new GLSL shader and load vertex/fragment source codes from files
    //!
    //! The data will be loaded from text files which must contain valid
    //! shaders in the GLSL language.
    //!
    //! \param aVertexShaderFile   Path of the text file containing the vertex shader.
    //! \param aFragmentShaderFile Path of the text file containing the fragment shader.
    //!
    //! \throws on failure.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`).
    virtual std::unique_ptr<GLSLShader> createGLSLShader(
        const std::filesystem::path& aVertexShaderFile,
        const std::filesystem::path& aFragmentShaderFile) const = 0;

    //! \brief Create a new GLSL shader and load vertex/fragment/geometry source codes from files
    //!
    //! The data will be loaded from text files which must contain valid
    //! shaders in the GLSL language.
    //!
    //! \param aVertexShaderFile   Path of the text file containing the vertex shader.
    //! \param aFragmentShaderFile Path of the text file containing the fragment shader.
    //! \param aGeometryShaderFile Path of the text file containing the geometry shader.
    //!
    //! \throws on failure.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`, `getAvailableGeometryShaders()`).
    virtual std::unique_ptr<GLSLShader> createGLSLShader(
        const std::filesystem::path& aVertexShaderFile,
        const std::filesystem::path& aFragmentShaderFile,
        const std::filesystem::path& aGeometryShaderFile) const = 0;

    //! \brief Create a new GLSL shader and load its source code from memory
    //!
    //! This function loads only one of: vertex shader, fragment shader, or geometry shader
    //! (identified by the second argument). Its data will be loaded from a string in memory
    //! which must contain a valid shader in GLSL language.
    //!
    //! \param aShaderSource String containing the source code of the shader to load.
    //! \param aKind Kind of shader to load (vertex, geometry or fragment).
    //!
    //! \throws on failure.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`, `getAvailableGeometryShaders()`).
    virtual std::unique_ptr<GLSLShader> createGLSLShader(const std::string& aShaderSource,
                                                         Shader::Kind       aKind) const = 0;

    //! \brief Create a new GLSL shader and load vertex/fragment source codes from memory
    //!
    //! The data will be loaded from strings in memory which must contain valid
    //! shaders in the GLSL language.
    //!
    //! \param aVertexShaderSource   String containing the source code of the vertex shader.
    //! \param aFragmentShaderSource String containing the source code of the fragment shader.
    //!
    //! \throws on failure.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`).
    virtual std::unique_ptr<GLSLShader> createGLSLShader(
        const std::string& aVertexShaderSource,
        const std::string& aFragmentShaderSource) const = 0;

    //! \brief Create a new GLSL shader and load vertex/fragment/geometry source codes from memory
    //!
    //! The data will be loaded from strings in memory which must contain valid
    //! shaders in the GLSL language.
    //!
    //! \param aVertexShaderSource   String containing the source code of the vertex shader.
    //! \param aFragmentShaderSource String containing the source code of the fragment shader.
    //! \param aGeometryShaderSource String containing the source code of the geometry shader.
    //!
    //! \throws on failure.
    //!
    //! \returns unique pointer to the shader, or `nullptr` if it is not supported (see
    //!          `getAvailableShaders()`, `getAvailableGeometryShaders()`).
    virtual std::unique_ptr<GLSLShader> createGLSLShader(
        const std::string& aVertexShaderSource,
        const std::string& aFragmentShaderSource,
        const std::string& aGeometryShaderSource) const = 0;
};

// MARK: System factories

//! Create a new graphics system, selecting by provider designator.
//!
//! \param aProvider designator of the graphics system provider.
//!
//! \throws TracedLogicError if an invalid designator is passed, or the selected provider is not
//!                          supported on the current machine (such as Direct3D on Mac).
std::shared_ptr<System> CreateGraphicsSystem(System::Provider aProvider);

//! Create a new graphics system, selecting by provider name.
//!
//! \param aSystemProviderName name of the graphics system provider. Must be one of the following:
//!                            - "SFML": Use SFML (Graphics API: OpenGL).
//!
//! \throws TracedLogicError if an invalid name is passed, or the selected provider is not
//!                          supported on the current machine (such as Direct3D on Mac).
std::shared_ptr<System> CreateGraphicsSystem(const char* aSystemProviderName);

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SYSTEM_HPP

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP
#define UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP

#include <Hobgoblin/UWGA/System.hpp>

#include <Hobgoblin/UWGA/Private/Draw_batching_decorator_fnl.hpp>

#include "Font_impl.hpp"
#include "Glsl_shader_impl.hpp"
#include "Image_impl.hpp"
#include "Render_texture_impl.hpp"
#include "Render_window_impl.hpp"
#include "Text_impl.hpp"
#include "Texture_impl.hpp"
#include "Transform_impl.hpp"
#include "View_impl.hpp"

#include "../BuiltinFonts/Helper.hpp"

#include <SFML/Graphics/Shader.hpp>

#include <array>
#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

namespace {
constexpr PZInteger DEFAULT_WIDTH  = 640;
constexpr PZInteger DEFAULT_HEIGHT = 480;

const BatchingConfig DEFAULT_BATCHING_CFG = {.strategy = BatchingConfig::Strategy::DISABLED};

template <class taCanvas, class... taArgs>
std::unique_ptr<taCanvas> CreateCanvas(const BatchingConfig& aBatchingConfig, taArgs&&... aArgs) {
    switch (aBatchingConfig.strategy) {
    case BatchingConfig::Strategy::DISABLED:
        return std::make_unique<taCanvas>(std::forward<taArgs>(aArgs)...);

    case BatchingConfig::Strategy::STRICT:
        HG_NOT_IMPLEMENTED();

    case BatchingConfig::Strategy::FAST_N_LOOSE:
        {
            using Type = detail::FastNLooseDrawBatchingDecorator<taCanvas>;
            return std::make_unique<Type>(std::forward<taArgs>(aArgs)...);
        }

    default:
        HG_UNREACHABLE("Invalid value for BatchingConfig::Strategy ({}).",
                       (int)aBatchingConfig.strategy);
    }
}
} // namespace

class SFMLSystemImpl : public System {
public:
    ~SFMLSystemImpl() override = default;

    Provider getProvider() const override {
        return Provider::SFML;
    }

    const char* getProviderName() const override {
        return "SFML";
    }

    GraphicsAPI getGraphicsAPI() const override {
        return GraphicsAPI::OPENGL;
    }

    const char* getGraphicsAPIName() const override {
        return "OpenGL";
    }

    // MARK: RenderWindow

    std::unique_ptr<RenderWindow> createRenderWindow(const BatchingConfig& aBatchingConfig,
                                                     PZInteger             aWidth,
                                                     PZInteger             aHeight,
                                                     WindowStyle           aStyle,
                                                     const UnicodeString&  aTitle,
                                                     bool                  aEnableSRgb) const override {
        return CreateCanvas<SFMLRenderWindowImpl>(aBatchingConfig,
                                                  SELF,
                                                  aWidth,
                                                  aHeight,
                                                  aStyle,
                                                  aTitle,
                                                  aEnableSRgb);
    }

    std::unique_ptr<RenderWindow> createRenderWindow(PZInteger            aWidth,
                                                     PZInteger            aHeight,
                                                     WindowStyle          aStyle,
                                                     const UnicodeString& aTitle,
                                                     bool                 aEnableSRgb) const override {
        return createRenderWindow(DEFAULT_BATCHING_CFG, aWidth, aHeight, aStyle, aTitle, aEnableSRgb);
    }

    // MARK: Image

    std::unique_ptr<Image> createImage() const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset();
        return image;
    }

    std::unique_ptr<Image> createImage(PZInteger aWidth,
                                       PZInteger aHeight,
                                       Color     aColor) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aWidth, aHeight, aColor);
        return image;
    }

    std::unique_ptr<Image> createImage(math::Vector2pz aSize, Color aColor) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aSize, aColor);
        return image;
    }

    std::unique_ptr<Image> createImage(PZInteger                      aWidth,
                                       PZInteger                      aHeight,
                                       NeverNull<const std::uint8_t*> aPixels) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aWidth, aHeight, aPixels);
        return image;
    }

    std::unique_ptr<Image> createImage(math::Vector2pz                aSize,
                                       NeverNull<const std::uint8_t*> aPixels) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aSize, aPixels);
        return image;
    }

    std::unique_ptr<Image> createImage(const std::filesystem::path& aImagePath) const override {
        auto image = std::make_unique<SFMLImageImpl>(SELF);
        image->reset(aImagePath);
        return image;
    }

    // MARK: Texture

    PZInteger getMaximumTextureSize() const override {
        return ToPz(sf::Texture::getMaximumSize());
    }

    std::unique_ptr<Texture> createTexture() const override {
        return std::make_unique<SFMLTextureImpl>(SELF, DEFAULT_WIDTH, DEFAULT_HEIGHT, false);
    }

    std::unique_ptr<Texture> createTexture(PZInteger aWidth,
                                           PZInteger aHeight,
                                           bool      aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aWidth, aHeight, aEnableSRgb);
    }

    std::unique_ptr<Texture> createTexture(math::Vector2pz aSize, bool aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aSize.x, aSize.y, aEnableSRgb);
    }

    std::unique_ptr<Texture> createTexture(const std::filesystem::path& aImagePath,
                                           TextureRect                  aArea,
                                           bool                         aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aImagePath, aArea, aEnableSRgb);
    }

    std::unique_ptr<Texture> createTexture(const Image&      aImage,
                                           const TextureRect aArea,
                                           bool              aEnableSRgb) const override {
        return std::make_unique<SFMLTextureImpl>(SELF, aImage, aArea, aEnableSRgb);
    }

    // MARK: RenderTexture

    std::unique_ptr<RenderTexture> createRenderTexture(
        const BatchingConfig& aBatchingConfig) const override //
    {
        return CreateCanvas<SFMLRenderTextureImpl>(aBatchingConfig,
                                                   SELF,
                                                   DEFAULT_WIDTH,
                                                   DEFAULT_HEIGHT,
                                                   false);
    }

    std::unique_ptr<RenderTexture> createRenderTexture() const override {
        return createRenderTexture(DEFAULT_BATCHING_CFG);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                       PZInteger             aWidth,
                                                       PZInteger             aHeight,
                                                       bool aEnableSRgb) const override {
        return CreateCanvas<SFMLRenderTextureImpl>(aBatchingConfig, SELF, aWidth, aHeight, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(PZInteger aWidth,
                                                       PZInteger aHeight,
                                                       bool      aEnableSRgb) const override {
        return createRenderTexture(DEFAULT_BATCHING_CFG, aWidth, aHeight, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                       math::Vector2pz       aSize,
                                                       bool aEnableSRgb) const override {
        return CreateCanvas<SFMLRenderTextureImpl>(aBatchingConfig, SELF, aSize.x, aSize.y, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(math::Vector2pz aSize,
                                                       bool            aEnableSRgb) const override {
        return createRenderTexture(DEFAULT_BATCHING_CFG, aSize.x, aSize.y, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig&        aBatchingConfig,
                                                       const std::filesystem::path& aImagePath,
                                                       TextureRect                  aArea,
                                                       bool aEnableSRgb) const override {
        return CreateCanvas<SFMLRenderTextureImpl>(aBatchingConfig,
                                                   SELF,
                                                   aImagePath,
                                                   aArea,
                                                   aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const std::filesystem::path& aImagePath,
                                                       TextureRect                  aArea,
                                                       bool aEnableSRgb) const override {
        return createRenderTexture(DEFAULT_BATCHING_CFG, aImagePath, aArea, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const BatchingConfig& aBatchingConfig,
                                                       const Image&          aImage,
                                                       const TextureRect     aArea,
                                                       bool aEnableSRgb) const override {
        return CreateCanvas<SFMLRenderTextureImpl>(aBatchingConfig, SELF, aImage, aArea, aEnableSRgb);
    }

    std::unique_ptr<RenderTexture> createRenderTexture(const Image&      aImage,
                                                       const TextureRect aArea,
                                                       bool              aEnableSRgb) const override {
        return createRenderTexture(DEFAULT_BATCHING_CFG, aImage, aArea, aEnableSRgb);
    }

    // MARK: View

    std::unique_ptr<View> createView() const override {
        return std::make_unique<SFMLViewImpl>(SELF);
    }

    // MARK: Transform

    std::unique_ptr<Transform> createTransform() const override {
        return std::make_unique<SFMLTransformImpl>(SELF);
    }

    // MARK: Font

    std::unique_ptr<Font> createFont() const override {
        return std::make_unique<SFMLFontImpl>(SELF);
    }

    std::unique_ptr<Font> createFont(const std::filesystem::path& aFilePath) const override {
        auto font = std::make_unique<SFMLFontImpl>(SELF);
        font->reset(aFilePath);
        return font;
    }

    std::unique_ptr<Font> createFont(const void* aData, PZInteger aByteCount) const override {
        auto font = std::make_unique<SFMLFontImpl>(SELF);
        font->reset(aData, aByteCount);
        return font;
    }

    std::unique_ptr<Font> createBuiltinFont(BuiltInFont aFontChoice) const override {
        const void* data      = nullptr;
        PZInteger   byteCount = 0;

        GetBuiltinFontData(aFontChoice, &data, &byteCount);
        HG_HARD_ASSERT(data != nullptr && byteCount > 0);

        auto font = std::make_unique<SFMLFontImpl>(SELF);
        font->reset(data, byteCount);
        return font;
    }

    const Font& getBuiltinFont(BuiltInFont aFontChoice) const override {
        auto& fontPtr = _cachedBuiltinFonts.at((std::size_t)aFontChoice);

        if (!fontPtr) {
            fontPtr = createBuiltinFont(aFontChoice);
        }

        return *fontPtr;
    }

    // MARK: Text

    std::unique_ptr<Text> createText(const Font& aFont) const override {
        auto text = std::make_unique<SFMLTextImpl>(SELF);
        text->setFont(aFont);
        return text;
    }

    std::unique_ptr<Text> createText(const Font&        aFont,
                                     const std::string& aString,
                                     PZInteger          aCharacterSize) const override {
        auto text = std::make_unique<SFMLTextImpl>(SELF);
        text->setFont(aFont);
        text->setString(aString);
        text->setCharacterSize(aCharacterSize);
        return text;
    }

    std::unique_ptr<Text> createText(const Font&          aFont,
                                     const UnicodeString& aString,
                                     PZInteger            aCharacterSize) const override {
        auto text = std::make_unique<SFMLTextImpl>(SELF);
        text->setFont(aFont);
        text->setString(aString);
        text->setCharacterSize(aCharacterSize);
        return text;
    }

    // MARK: Shader

    Shader::Language getAvailableShaders() const override {
        if (!sf::Shader::isAvailable()) {
            return Shader::Language::NONE;
        }
        return Shader::Language::GLSL;
    }

    Shader::Language getAvailableGeometryShaders() const override {
        if (!sf::Shader::isGeometryAvailable()) {
            return Shader::Language::NONE;
        }
        return Shader::Language::GLSL;
    }

    std::unique_ptr<GLSLShader> createGLSLShader() const override {
        if ((getAvailableShaders() & Shader::Language::GLSL) == Shader::Language::NONE) {
            return nullptr;
        }
        return std::make_unique<SFMLGLSLShaderImpl>(SELF);
    }

    std::unique_ptr<GLSLShader> createGLSLShader(const std::filesystem::path& aFile,
                                                 Shader::Kind                 aKind) const override {
        if (aKind == Shader::Kind::GEOMETRY) {
            if ((getAvailableGeometryShaders() & Shader::Language::GLSL) == Shader::Language::NONE) {
                return nullptr;
            }
        }

        auto shader = createGLSLShader();
        shader->reset(aFile, aKind);
        return shader;
    }

    std::unique_ptr<GLSLShader> createGLSLShader(
        const std::filesystem::path& aVertexShaderFile,
        const std::filesystem::path& aFragmentShaderFile) const override //
    {
        auto shader = createGLSLShader();
        shader->reset(aVertexShaderFile, aFragmentShaderFile);
        return shader;
    }

    std::unique_ptr<GLSLShader> createGLSLShader(
        const std::filesystem::path& aVertexShaderFile,
        const std::filesystem::path& aFragmentShaderFile,
        const std::filesystem::path& aGeometryShaderFile) const override //
    {
        if ((getAvailableGeometryShaders() & Shader::Language::GLSL) == Shader::Language::NONE) {
            return nullptr;
        }
        auto shader = createGLSLShader();
        shader->reset(aVertexShaderFile, aFragmentShaderFile, aGeometryShaderFile);
        return shader;
    }

    std::unique_ptr<GLSLShader> createGLSLShader(const std::string& aShaderSource,
                                                 Shader::Kind       aKind) const override {
        if (aKind == Shader::Kind::GEOMETRY) {
            if ((getAvailableGeometryShaders() & Shader::Language::GLSL) == Shader::Language::NONE) {
                return nullptr;
            }
        }

        auto shader = createGLSLShader();
        shader->reset(aShaderSource, aKind);
        return shader;
    }

    std::unique_ptr<GLSLShader> createGLSLShader(
        const std::string& aVertexShaderSource,
        const std::string& aFragmentShaderSource) const override {
        auto shader = createGLSLShader();
        shader->reset(aVertexShaderSource, aFragmentShaderSource);
        return shader;
    }

    std::unique_ptr<GLSLShader> createGLSLShader(
        const std::string& aVertexShaderSource,
        const std::string& aFragmentShaderSource,
        const std::string& aGeometryShaderSource) const override //
    {
        if ((getAvailableGeometryShaders() & Shader::Language::GLSL) == Shader::Language::NONE) {
            return nullptr;
        }

        auto shader = createGLSLShader();
        shader->reset(aVertexShaderSource, aFragmentShaderSource, aGeometryShaderSource);
        return shader;
    }

private:
    mutable std::array<std::unique_ptr<Font>, (std::size_t)BuiltInFont::COUNT> _cachedBuiltinFonts;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP

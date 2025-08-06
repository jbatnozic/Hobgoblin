// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_RENDER_TEXTURE_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_RENDER_TEXTURE_IMPL_HPP

#include <Hobgoblin/UWGA/Texture.hpp>

#include <Hobgoblin/Math/Vector.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "SFML_conversions.hpp"
#include "SFML_drawing_adapter.hpp"
#include "SFML_err.hpp"
#include "Texture_common_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLRenderTextureImpl : public SFMLTextureCommonImpl<sf::RenderTexture> {
public:
    SFMLRenderTextureImpl(const System& aSystem, PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb)
        : SFMLTextureCommonImpl<sf::RenderTexture>{aSystem}
        , _activeView{aSystem} //
    {
        reset(aWidth, aHeight, aEnableSRgb);
    }

    SFMLRenderTextureImpl(const System& aSystem, math::Vector2pz aSize, bool aEnableSRgb)
        : SFMLTextureCommonImpl<sf::RenderTexture>{aSystem}
        , _activeView{aSystem} //
    {
        reset(aSize.x, aSize.y, aEnableSRgb);
    }

    SFMLRenderTextureImpl(const System&                aSystem,
                          const std::filesystem::path& aImagePath,
                          TextureRect                  aArea,
                          bool                         aEnableSRgb)
        : SFMLTextureCommonImpl<sf::RenderTexture>{aSystem}
        , _activeView{aSystem} //
    {
        reset(aImagePath, aArea, aEnableSRgb);
    }

    SFMLRenderTextureImpl(const System&     aSystem,
                          const Image&      aImage,
                          const TextureRect aArea,
                          bool              aEnableSRgb)
        : SFMLTextureCommonImpl<sf::RenderTexture>{aSystem}
        , _activeView{aSystem} //
    {
        reset(aImage, aArea, aEnableSRgb);
    }

    void display() override {
        _texture->display();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Texture                                                               //
    ///////////////////////////////////////////////////////////////////////////

    // MARK: Texture/Reset

    void reset(PZInteger aWidth, PZInteger aHeight, bool aEnableSRgb) override {
        _texture.emplace();

        sf::ContextSettings contextSettings;
        contextSettings.sRgbCapable = aEnableSRgb;

        SFMLErrorCatcher sfErr;
        if (!_texture->create(static_cast<unsigned>(aWidth),
                              static_cast<unsigned>(aHeight),
                              contextSettings)) {
            HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
        }

        _activeView = SFMLViewImpl{
            _system,
            _texture->getDefaultView(),
            {0.0, 0.0}
        };
        _activeViewAnchor = {0.0, 0.0};
    }

    void reset(const std::filesystem::path& aPath, TextureRect aArea, bool aEnableSRgb) override {
        HG_NOT_IMPLEMENTED();
    }

    void reset(const Image& aImage, TextureRect aArea, bool aEnableSRgb) override {
        HG_NOT_IMPLEMENTED();
    }

    // MARK: Texture/Updating

    void update(const std::uint8_t* aPixels, PZInteger aPixelsByteCount) override {
        HG_NOT_IMPLEMENTED();
    }

    void update(const std::uint8_t* aPixels,
                PZInteger           aWidth,
                PZInteger           aHeight,
                PZInteger           aX,
                PZInteger           aY) override {
        HG_NOT_IMPLEMENTED();
    }

    void update(const Texture& aTexture) override {
        update(aTexture, 0, 0);
    }

    void update(const Texture& aTexture, PZInteger aX, PZInteger aY) override {
        assert(&aTexture.getSystem() == &_system);

        HG_NOT_IMPLEMENTED();
    }

    void update(const Image& aImage) override {
        update(aImage, 0, 0);
    }

    void update(const Image& aImage, PZInteger aX, PZInteger aY) override {
        assert(&aImage.getSystem() == &_system);

        HG_NOT_IMPLEMENTED();
    }

    void update(const Window& aWindow) override {
        update(aWindow, 0, 0);
    }

    void update(const Window& aWindow, PZInteger aX, PZInteger aY) override {
        assert(&aWindow.getSystem() == &_system);

        HG_NOT_IMPLEMENTED();

        HG_UNREACHABLE("Incompatible Window implementation provided.");
    }

    // MARK: Texture/Miscellaneous

    [[nodiscard]] std::unique_ptr<Image> copyToImage() const override {
        return std::make_unique<SFMLImageImpl>(_system, _texture->getTexture());
    }

    [[nodiscard]] std::optional<std::int64_t> getNativeHandle() const override {
        const auto openglHandle = _texture->getTexture().getNativeHandle();
        if (openglHandle == 0) {
            return std::nullopt;
        }
        return static_cast<std::int64_t>(openglHandle);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Canvas                                                                //
    ///////////////////////////////////////////////////////////////////////////

    // MARK: Canvas/Views

    void setView(const View& aView) override {
        assert(&aView.getSystem() == &getSystem());

        const auto& viewImpl = static_cast<const SFMLViewImpl&>(aView);
        _texture->setView(viewImpl.getUnderlyingView());
        _activeView       = viewImpl;
        _activeViewAnchor = aView.getAnchor();
    }

    void setDefaultView() override {
        _texture->setView(_texture->getDefaultView());
    }

    const View& getView() const override {
        return _activeView;
    }

    math::Rectangle<int> viewportToPixels(const View& aView) const override {
        const auto size = math::VectorCast<float>(getSize());

        const auto viewport = _activeView.getViewport();

        return {static_cast<int>(0.5f + size.x * viewport.x),
                static_cast<int>(0.5f + size.y * viewport.y),
                static_cast<int>(0.5f + size.x * viewport.w),
                static_cast<int>(0.5f + size.y * viewport.h)};
    }

    const sf::View& getDefaultView() const {
        return _texture->getDefaultView();
    }

    // MARK: Canvas/Drawing

    void clear(Color aColor) override {
        _texture->clear(ToSf(aColor));
    }

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aRenderStates) override {
        SFMLDrawingAdapter drawingAdapter{_system, *_texture, _defaultRenderStates, _activeViewAnchor};
        drawingAdapter.draw(aVertices, aVertexCount, aPrimitiveType, aAnchor, aRenderStates);
    }

    void flush() override {
        /* Nothing to do (there is no batching in SFML). */
    }

private:
    sf::RenderStates _defaultRenderStates = sf::RenderStates::Default;
    SFMLViewImpl     _activeView;
    math::Vector2d   _activeViewAnchor;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_RENDER_TEXTURE_IMPL_HPP

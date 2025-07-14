// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP
#define UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP

#include <Hobgoblin/UWGA/System.hpp>

#include "Image_impl.hpp"
#include "Render_window_impl.hpp"
#include "Transform_impl.hpp"
#include "Vertex_array_impl.hpp"
#include "View_impl.hpp"

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLSystemImpl : public System {
public:
    ~SFMLSystemImpl() override = default;

    // MARK: RenderWindow

    std::unique_ptr<RenderWindow> createRenderWindow(PZInteger            aWidth,
                                                     PZInteger            aHeight,
                                                     WindowStyle          aStyle,
                                                     const UnicodeString& aTitle) const override {
        return std::make_unique<SFMLRenderWindowImpl>(SELF, aWidth, aHeight, aStyle, aTitle);
    }

    // MARK: Image

    std::unique_ptr<Image> createImage() const override {
        return {}; // TODO
    }

    std::unique_ptr<Image> createImage(PZInteger aWidth,
                                       PZInteger aHeight,
                                       Color     aColor) const override {
        return {}; // TODO
    }

    std::unique_ptr<Image> createImage(math::Vector2pz aSize, Color aColor) const override {
        return {}; // TODO
    }

    std::unique_ptr<Image> createImage(PZInteger                      aWidth,
                                       PZInteger                      aHeight,
                                       NeverNull<const std::uint8_t*> aPixels) const override {
        return {}; // TODO
    }

    std::unique_ptr<Image> createImage(math::Vector2pz                aSize,
                                       NeverNull<const std::uint8_t*> aPixels) const override {
        return {}; // TODO
    }

    std::unique_ptr<Image> createImage(const std::filesystem::path& aImagePath) const override {
        return {}; // TODO
    }

    // MARK: Texture

    // TODO

    // MARK: RenderTexture

    // TODO

    // MARK: View

    std::unique_ptr<View> createView() const override {
        return std::make_unique<SFMLViewImpl>(SELF);
    }

    std::unique_ptr<View> createDefaultView(const RenderWindow& aRenderWindow) const override {
        assert(&aRenderWindow.getSystem() == this);
        return std::make_unique<SFMLViewImpl>(
            SELF,
            static_cast<const SFMLRenderWindowImpl&>(aRenderWindow).getDefaultView(),
            math::Vector2d{0.0, 0.0});
    }

    // MARK: Transform

    std::unique_ptr<Transform> createTransform() const override {
        return std::make_unique<SFMLTransformImpl>(SELF);
    }

    // MARK: VertexArray

    std::unique_ptr<VertexArray> createVertexArray(PZInteger aSize) const override {
        // return std::make_unique
        return {};
    }
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_SYSTEM_HPP

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TEXTURE_COMMON_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_TEXTURE_COMMON_IMPL_HPP

#include <Hobgoblin/UWGA/Texture.hpp>

#include <SFML/Graphics/Texture.hpp>

#include "SFML/Image_impl.hpp"
#include "SFML/Render_window_impl.hpp"
#include "SFML/Texture_provider.hpp"

#include <optional>
#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

/*
 * Class hierarchy:
 *
 *    Texture        Canvas
 *       \             /
 *        \           /
 *         \         /
 *        RenderTexture
 *              |
 *              |
 *      SFMLTextureProvider
 *              |
 *              |
 *     SFMLTextureCommonImpl<>
 *        /            \
 *       /              \
 *      /                \
 * TextureImpl      RenderTextureImpl
 */
template <class taUnderlying>
class SFMLTextureCommonImpl : public SFMLTextureProvider {
public:
    SFMLTextureCommonImpl(const System& aSystem);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SFMLTextureProvider                                             //
    ///////////////////////////////////////////////////////////////////////////

    sf::Texture& getUnderlyingTexture() override;

    const sf::Texture& getUnderlyingTexture() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Texture                                                         //
    ///////////////////////////////////////////////////////////////////////////

    // Miscellaneous

    math::Vector2pz getSize() const override;

    void setSmooth(bool aSmooth) override;

    bool isSmooth() const override;

    bool isSrgb() const override;

    void setRepeated(bool aRepeated) override;

    bool isRepeated() const override;

    [[nodiscard]] bool generateMipmap() override;

    void swap(Texture& aOther) override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Canvas                                                          //
    ///////////////////////////////////////////////////////////////////////////

    // math::Vector2pz getSize() const override; -- see section 'Texture'

    // Viwes

    void setView(const View& aView) override;

    const View& getView() const override;

    void setDefaultView() override;

    std::unique_ptr<View> createDefaultView() const override;

    math::Rectangle<int> viewportToPixels(const View& aView) const override;

    // Drawing

    void clear(Color aColor) override;

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates) override;

    void flush() override;

    const PerformanceCounters& getPerformanceCounters() const override;

    void resetPerformanceCounters() override;

    PerformanceCounters getAndResetPerformanceCounters() override;

protected:
    const System&               _system;
    std::optional<taUnderlying> _texture;
};

///////////////////////////////////////////////////////////////////////////
// MARK: Method definitions                                              //
///////////////////////////////////////////////////////////////////////////

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::SFMLTextureCommonImpl(const System& aSystem)
    : _system{aSystem} {}

template <class taUnderlying>
sf::Texture& SFMLTextureCommonImpl<taUnderlying>::getUnderlyingTexture() {
    if constexpr (std::is_same_v<taUnderlying, sf::RenderTexture>) {
        HG_UNREACHABLE();
    } else {
        return *_texture;
    }
}

template <class taUnderlying>
const sf::Texture& SFMLTextureCommonImpl<taUnderlying>::getUnderlyingTexture() const {
    if constexpr (std::is_same_v<taUnderlying, sf::RenderTexture>) {
        return _texture->getTexture();
    } else {
        return *_texture;
    }
}

// ===== Element =====

template <class taUnderlying>
const System& SFMLTextureCommonImpl<taUnderlying>::getSystem() const {
    return _system;
}

// ===== Texture =====

// Miscellaneous

template <class taUnderlying>
math::Vector2pz SFMLTextureCommonImpl<taUnderlying>::getSize() const {
    const auto size = _texture->getSize();
    return {ToPz(size.x), ToPz(size.y)};
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setSmooth(bool aSmooth) {
    _texture->setSmooth(aSmooth);
}

template <class taUnderlying>
bool SFMLTextureCommonImpl<taUnderlying>::isSmooth() const {
    return _texture->isSmooth();
}

template <class taUnderlying>
bool SFMLTextureCommonImpl<taUnderlying>::isSrgb() const {
    return _texture->isSrgb();
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setRepeated(bool aRepeated) {
    _texture->setRepeated(aRepeated);
}

template <class taUnderlying>
bool SFMLTextureCommonImpl<taUnderlying>::isRepeated() const {
    return _texture->isRepeated();
}

template <class taUnderlying>
[[nodiscard]] bool SFMLTextureCommonImpl<taUnderlying>::generateMipmap() {
    return _texture->generateMipmap();
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::swap(Texture& aOther) {
    assert(&aOther.getSystem() == &_system);

    // auto& textureProvider = static_cast<SFMLTextureProvider&>(aOther);
    // _texture->swap(textureProvider.getUnderlyingTexture());

    HG_NOT_IMPLEMENTED();

    // TODO: If both are regular sf::Texture - use code above; otherwise, it gets complicated
}

// ===== Canvas =====

// Viwes

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setView(const View& aView) {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
const View& SFMLTextureCommonImpl<taUnderlying>::getView() const {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::setDefaultView() {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
std::unique_ptr<View> SFMLTextureCommonImpl<taUnderlying>::createDefaultView() const {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
math::Rectangle<int> SFMLTextureCommonImpl<taUnderlying>::viewportToPixels(const View& aView) const {
    HG_UNREACHABLE("Illegal operation!");
}

// Drawing

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::clear(Color aColor) {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::draw(const Vertex*       aVertices,
                                               PZInteger           aVertexCount,
                                               PrimitiveType       aPrimitiveType,
                                               math::Vector2d      aAnchor,
                                               const RenderStates& aStates) {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::flush() {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
const SFMLTextureCommonImpl<taUnderlying>::PerformanceCounters& SFMLTextureCommonImpl<
    taUnderlying>::getPerformanceCounters() const {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
void SFMLTextureCommonImpl<taUnderlying>::resetPerformanceCounters() {
    HG_UNREACHABLE("Illegal operation!");
}

template <class taUnderlying>
SFMLTextureCommonImpl<taUnderlying>::PerformanceCounters SFMLTextureCommonImpl<
    taUnderlying>::getAndResetPerformanceCounters() {
    HG_UNREACHABLE("Illegal operation!");
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TEXTURE_COMMON_IMPL_HPP

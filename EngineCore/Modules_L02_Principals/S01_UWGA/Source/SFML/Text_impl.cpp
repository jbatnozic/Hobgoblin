// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Text_impl.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Font.hpp>
#include <Hobgoblin/UWGA/System.hpp>
#include <Hobgoblin/UWGA/Text.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>

#include "Font_impl.hpp"
#include "SFML_conversions.hpp"
#include "Texture_wrapper.hpp"

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

SFMLTextImpl::SFMLTextImpl(const System& aSystem)
    : Text{aSystem} {}

const System& SFMLTextImpl::getSystem() const {
    return getTransform().getSystem();
}

// MARK: String

void SFMLTextImpl::setString(const std::string& aString) {
    _text.setString(aString);
}

void SFMLTextImpl::setString(const UnicodeString& aString) {
    sf::String sfStr;
    hobgoblin::detail::StoreUStringInSfString(aString, &sfStr);
    _text.setString(sfStr);
}

std::string SFMLTextImpl::getString() const {
    auto utf8str = _text.getString().toUtf8();

    std::string result;
    static_assert(sizeof(decltype(utf8str)::value_type) == sizeof(decltype(result)::value_type));

    result = std::move(reinterpret_cast<std::string&>(utf8str));
    return result;
}

UnicodeString SFMLTextImpl::getUnicodeString() const {
    const auto&   sfStr = _text.getString();
    UnicodeString result;
    hobgoblin::detail::LoadUStringFromSfString(result, &sfStr);
    return result;
}

// MARK: Font

void SFMLTextImpl::setFont(const Font& aFont) {
    assert(&aFont.getSystem() == &getSystem());

    _font = &aFont;

    _text.setFont(static_cast<const SFMLFontImpl*>(_font)->getUnderlyingFont());
}

const Font& SFMLTextImpl::getFont() const {
    HG_HARD_ASSERT(_font != nullptr);
    return *_font;
}

// MARK: Style

void SFMLTextImpl::setStyle(std::uint32_t aStyle) {
    // clang-format off
    static_assert(static_cast<int>(REGULAR)       == static_cast<int>(sf::Text::Regular));
    static_assert(static_cast<int>(BOLD)          == static_cast<int>(sf::Text::Bold));
    static_assert(static_cast<int>(ITALIC)        == static_cast<int>(sf::Text::Italic));
    static_assert(static_cast<int>(UNDERLINED)    == static_cast<int>(sf::Text::Underlined));
    static_assert(static_cast<int>(STRIKETHROUGH) == static_cast<int>(sf::Text::StrikeThrough));
    // clang-format on

    _text.setStyle(aStyle);
}

std::uint32_t SFMLTextImpl::getStyle() const {
    return _text.getStyle();
}

// MARK: Color

void SFMLTextImpl::setFillColor(Color aColor) {
    _text.setFillColor(ToSf(aColor));
}

void SFMLTextImpl::setOutlineColor(Color aColor) {
    _text.setOutlineColor(ToSf(aColor));
}

Color SFMLTextImpl::getFillColor() const {
    return ToHg(_text.getFillColor());
}

Color SFMLTextImpl::getOutlineColor() const {
    return ToHg(_text.getOutlineColor());
}

// MARK: Outline

void SFMLTextImpl::setOutlineThickness(float aThickness) {
    _text.setOutlineThickness(aThickness);
}

float SFMLTextImpl::getOutlineThickness() const {
    return _text.getOutlineThickness();
}

// MARK: Shape

void SFMLTextImpl::setCharacterSize(PZInteger aSize) {
    _text.setCharacterSize(static_cast<unsigned int>(aSize));
}

PZInteger SFMLTextImpl::getCharacterSize() const {
    return static_cast<PZInteger>(_text.getCharacterSize());
}

void SFMLTextImpl::setLineSpacing(float aSpacingFactor) {
    return _text.setLineSpacing(aSpacingFactor);
}

float SFMLTextImpl::getLineSpacing() const {
    return _text.getLineSpacing();
}

void SFMLTextImpl::setLetterSpacing(float aSpacingFactor) {
    _text.setLetterSpacing(aSpacingFactor);
}

float SFMLTextImpl::getLetterSpacing() const {
    return _text.getLetterSpacing();
}

math::Vector2f SFMLTextImpl::findCharacterPos(PZInteger aIndex) const {
    return ToHg(_text.findCharacterPos(pztos(aIndex)));
}

// MARK: Bounds

math::Rectangle<float> SFMLTextImpl::getLocalBounds() const {
    return ToHg(_text.getLocalBounds());
}

math::Rectangle<float> SFMLTextImpl::getGlobalBounds() const {
    return ToHg(_text.getGlobalBounds());
}

// MARK: Draw

/*
Original SFML draw impl:
    if (m_font)
    {
        ensureGeometryUpdate();

        states.transform *= getTransform();
        states.texture = &m_font->getTexture(m_characterSize);

        // Only draw the outline if there is something to draw
        if (m_outlineThickness != 0)
            target.draw(m_outlineVertices, states);

        target.draw(m_vertices, states);
    }
*/

void SFMLTextImpl::drawOnto(Canvas& aCanvas, const RenderStates& aRenderStates) const {
    assert(&aCanvas.getSystem() == &getSystem());

    if (_font) {
        _text.ensureGeometryUpdate();

        RenderStates states{aRenderStates};

        if (!states.transform) {
            states.transform = &getTransform();
        } else {
            if (!_drawTransform) {
                _drawTransform = states.transform->clone();
            } else {
                _drawTransform->setToCopyOf(*states.transform);
            }
            _drawTransform->combine(getTransform());
            states.transform = _drawTransform.get();
        }

        const SFMLTextureWrapper texWrap{
            getSystem(),
            static_cast<const SFMLFontImpl*>(_font)->getUnderlyingFont().getTexture(
                _text.getCharacterSize())};

        states.texture = &texWrap;

        if (_text.getOutlineThickness() != 0.f) {
            const auto& sfmlOutlineVertexArray = _text.m_outlineVertices;
            aCanvas.draw(reinterpret_cast<const Vertex*>(sfmlOutlineVertexArray.m_vertices.data()),
                         stopz(sfmlOutlineVertexArray.getVertexCount()),
                         ToHg(sfmlOutlineVertexArray.getPrimitiveType()),
                         getAnchor(),
                         states);
        }

        const auto& sfmlVertexArray = _text.m_vertices;
        aCanvas.draw(reinterpret_cast<const Vertex*>(sfmlVertexArray.m_vertices.data()),
                     stopz(sfmlVertexArray.getVertexCount()),
                     ToHg(sfmlVertexArray.getPrimitiveType()),
                     getAnchor(),
                     states);
    }
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

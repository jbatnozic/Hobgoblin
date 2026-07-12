// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFMLTEXT_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFMLTEXT_IMPL_HPP

#include <Hobgoblin/UWGA/Text.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>

namespace jbatnozic::hobgoblin::uwga {
class SFMLTextImpl;
}

// Of all the terrible hacks that can be found in Hobgoblin, this HAS to be the worst one -
// the amount of Undefined Behaviour this relies on is astounding.
// However... It does actually work, and since SFML isn't likely to change this code very
// much, it will probably keep working for the foreeable future.
// Still, it would be good to refactor it into something more sane and less fragile, eventually.
// TODO(think of a better way to implement SFMLTextImpl)
#define private private: friend class ::jbatnozic::hobgoblin::uwga::SFMLTextImpl; private
#include <SFML/Graphics/Text.hpp>
#undef private

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Font;

class SFMLTextImpl : public Text {
public:
    SFMLTextImpl(const System& aSystem);

    ~SFMLTextImpl() override = default;

    const System& getSystem() const override;

    // MARK: String

    void setString(const std::string& aString) override;

    void setString(const UnicodeString& aString) override;

    std::string getString() const override;

    UnicodeString getUnicodeString() const override;

    // MARK: Font

    void setFont(const Font& aFont) override;

    const Font& getFont() const override;

    // MARK: Style

    void setStyle(std::uint32_t aStyle) override;

    std::uint32_t getStyle() const override;

    // MARK: Color

    void setFillColor(Color aColor) override;

    void setOutlineColor(Color aColor) override;

    Color getFillColor() const override;

    Color getOutlineColor() const override;

    // MARK: Outline

    void setOutlineThickness(float aThickness) override;

    float getOutlineThickness() const override;

    // MARK: Shape

    void setCharacterSize(PZInteger aSize) override;

    PZInteger getCharacterSize() const override;

    void setLineSpacing(float aSpacingFactor) override;

    float getLineSpacing() const override;

    void setLetterSpacing(float aSpacingFactor) override;

    float getLetterSpacing() const override;

    math::Vector2f findCharacterPos(PZInteger aIndex) const override;

    // MARK: Bounds

    math::Rectangle<float> getLocalBounds() const override;

    math::Rectangle<float> getGlobalBounds() const override;

    // MARK: Draw

    void drawOnto(Canvas&             aCanvas,
                  const RenderStates& aRenderStates = RENDER_STATES_DEFAULT) const override;

private:
    const Font* _font = nullptr;
    sf::Text    _text;

    //! Cached Transform object to use to combine the passed transform with this instance's Transform
    //! during drawing in case a non-null Transform is passed as a part of render states
    mutable std::unique_ptr<Transform> _drawTransform;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TEXT_HPP

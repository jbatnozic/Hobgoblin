// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_FONT_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_FONT_IMPL_HPP

#include <Hobgoblin/UWGA/Font.hpp>

#include <SFML/Graphics/Font.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLFontImpl : public Font {
public:
    SFMLFontImpl(const System& aSystem);

    const System& getSystem() const override;

    const sf::Font& getUnderlyingFont() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RESETING                                                        //
    ///////////////////////////////////////////////////////////////////////////

    void reset(const std::filesystem::path& aFile) override;

    void reset(const void* aData, PZInteger aByteCount) override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: INFO                                                            //
    ///////////////////////////////////////////////////////////////////////////

    const std::string& getFontFamily() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: GLYPHS                                                          //
    ///////////////////////////////////////////////////////////////////////////

    bool hasGlyph(std::uint32_t aCodePoint) const override;

    Glyph getGlyph(std::uint32_t aCodePoint,
                   PZInteger     aCharacterSize,
                   bool          aBold,
                   float         aOutlineThickness = 0) const override;

    float getKerning(std::uint32_t aFirst,
                     std::uint32_t aSecond,
                     PZInteger     aCharacterSize,
                     bool          aBold = false) const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: UNDERLINE                                                       //
    ///////////////////////////////////////////////////////////////////////////

    float getUnderlinePosition(PZInteger aCharacterSize) const override;

    float getUnderlineThickness(PZInteger aCharacterSize) const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: MISCELLANEOUS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    float getLineSpacing(PZInteger aCharacterSize) const override;

    void setSmooth(bool aSmooth) override;

    bool isSmooth() const override;

private:
    const System& _system;
    sf::Font      _font;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_FONT_IMPL_HPP

// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
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
//
////////////////////////////////////////////////////////////

#ifndef UHOBGOBLIN_UWGA_TEXT_HPP
#define UHOBGOBLIN_UWGA_TEXT_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Transformable.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Font;
class System;

//!/ \brief Graphical text that can be drawn to a render target
class Text
    : public Element
    , public Drawable
    , public Transformable {
public:
    explicit Text(const System& aSystem);

    //! Virtual destructor,
    virtual ~Text() = default;

    //! \brief Enumeration of the string drawing styles
    enum Style {
        REGULAR       = 0,      //!!< Regular characters, no style
        BOLD          = 1 << 0, //!!< Bold characters
        ITALIC        = 1 << 1, //!!< Italic characters
        UNDERLINED    = 1 << 2, //!!< Underlined characters
        STRIKETHROUGH = 1 << 3  //!!< Strike through characters
    };

    static constexpr PZInteger DEFAULT_CHARACTER_SIZE = 30;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: STRING                                                          //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Set the text's string from a standard string, which will be interpreted as UTF-8.
    virtual void setString(const std::string& aString) = 0;

    //! \brief Set the text's string from a unicode string.
    virtual void setString(const UnicodeString& aString) = 0;

    //! \brief Get the text's string as a UTF-8 encoded standard string.
    virtual std::string getString() const = 0;

    //! \brief Get the text's string as a unicode string.
    virtual UnicodeString getUnicodeString() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: FONT                                                            //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Set the text's font
    //!
    //! The \a font argument refers to a font that must
    //! exist as long as the text uses it. Indeed, the text
    //! doesn't store its own copy of the font, but rather keeps
    //! a pointer to the one that you passed to this function.
    //! If the font is destroyed and the text tries to
    //! use it, the behavior is undefined.
    //!
    //! \param aFont New font
    virtual void setFont(const Font& aFont) = 0;

    //! \brief Disallow setting from a temporary font.
    void setFont(Font&& aFont) = delete;

    //! \brief Get the text's font
    //!
    //! The returned reference is const, which means that you
    //! cannot modify the font when you get it from this function.
    //!
    //! \return Reference to the text's font
    //!
    //! \see setFont
    virtual const Font& getFont() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: STYLE                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Set the text's style.
    //!
    //! You can pass a combination of one or more styles, for
    //! example uwga::Text::BOLD | uwga::Text::ITALIC.
    //! The default style is uwga::Text::REGULAR.
    //!
    //! \param aStyle New style
    virtual void setStyle(std::uint32_t aStyle) = 0;

    //! \brief Get the text's style.
    virtual std::uint32_t getStyle() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: COLOR                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Set the fill color of the text
    //!
    //! By default, the text's fill color is opaque white.
    //! Setting the fill color to a transparent color with an outline
    //! will cause the outline to be displayed in the fill area of the text.
    //!
    //! \param aColor New fill color of the text
    virtual void setFillColor(Color aColor) = 0;

    //! \brief Set the outline color of the text.
    //!
    //! By default, the text's outline color is opaque black.
    //!
    //! \param aColor New outline color of the text.
    virtual void setOutlineColor(Color aColor) = 0;

    //! \brief Get the fill color of the text.
    virtual Color getFillColor() const = 0;

    //! \brief Get the outline color of the text.
    virtual Color getOutlineColor() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: OUTLINE                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Set the thickness of the text's outline.
    //!
    //! By default, the outline thickness is 0.
    //!
    //! Be aware that using a negative value for the outline
    //! thickness will cause distorted rendering.
    //!
    //! \param aThickness New outline thickness, in pixels.
    virtual void setOutlineThickness(float aThickness) = 0;

    //! \brief Get the outline thickness of the text.
    //!
    //! \return Outline thickness of the text, in pixels.
    virtual float getOutlineThickness() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SHAPE                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Set the character size
    //!
    //! The default size is 30.
    //!
    //! Note that if the used font is a bitmap font, it is not
    //! scalable, thus not all requested sizes will be available
    //! to use. This needs to be taken into consideration when
    //! setting the character size. If you need to display text
    //! of a certain size, make sure the corresponding bitmap
    //! font that supports that size is used.
    //!
    //! \param size New character size, in pixels
    //!
    //! \see getCharacterSize
    virtual void setCharacterSize(PZInteger aSize) = 0;

    //! \brief Get the character size
    //!
    //! \return Size of the characters, in pixels
    //!
    //! \see setCharacterSize
    virtual PZInteger getCharacterSize() const = 0;

    //! \brief Set the line spacing factor
    //!
    //! The default spacing between lines is defined by the font.
    //! This method enables you to set a factor for the spacing
    //! between lines. By default the line spacing factor is 1.
    //!
    //! \param spacingFactor New line spacing factor
    //!
    //! \see getLineSpacing
    virtual void setLineSpacing(float aSpacingFactor) = 0;

    //! \brief Get the size of the line spacing factor
    //!
    //! \return Size of the line spacing factor
    //!
    //! \see setLineSpacing
    virtual float getLineSpacing() const = 0;

    //! \brief Set the letter spacing factor
    //!
    //! The default spacing between letters is defined by the font.
    //! This factor doesn't directly apply to the existing
    //! spacing between each character, it rather adds a fixed
    //! space between them which is calculated from the font
    //! metrics and the character size.
    //! Note that factors below 1 (including negative numbers) bring
    //! characters closer to each other.
    //! By default the letter spacing factor is 1.
    //!
    //! \param spacingFactor New letter spacing factor
    //!
    //! \see getLetterSpacing
    virtual void setLetterSpacing(float aSpacingFactor) = 0;

    //! \brief Get the size of the letter spacing factor
    //!
    //! \return Size of the letter spacing factor
    //!
    //! \see setLetterSpacing
    virtual float getLetterSpacing() const = 0;

    //! \brief Return the position of the \a index-th character
    //!
    //! This function computes the visual position of a character
    //! from its index in the string. The returned position is
    //! in global coordinates (translation, rotation, scale and
    //! origin are applied).
    //! If \a index is out of range, the position of the end of
    //! the string is returned.
    //!
    //! \param index Index of the character
    //!
    //! \return Position of the character
    virtual math::Vector2f findCharacterPos(PZInteger aIndex) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: BOUNDS                                                          //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Get the local bounding rectangle of the entity
    //!
    //! The returned rectangle is in local coordinates, which means
    //! that it ignores the transformations (translation, rotation,
    //! scale, ...) that are applied to the entity.
    //! In other words, this function returns the bounds of the
    //! entity in the entity's coordinate system.
    //!
    //! \return Local bounding rectangle of the entity
    virtual math::Rectangle<float> getLocalBounds() const = 0;

    //! \brief Get the global bounding rectangle of the entity
    //!
    //! The returned rectangle is in global coordinates, which means
    //! that it takes into account the transformations (translation,
    //! rotation, scale, ...) that are applied to the entity.
    //! In other words, this function returns the bounds of the
    //! text in the global 2D world's coordinate system.
    //!
    //! \return Global bounding rectangle of the entity
    virtual math::Rectangle<float> getGlobalBounds() const = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TEXT_HPP

////////////////////////////////////////////////////////////
/// \class sf::Text
/// \ingroup graphics
///
/// sf::Text is a drawable class that allows to easily display
/// some text with custom style and color on a render target.
///
/// It inherits all the functions from sf::Transformable:
/// position, rotation, scale, origin. It also adds text-specific
/// properties such as the font to use, the character size,
/// the font style (bold, italic, underlined and strike through), the
/// text color, the outline thickness, the outline color, the character
/// spacing, the line spacing and the text to display of course.
/// It also provides convenience functions to calculate the
/// graphical size of the text, or to get the global position
/// of a given character.
///
/// sf::Text works in combination with the sf::Font class, which
/// loads and provides the glyphs (visual characters) of a given font.
///
/// The separation of sf::Font and sf::Text allows more flexibility
/// and better performances: indeed a sf::Font is a heavy resource,
/// and any operation on it is slow (often too slow for real-time
/// applications). On the other side, a sf::Text is a lightweight
/// object which can combine the glyphs data and metrics of a sf::Font
/// to display any text on a render target.
///
/// It is important to note that the sf::Text instance doesn't
/// copy the font that it uses, it only keeps a reference to it.
/// Thus, a sf::Font must not be destructed while it is
/// used by a sf::Text (i.e. never write a function that
/// uses a local sf::Font instance for creating a text).
///
/// See also the note on coordinates and undistorted rendering in sf::Transformable.
///
/// Usage example:
/// \code
/// //! Declare and load a font
/// sf::Font font;
/// if (!font.loadFromFile("arial.ttf"))
/// {
///     //! Handle error...
/// }
///
/// //! Create a text
/// sf::Text text(font, "hello");
/// text.setCharacterSize(30);
/// text.setStyle(sf::Text::Bold);
/// text.setFillColor(sf::Color::Red);
///
/// //! Draw it
/// window.draw(text);
/// \endcode
///
/// \see sf::Font, sf::Transformable
///
////////////////////////////////////////////////////////////

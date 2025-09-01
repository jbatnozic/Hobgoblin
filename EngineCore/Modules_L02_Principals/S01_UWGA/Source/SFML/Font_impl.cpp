// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented{}
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

#include "SFML/Font_impl.hpp"

#include "SFML/SFML_conversions.hpp"
#include "SFML/SFML_err.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

SFMLFontImpl::SFMLFontImpl(const System& aSystem)
    : _system{aSystem} {}

const System& SFMLFontImpl::getSystem() const {
    return _system;
}

const sf::Font& SFMLFontImpl::getUnderlyingFont() const {
    return _font;
}

///////////////////////////////////////////////////////////////////////////
// MARK: RESETING                                                        //
///////////////////////////////////////////////////////////////////////////

void SFMLFontImpl::reset(const std::filesystem::path& aFile) {
    SFMLErrorCatcher sfErr;
    if (!_font.loadFromFile(FilesystemPathToSfPath(aFile))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

void SFMLFontImpl::reset(const void* aData, PZInteger aByteCount) {
    SFMLErrorCatcher sfErr;
    if (!_font.loadFromMemory(aData, pztos(aByteCount))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

///////////////////////////////////////////////////////////////////////////
// MARK: INFO                                                            //
///////////////////////////////////////////////////////////////////////////

const std::string& SFMLFontImpl::getFontFamily() const {
    return _font.getInfo().family;
}

///////////////////////////////////////////////////////////////////////////
// MARK: GLYPHS                                                          //
///////////////////////////////////////////////////////////////////////////

bool SFMLFontImpl::hasGlyph(std::uint32_t aCodePoint) const {
    return _font.hasGlyph(aCodePoint);
}

Glyph SFMLFontImpl::getGlyph(std::uint32_t aCodePoint,
                             PZInteger     aCharacterSize,
                             bool          aBold,
                             float         aOutlineThickness) const {
    return ToHg(
        _font.getGlyph(aCodePoint, static_cast<unsigned int>(aCharacterSize), aBold, aOutlineThickness));
}

float SFMLFontImpl::getKerning(std::uint32_t aFirst,
                               std::uint32_t aSecond,
                               PZInteger     aCharacterSize,
                               bool          aBold) const {
    return _font.getKerning(aFirst, aSecond, static_cast<unsigned int>(aCharacterSize), aBold);
}

///////////////////////////////////////////////////////////////////////////
// MARK: UNDERLINE                                                       //
///////////////////////////////////////////////////////////////////////////

float SFMLFontImpl::getUnderlinePosition(PZInteger aCharacterSize) const {
    return _font.getUnderlinePosition(static_cast<unsigned int>(aCharacterSize));
}

float SFMLFontImpl::getUnderlineThickness(PZInteger aCharacterSize) const {
    return _font.getUnderlineThickness(static_cast<unsigned int>(aCharacterSize));
}

///////////////////////////////////////////////////////////////////////////
// MARK: MISCELLANEOUS                                                   //
///////////////////////////////////////////////////////////////////////////

float SFMLFontImpl::getLineSpacing(PZInteger aCharacterSize) const {
    return _font.getLineSpacing(static_cast<unsigned int>(aCharacterSize));
}

void SFMLFontImpl::setSmooth(bool aSmooth) {
    _font.setSmooth(aSmooth);
}

bool SFMLFontImpl::isSmooth() const {
    return _font.isSmooth();
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

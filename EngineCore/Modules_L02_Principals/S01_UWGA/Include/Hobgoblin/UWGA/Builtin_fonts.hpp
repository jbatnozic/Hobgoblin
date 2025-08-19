// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_BUILTIN_FONTS_HPP
#define UHOBGOBLIN_UWGA_BUILTIN_FONTS_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! Use this enum with an instance of `uwga::System` to access instances of builtin fonts.
enum class BuiltInFont {
    /// A nice serif roman font.
    /// SOURCE: https://www.1001fonts.com/cmu-font.html#styles
    COMPUTER_MODERN_SERIF_ROMAN,

    /// A monospaced typewriter-style font.
    /// SOURCE: https://www.1001fonts.com/cmu-font.html#styles
    COMPUTER_MODERN_TYPEWRITER_TEXT,

    /// Old classic serif font.
    /// SOURCE: https://github.com/georgd/EB-Garamond
    EB_GARAMOND_12_REGULAR,

    /// Clean, modern monospace font.
    /// SOURCE: https://fonts.google.com/specimen/Inconsolata
    INCONSOLATA_REGULAR,

    /// A modern looking sans-serif font.
    /// SOURCE: https://fonts.google.com/specimen/Titillium+Web
    TITILLIUM_REGULAR,

    /// Total number of supported builtin fonts.
    /// \warning always keep last in the enum!
    COUNT
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_BUILTIN_FONTS_HPP

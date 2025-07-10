// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_COLOR_HPP
#define UHOBGOBLIN_UWGA_COLOR_HPP

#include <cstdint>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! Represents a color in 32-bit RGBA format.
class Color {
public:
    constexpr Color() = default;

    //! Bits 31..24 = Red, Bits 23..16 = Green, Bits 15..8 = Blue, 7..0 = Alpha
    constexpr explicit Color(std::uint32_t aColor)
        : r{static_cast<std::uint8_t>((aColor >> 24) & 0xFF)}
        , g{static_cast<std::uint8_t>((aColor >> 16) & 0xFF)}
        , b{static_cast<std::uint8_t>((aColor >> 8) & 0xFF)}
        , a{static_cast<std::uint8_t>((aColor >> 0) & 0xFF)} {}

    constexpr Color(std::uint8_t aRed,
                    std::uint8_t aGreen,
                    std::uint8_t aBlue,
                    std::uint8_t aAlpha = 255)
        : r{aRed}
        , g{aGreen}
        , b{aBlue}
        , a{aAlpha} {}

    constexpr std::uint32_t toInt() const {
        return static_cast<std::uint32_t>((r << 24) | (g << 16) | (b << 8) | a);
    }

    constexpr Color withAlpha(std::uint8_t aAlpha) const {
        Color result = SELF;
        result.a     = aAlpha;
        return result;
    }

    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

static_assert(sizeof(Color) == sizeof(std::int32_t));

inline constexpr bool operator==(Color aLhs, Color aRhs) {
    return aLhs.r == aRhs.r && aLhs.g == aRhs.g && aLhs.b == aRhs.b && aLhs.a == aRhs.a;
}

#define XMACRO(_name_, _hexcode_) constexpr Color COLOR_##_name_{_hexcode_};
#include <Hobgoblin/UWGA/Private/Xmacro_html_colors.hpp>
#undef XMACRO

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_COLOR_HPP

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Builtin_fonts.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

extern const unsigned char UHOBGOBLIN_UWGA_cmu_serif_roman_ttf[];
extern const unsigned int  UHOBGOBLIN_UWGA_cmu_serif_roman_ttf_len;
extern const unsigned char UHOBGOBLIN_UWGA_cmuntt_ttf[];
extern const unsigned int  UHOBGOBLIN_UWGA_cmuntt_ttf_len;
extern const unsigned char UHOBGOBLIN_UWGA_EBGaramond12_Regular_ttf[];
extern const unsigned int  UHOBGOBLIN_UWGA_EBGaramond12_Regular_ttf_len;
extern const unsigned char UHOBGOBLIN_UWGA_Inconsolata_Regular_ttf[];
extern const unsigned int  UHOBGOBLIN_UWGA_Inconsolata_Regular_ttf_len;
extern const unsigned char UHOBGOBLIN_UWGA_TitilliumWeb_Regular_ttf[];
extern const unsigned int  UHOBGOBLIN_UWGA_TitilliumWeb_Regular_ttf_len;

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

inline void GetBuiltinFontData(BuiltInFont aFontChoice, const void** aData, PZInteger* aByteCount) {
    switch (aFontChoice) {
    case BuiltInFont::COMPUTER_MODERN_SERIF_ROMAN:
        (*aData)      = &UHOBGOBLIN_UWGA_cmu_serif_roman_ttf;
        (*aByteCount) = ToPz(UHOBGOBLIN_UWGA_cmu_serif_roman_ttf_len);
        break;

    case BuiltInFont::COMPUTER_MODERN_TYPEWRITER_TEXT:
        (*aData)      = &UHOBGOBLIN_UWGA_cmuntt_ttf;
        (*aByteCount) = ToPz(UHOBGOBLIN_UWGA_cmuntt_ttf_len);
        break;

    case BuiltInFont::EB_GARAMOND_12_REGULAR:
        (*aData)      = &UHOBGOBLIN_UWGA_EBGaramond12_Regular_ttf;
        (*aByteCount) = ToPz(UHOBGOBLIN_UWGA_EBGaramond12_Regular_ttf_len);
        break;

    case BuiltInFont::INCONSOLATA_REGULAR:
        (*aData)      = &UHOBGOBLIN_UWGA_Inconsolata_Regular_ttf;
        (*aByteCount) = ToPz(UHOBGOBLIN_UWGA_Inconsolata_Regular_ttf_len);
        break;

    case BuiltInFont::TITILLIUM_REGULAR:
        (*aData)      = &UHOBGOBLIN_UWGA_TitilliumWeb_Regular_ttf;
        (*aByteCount) = ToPz(UHOBGOBLIN_UWGA_TitilliumWeb_Regular_ttf_len);
        break;

    default:
        break;
    }

    HG_THROW_TRACED(InvalidArgumentError, 0, "Invalid value for BuiltInFont ({}).", (int)aFontChoice);
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

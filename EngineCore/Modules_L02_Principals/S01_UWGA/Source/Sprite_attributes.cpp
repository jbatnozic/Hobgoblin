// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Sprite_attributes.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <unicode/regex.h>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

namespace {
SpriteAttributes ReadSpriteAttributesFromOriginFile(const std::filesystem::path& aPath) {
    SpriteAttributes attrs;

    if (!std::filesystem::exists(aPath)) {
        return attrs;
    }

    const auto contents = LoadWholeFile(aPath);

    // TAG x: +/-123.456 y: +/-123.456
    const auto* pattern =
        uR"_(\s*([a-z]+)[\s]+x:[\s]*([+-]?[0-9]+[.]?[0-9]*)[\s]+y:[\s]*([+-]?[0-9]+[.]?[0-9]*)[\s]*)_";

    URegex        regex{pattern, URegex::CASE_INSENSITIVE};
    UMatchResults matchResults;

    if (RegexMatch(contents, regex, matchResults)) {
        HG_HARD_ASSERT(matchResults.getGroupCount() == 3);

        const auto tag = matchResults[1];

        const auto x    = matchResults[2];
        const auto xnum = std::stof(UniStrConv(TO_ASCII_STD_STRING, x));

        const auto y    = matchResults[3];
        const auto ynum = std::stof(UniStrConv(TO_ASCII_STD_STRING, y));

        const auto offset = math::Vector2f{xnum, ynum};

        using OriginOffset = SpriteAttributes::OriginOffset;
        if (tag.caseCompare(HG_UNILIT("TOPLEFT"), U_FOLD_CASE_DEFAULT) == 0) {
            attrs.originOffset.emplace(OriginOffset{offset, OriginOffset::RELATIVE_TO_TOP_LEFT});
            return attrs;
        }
        if (tag.caseCompare(HG_UNILIT("CENTER"), U_FOLD_CASE_DEFAULT) == 0) {
            attrs.originOffset.emplace(OriginOffset{offset, OriginOffset::RELATIVE_TO_CENTER});
            return attrs;
        }
    }

    return attrs;
}

SpriteAttributes ReadSpriteAttributesFromSAttrFile(const std::filesystem::path& aPath) {
    SpriteAttributes attrs;

    if (!std::filesystem::exists(aPath)) {
        return attrs;
    }

    // TODO

    return attrs;
}
} // namespace

SpriteAttributes ReadSpriteAttributesFromFile(const std::filesystem::path& aPath) {
    if (!aPath.has_extension()) {
        goto BAD_EXTENSION;
    }

    if (aPath.extension() == ".sattr") {
        return ReadSpriteAttributesFromSAttrFile(aPath);
    }

    if (aPath.extension() == ".origin") {
        return ReadSpriteAttributesFromOriginFile(aPath);
    }

BAD_EXTENSION:
    HG_THROW_TRACED(TracedLogicError,
                    0,
                    "Invalid extension for file containing sprite attributes: '{}'",
                    aPath.extension().string());

    return {};
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

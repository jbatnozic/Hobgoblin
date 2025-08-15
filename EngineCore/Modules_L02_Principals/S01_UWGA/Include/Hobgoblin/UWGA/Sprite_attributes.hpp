// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_ORIGIN_OFFSET_HPP
#define UHOBGOBLIN_UWGA_ORIGIN_OFFSET_HPP

#include <Hobgoblin/Math.hpp>

#include <filesystem>
#include <optional>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

struct SpriteAttributes {
    //! Tells the loader where to place the origin of the sprite.
    //! By default the top-left pixel is used as the origin.
    struct OriginOffset {
        math::Vector2f value;

        enum Kind {
            RELATIVE_TO_TOP_LEFT,
            RELATIVE_TO_CENTER
        };

        Kind kind;
    };

    //! Offset of the sprite's origin. If not set (==`std::nullopt`), the default origin will be used.
    std::optional<OriginOffset> originOffset;

    //! Check if the object represents default attributes (returns `true`) or not (returns `false`).
    bool isDefault() const;
};

//! Read sprite attributes from a text file. The file extension must be one of:
//! - `.sattr`  - recommended format (JSON)
//! - `.origin` - old custom format, supported for backwards compatibility.
//! \throws TracedLogicError if any other extension (or none) is given.
//! \note if the extension is valid but the file doesn't exist, the default attributes are returned.
SpriteAttributes ReadSpriteAttributesFromFile(const std::filesystem::path& aPath);

inline bool SpriteAttributes::isDefault() const {
    return !originOffset.has_value();
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_ORIGIN_OFFSET_HPP

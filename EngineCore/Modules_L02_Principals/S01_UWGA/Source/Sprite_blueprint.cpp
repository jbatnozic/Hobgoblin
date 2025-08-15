// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Sprite_blueprint.hpp>

#include <Hobgoblin/UWGA/System.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

namespace {
using OriginOffset = SpriteAttributes::OriginOffset;
void ApplyOffset(Sprite& aSprite, const OriginOffset& aOffset) {
    switch (aOffset.kind) {
    case OriginOffset::RELATIVE_TO_TOP_LEFT:
        aSprite.setOrigin(aOffset.value);
        break;

    case OriginOffset::RELATIVE_TO_CENTER:
        {
            const auto size = aSprite.getLocalBounds();
            aSprite.setOrigin(size.w / 2.f + aOffset.value.x, size.h / 2.f + aOffset.value.y);
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for OriginOffset::Kind ({}).", (int)aOffset.kind);
    }
}
} // namespace

Sprite SpriteBlueprint::spr() const {
    auto result = Sprite{_texture->getSystem(), _texture, _textureRects.begin(), _textureRects.end()};
    if (_spriteAttrs.originOffset.has_value()) {
        ApplyOffset(result, *_spriteAttrs.originOffset);
    }
    return result;
}

PZInteger SpriteBlueprint::getSubspriteCount() const {
    return stopz(_textureRects.size());
}

const SpriteAttributes& SpriteBlueprint::getSpriteAttributes() const {
    return _spriteAttrs;
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

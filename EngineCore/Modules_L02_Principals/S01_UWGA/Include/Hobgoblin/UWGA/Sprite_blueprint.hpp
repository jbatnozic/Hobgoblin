// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SPRITE_BLUEPRINT_HPP
#define UHOBGOBLIN_UWGA_SPRITE_BLUEPRINT_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Sprite.hpp>
#include <Hobgoblin/UWGA/Sprite_attributes.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>
#include <Hobgoblin/Utility/Compressed_small_vector.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Texture;

class SpriteBlueprint {
public:
    ///////////////////////////////////////////////////////////////////////////
    // MARK: CONSTRUCTION                                                    //
    ///////////////////////////////////////////////////////////////////////////

    //! Construct with single subsprite
    SpriteBlueprint(const Texture&          aTexture,
                    TextureRect             aTextureRect,
                    const SpriteAttributes& aSpriteAttrs = {});

    //! Construct with one or more subsprites, providing begin and end iterators
    //! to a container of `TextureRect` objects.
    template <class taRectsBeginIter, class taRectsEndIter>
    SpriteBlueprint(const Texture&          aTexture,
                    const taRectsBeginIter& aTextureRectsBeginIter,
                    const taRectsEndIter&   aTextureRectsEndIter,
                    const SpriteAttributes& aSpriteAttrs = {});

    //! Construct with one or more subsprites, providing a reference to a
    //! container of `TextureRect` objects.
    template <class taRects>
    SpriteBlueprint(const Texture&          aTexture,
                    const taRects&          aTextureRects,
                    const SpriteAttributes& aSpriteAttrs = {});

    ///////////////////////////////////////////////////////////////////////////
    // MARK: GETTERS                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! Construct and return the sprite.
    Sprite spr() const;

    //! Returns number of contained subsprites.
    PZInteger getSubspriteCount() const;

    //! Get the sprite attributes that will modify the sprite that's returned when you call `spr()`.
    const SpriteAttributes& getSpriteAttributes() const;

private:
    const Texture* _texture;

    util::CompressedSmallVector<TextureRect, 1> _textureRects;

    SpriteAttributes _spriteAttrs;
};

// MARK: Inline constructors

inline SpriteBlueprint::SpriteBlueprint(const Texture&          aTexture,
                                        TextureRect             aTextureRect,
                                        const SpriteAttributes& aSpriteAttrs)
    : _texture{&aTexture}
    , _spriteAttrs{aSpriteAttrs} //
{
    _textureRects.push_back(aTextureRect);
}

template <class taRectsBeginIter, class taRectsEndIter>
SpriteBlueprint::SpriteBlueprint(const Texture&          aTexture,
                                 const taRectsBeginIter& aTextureRectsBeginIter,
                                 const taRectsEndIter&   aTextureRectsEndIter,
                                 const SpriteAttributes& aSpriteAttrs)
    : _texture{&aTexture}
    , _spriteAttrs{aSpriteAttrs} //
{
    const auto size = std::distance(aTextureRectsBeginIter, aTextureRectsEndIter);

    if (size == 0) {
        HG_THROW_TRACED(InvalidArgumentError,
                        0,
                        "SpriteBlueprint must be constructed with at least 1 subsprite.");
    }

    _textureRects.reserve(size);

    auto iter = aTextureRectsBeginIter;
    for (; iter != aTextureRectsEndIter; ++iter) {
        _textureRects.push_back(*iter);
    }
}

template <class taRects>
SpriteBlueprint::SpriteBlueprint(const Texture&          aTexture,
                                 const taRects&          aTextureRects,
                                 const SpriteAttributes& aSpriteAttrs)
    : SpriteBlueprint{aTexture, aTextureRects.begin(), aTextureRects.end(), aSpriteAttrs} {}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SPRITE_BLUEPRINT_HPP

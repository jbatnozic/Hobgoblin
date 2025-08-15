// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SPRITE_HPP
#define UHOBGOBLIN_UWGA_SPRITE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Drawable.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>
#include <Hobgoblin/UWGA/Transformable.hpp>
#include <Hobgoblin/UWGA/Vertex.hpp>
#include <Hobgoblin/Utility/Compressed_small_vector.hpp>

#include <array>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

//! Drawable representation of a Texture, with its own transformations, color, etc.
//! Using this class allows us to easily display a Texture (or a part of it) on a Canvas.
class Sprite
    : public Drawable
    , public Transformable {
public:
    class Subsprite;

    //! Default-constructs the sprite.
    //! Note that the sprite won't be visible when drawn until it's given
    //! a non-null texture (see `setTexture`) and at least 1 subsprite (see
    //! `addSubsprite`).
    Sprite(const System& aSystem, const Texture* aTexture = nullptr);

    //! Constructs a sprite with a single subsprite.
    Sprite(const System& aSystem, const Texture* aTexture, const Subsprite& aSubsprite);

    //! Constructs a sprite with one or more subsprite from a collection of
    //! `Subsprite`s, or objects implicitly convertible to `Subsprite`s (such as `TextureRect`s).
    template <class taBeginIter, class taEndIter>
    Sprite(const System&       aSystem,
           const Texture*      aTexture,
           const taBeginIter&& aBeginIter,
           const taEndIter&&   aEndIter);

    //! Copy contructor.
    Sprite(const Sprite& aOther);

    //! Copy assignment.
    Sprite& operator=(const Sprite& aOther);

    //! Move contructor.
    Sprite(Sprite&& aOther);

    //! Move assignment.
    Sprite& operator=(Sprite&& aOther);

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: TEXTURE                                                         //
    ///////////////////////////////////////////////////////////////////////////

    void setTexture(const Texture* aTexture);

    const Texture* getTexture() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SUBSPRITES                                                      //
    ///////////////////////////////////////////////////////////////////////////

    //! Represents a single subsprite of the sprite.
    //! A subsprite is defined as subsection (rectangle) on the sprite's texture.
    class Subsprite {
    public:
        Subsprite(TextureRect aTextureRect);

    private:
        static constexpr std::size_t VERTEX_COUNT = 5;

        TextureRect                      _textureRect;
        std::array<Vertex, VERTEX_COUNT> _vertices;

        friend class Sprite;

        math::Rectangle<float> getLocalBounds() const;
    };

    //! All of the sprite's subsprites.
    //! This collection is public and you can freely manipulate it to add, remove,
    //! shuffle around, or otherwise change the sprite's set of subsprites.
    util::CompressedSmallVector<Subsprite, 1> subsprites;

    //! Utility function to append a new subsprite.
    //! \param aSubsprite the subsprite to add.
    void addSubsprite(const Subsprite& aSubsprite);

    //! Utility function to get a single subsprite.
    //! \param aSubspriteIndex index of the subsprite to return.
    const Subsprite& getSubsprite(PZInteger aSubspriteIndex) const;

    //! Utility function to remove a single subsprite.
    //! \param aSubspriteIndex index of the subsprite to remove.
    void removeSubsprite(PZInteger aSubspriteIndex);

    //! Utility function to return the number of subsprites in the sprite.
    PZInteger getSubspriteCount() const;

    //! Returns the value of the underlying subsprite selector.
    float getSubspriteSelector() const;

    //! Returns the index of the subsprite that's currently considered active
    //! based on the selector's value (see `getSubspriteSelector`). As the
    //! selector is a floating-point value, there are some extra considerations:
    //! - If the selector is not a whole number, it will be rounded down.
    //! - If the selector is out of bounds [0..subsprite_count], it will be brought
    //!   back into bounds by modulo arithmetic.
    //!
    //! If the sprite has no subsprites, this method will throw.
    PZInteger getCurrentSubspriteIndex() const;

    void selectSubsprite(int aSelectorValue);

    void selectSubsprite(float aSelectorValue);

    void advanceSubsprite(int aSelectorOffset);

    void advanceSubsprite(float aSelectorOffset);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: COLOR                                                           //
    ///////////////////////////////////////////////////////////////////////////

    void setColor(Color aColor);

    Color getColor() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: BOUNDS                                                          //
    ///////////////////////////////////////////////////////////////////////////

    //! Returns the local bounding rectangle of a specific subsprite.
    //! The returned rectangle is in local coordinates, which means that it
    //! ignores the transformations (translation, rotation, scale, ...) that are
    //! applied to the subsprite. In other words, this function returns the
    //! bounds of the subsprite in the subsprite's coordinate system.
    math::Rectangle<float> getLocalBounds(PZInteger aSubspriteIndex) const;

    //! Returns the local bounding rectangle of the surrently selected subsprite.
    math::Rectangle<float> getLocalBounds() const;

    //! Returns the global bounding rectangle of a specific subsprite.
    //! The returned rectangle is in global coordinates, which means that it takes
    //! into account the transformations (translation, rotation, scale, ...) that
    //! are applied to the subsprite. In other words, this function returns the
    //! bounds of the subsprite in the global 2D world's coordinate system.
    math::Rectangle<float> getGlobalBounds(PZInteger aSubspriteIndex) const;

    //! Returns the global bounding rectangle of the currently selected subsprite.
    math::Rectangle<float> getGlobalBounds() const;

    //! Returns true if all the subsprites are of the same size.
    bool isNormalized() const;

    //! Same as `isNormalized`.
    bool areAllSubspritesOfSameSize() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: DRAW                                                            //
    ///////////////////////////////////////////////////////////////////////////

    void drawOnto(Canvas&             aCanvas,
                  const RenderStates& aRenderStates = RENDER_STATES_DEFAULT) const override;

private:
    const System*  _system;
    const Texture* _texture = nullptr;

    float _subspriteSelector = 0.f;
    Color _color             = COLOR_WHITE;

    static std::unique_ptr<Transform> _createTransform(const System& aSystem);
};

template <class taBeginIter, class taEndIter>
Sprite::Sprite(const System&       aSystem,
               const Texture*      aTexture,
               const taBeginIter&& aBeginIter,
               const taEndIter&&   aEndIter)
    : Transformable{_createTransform(aSystem)}
    , _system{&aSystem}
    , _texture{aTexture} //
{
    for (auto iter = aBeginIter; iter != aEndIter; ++iter) {
        const auto& elem = *iter;
        addSubsprite(elem);
    }
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SPRITE_HPP

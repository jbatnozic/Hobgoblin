// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SPRITE_LOADER_HPP
#define UHOBGOBLIN_UWGA_SPRITE_LOADER_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Sprite.hpp>
#include <Hobgoblin/UWGA/Sprite_blueprint.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>
#include <Hobgoblin/UWGA/Texture_packing.hpp>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! Used to refer to a sprite by a numerical handle.
using SpriteIdNumerical = PZInteger;

//! Used to refer to a sprite by a textual handle (name).
using SpriteIdTextual = std::string;

namespace detail {
class TextureBuilderImpl;
} // namespace detail

//! Exception that's thrown when processing a sprite manifest (see `loadSpriteManifest()`) fails.
//! \note it can fail for several reasons: syntatic or semantic errors in the manifest file itself,
//!       failure to open files containing sprites, failure to create or pack textures, etc.
class SpriteManifestProcessingError : public TracedRuntimeError {
public:
    using TracedRuntimeError::TracedRuntimeError;
};

class System;

class SpriteLoader {
public:
    //! Constructor
    SpriteLoader(const System& aSystem);

    //! Copying not allowed
    SpriteLoader(const SpriteLoader& aOther)            = delete;
    SpriteLoader& operator=(const SpriteLoader& aOther) = delete;

    //! Moving is OK
    SpriteLoader(SpriteLoader&& aOther)            = default;
    SpriteLoader& operator=(SpriteLoader&& aOther) = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: TEXTUREBUILDER                                                  //
    ///////////////////////////////////////////////////////////////////////////

    class TextureBuilder {
    public:
        //! Creates a new sprite, then loads an image from a file and adds it as the
        //! sprite's first subsprite.
        //!
        //! \param aSpriteId Identifier of the newly created sprite.
        //! \param aFilePath Path to the image file to load.
        //!
        //! \throws <exception> If the sprite by the given ID already exists.
        //!
        //! \note As the `addSubsprite` method will add a new sprite if it does not
        //!       already exist, this method is not mandatory to use and is
        //!       basically syntatic sugar.
        virtual NeverNull<TextureBuilder*> addSprite(SpriteIdNumerical            aSpriteId,
                                                     const std::filesystem::path& aFilePath) = 0;

        //! Loads an image from a file and adds it as a subsprite.
        //!
        //! \param aSpriteId Identifier of the sprite to which to add the subsprite.
        //!                  If there does not already exist a sprite by this ID, it will
        //!                  be created automatically.
        //! \param aSubspriteIndex Index under which to store the loaded subsprite. If multiple
        //!                        loaded images target the same subsprite of a single sprite,
        //!                        which one will be kept is undefined.
        //! \param aFilePath Path to the image file to load.
        virtual NeverNull<TextureBuilder*> addSubsprite(SpriteIdNumerical            aSpriteId,
                                                        PZInteger                    aSubspriteIndex,
                                                        const std::filesystem::path& aFilePath) = 0;

        //! Loads an image from a file and adds it as a subsprite.
        //!
        //! \param aSpriteId Identifier of the sprite to which to add the subsprite.
        //!                  If there does not already exist a sprite by this ID, it will
        //!                  be created automatically.
        //! \param aFilePath Path to the image file to load.
        //!
        //! \note As this method does not take a subsprite index as a parameter, the loaded
        //!       image will be appended to the sprite's array of subsprites.
        virtual NeverNull<TextureBuilder*> addSubsprite(SpriteIdNumerical            aSpriteId,
                                                        const std::filesystem::path& aFilePath) = 0;

        //! Creates a new sprite, then loads an image from a file and adds it as the
        //! sprite's first subsprite.
        //!
        //! \param aSpriteId Identifier of the newly created sprite.
        //! \param aFilePath Path to the image file to load.
        //!
        //! \throws <exception> If the sprite by the given ID already exists.
        //!
        //! \note As the `addSubsprite` method will add a new sprite if it does not
        //!       already exist, this method is not mandatory to use and is
        //!       basically syntatic sugar.
        virtual NeverNull<TextureBuilder*> addSprite(const SpriteIdTextual&       aSpriteId,
                                                     const std::filesystem::path& aFilePath) = 0;

        //! Loads an image from a file and adds it as a subsprite.
        //!
        //! \param aSpriteId Identifier of the sprite to which to add the subsprite.
        //!                  If there does not already exist a sprite by this ID, it will
        //!                  be created automatically.
        //! \param aSubspriteIndex Index under which to store the loaded subsprite. If multiple
        //!                        loaded images target the same subsprite of a single sprite,
        //!                        which one will be kept is undefined.
        //! \param aFilePath Path to the image file to load.
        virtual NeverNull<TextureBuilder*> addSubsprite(const SpriteIdTextual&       aSpriteId,
                                                        PZInteger                    aSubspriteIndex,
                                                        const std::filesystem::path& aFilePath) = 0;

        //! Loads an image from a file and adds it as a subsprite.
        //!
        //! \param aSpriteId Identifier of the sprite to which to add the subsprite.
        //!                  If there does not already exist a sprite by this ID, it will
        //!                  be created automatically.
        //! \param aFilePath Path to the image file to load.
        //!
        //! \note As this method does not take a subsprite index as a parameter, the loaded
        //!       image will be appended to the sprite's array of subsprites.
        virtual NeverNull<TextureBuilder*> addSubsprite(const SpriteIdTextual&       aSpriteId,
                                                        const std::filesystem::path& aFilePath) = 0;

        // TODO: loading images from memory

        //! Builds a texture from all the added images.
        //!
        //! \param aHeuristic Texture packing heuristic to use.
        //! \param aOccupancy Pointer to a float. If not null, it will be set to a value between
        //!                   0.f and 1.f signifying how much of the texture's surface was taken
        //!                   up by the loaded images.
        //!
        //! \returns Const reference to the built texture. Note that the lifetime of the texture
        //!          will be managed by the parent `SpriteLoader` of this `TextureBuilder`.
        //!
        //! \throws TextturePackignError in case of failure (most commonly not enough room on the
        //!                              target texture for all the requested sprites).
        virtual const Texture& finalize(TexturePackingHeuristic aHeuristic,
                                        float*                  aOccupancy = nullptr) = 0;

        //! Virtual destructor.
        virtual ~TextureBuilder() = default;
    };

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SPRITELOADER                                                    //
    ///////////////////////////////////////////////////////////////////////////

    //! Allocates a new texture with the given width and height (in pixels).
    //!
    //! \returns Unique pointer to `TextureBuilder` which can be used to
    //!          load sprite and subsprite images to the texture. The pointer is
    //!          guaranteed to not be null.
    //!          Don't forget to call `finalize()`!
    AvoidNull<std::unique_ptr<TextureBuilder>> startTexture(PZInteger aWidth, PZInteger aHeight);

    void removeTexture(Texture& aTexture);

    using SpriteManifestEnumerationMap = std::unordered_map<std::string, SpriteIdNumerical>;

    //! Creates textures and adds sprites to them according to a sprite manifest file.
    //! This is not mutually exclusive with the manual (`addTexture`) methods of loading sprites.
    //!
    //! \param aManifestFilePath path to the sprite manifest file (including the file name and its
    //!                          extension).
    //! \param aSpriteManifestEnumerationMap if not null, the pointed-to map will be populated with all
    //!                                      the enumerations defined in the manifest file.
    //!
    //! \throws SpriteManifestProcessingError on failure.
    void loadSpriteManifest(const std::filesystem::path&  aManifestFilePath,
                            SpriteManifestEnumerationMap* aSpriteManifestEnumerationMap = nullptr);

    SpriteBlueprint getBlueprint(SpriteIdNumerical aSpriteId) const;

    SpriteBlueprint getBlueprint(const SpriteIdTextual& aSpriteId) const;

    //! Deletes all texture and sprite blueprint data from the loader,
    //! returning it to its initial state. Be careful not to keep using
    //! any sprites or sprite blueprints which you previously got from
    //! this loader, which will now reference deallocated textures.
    void clear();

private:
    friend class detail::TextureBuilderImpl;

    const System* _system;

    std::vector<std::unique_ptr<Texture>> _textures;

    std::unordered_map<PZInteger, SpriteBlueprint>   _indexedBlueprints;
    std::unordered_map<std::string, SpriteBlueprint> _mappedBlueprints;

    void _pushBlueprint(SpriteIdNumerical aSpriteId, SpriteBlueprint aBlueprint);

    void _pushBlueprint(const SpriteIdTextual& aSpriteId, SpriteBlueprint aBlueprint);

    void _pushTexture(std::unique_ptr<Texture> aTexture);
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SPRITE_LOADER_HPP

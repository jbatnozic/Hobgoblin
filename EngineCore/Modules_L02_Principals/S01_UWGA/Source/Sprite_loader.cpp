// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Sprite_loader.hpp>
#include <Hobgoblin/UWGA/System.hpp>

#include <Hobgoblin/Logging.hpp>

#include <cassert>
#include <memory>

#include "Sprite_manifest_parser.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

///////////////////////////////////////////////////////////////////////////
// MARK: TEXTURE BUILDER                                                 //
///////////////////////////////////////////////////////////////////////////

namespace {
constexpr auto LOG_ID = "Hobgoblin.Graphics";
} // namespace

namespace detail {
class TextureBuilderImpl : public SpriteLoader::TextureBuilder {
public:
    TextureBuilderImpl(SpriteLoader& aSpriteLoader, PZInteger aTextureWidth, PZInteger aTextureHeight)
        : _loader{aSpriteLoader} //
    {
        _texture = _loader._system->createTexture(aTextureWidth, aTextureHeight);
    }

    NeverNull<TextureBuilder*> addSprite(SpriteIdNumerical            aSpriteId,
                                         const std::filesystem::path& aFilePath) override {
        _assertNotFinalized();

        auto loadedImage = _loadImage(aFilePath);

        auto [iter, didInsert] = _indexedRequests.try_emplace(aSpriteId);

        if (!didInsert) {
            HG_THROW_TRACED(TracedLogicError, 0, "Sprite with this ID ({}) already exists!", aSpriteId);
        }

        auto& req = iter->second;
        req.subsprites.emplace_back();
        req.subsprites.back().image    = std::move(loadedImage.image);
        req.subsprites.back().attrs    = loadedImage.attrs;
        req.subsprites.back().occupied = true;

        return this;
    }

    NeverNull<TextureBuilder*> addSubsprite(SpriteIdNumerical            aSpriteId,
                                            PZInteger                    aSubspriteIndex,
                                            const std::filesystem::path& aFilePath) override {
        _assertNotFinalized();

        auto loadedImage = _loadImage(aFilePath);

        auto& req = _indexedRequests[aSpriteId];
        if (req.subsprites.size() <= pztos(aSubspriteIndex)) {
            req.subsprites.resize(pztos(aSubspriteIndex) + 1);
        }
        auto& subsprite = req.subsprites[pztos(aSubspriteIndex)];
        if (subsprite.occupied) {
            HG_LOG_WARN(LOG_ID,
                        "addSubsprite() overwriting previously loaded subsprite at index "
                        "{} of sprite by ID {}.",
                        aSubspriteIndex,
                        aSpriteId);
        }
        subsprite.image    = std::move(loadedImage.image);
        subsprite.attrs    = loadedImage.attrs;
        subsprite.occupied = true;

        return this;
    }

    NeverNull<TextureBuilder*> addSubsprite(SpriteIdNumerical            aSpriteId,
                                            const std::filesystem::path& aFilePath) override {
        _assertNotFinalized();

        auto loadedImage = _loadImage(aFilePath);

        auto& req = _indexedRequests[aSpriteId];
        req.subsprites.emplace_back();
        req.subsprites.back().image    = std::move(loadedImage.image);
        req.subsprites.back().attrs    = loadedImage.attrs;
        req.subsprites.back().occupied = true;

        return this;
    }

    NeverNull<TextureBuilder*> addSprite(const SpriteIdTextual&       aSpriteId,
                                         const std::filesystem::path& aFilePath) override {
        _assertNotFinalized();

        auto loadedImage = _loadImage(aFilePath);

        auto [iter, didInsert] = _mappedRequests.try_emplace(aSpriteId);

        if (!didInsert) {
            HG_THROW_TRACED(TracedLogicError, 0, "Sprite with this ID ({}) already exists!", aSpriteId);
        }

        auto& req = iter->second;
        req.subsprites.emplace_back();
        req.subsprites.back().image    = std::move(loadedImage.image);
        req.subsprites.back().attrs    = loadedImage.attrs;
        req.subsprites.back().occupied = true;

        return this;
    }

    NeverNull<TextureBuilder*> addSubsprite(const SpriteIdTextual&       aSpriteId,
                                            PZInteger                    aSubspriteIndex,
                                            const std::filesystem::path& aFilePath) override {
        _assertNotFinalized();

        auto loadedImage = _loadImage(aFilePath);

        auto& req = _mappedRequests[aSpriteId];
        if (req.subsprites.size() <= pztos(aSubspriteIndex)) {
            req.subsprites.resize(pztos(aSubspriteIndex) + 1);
        }
        auto& subsprite = req.subsprites[pztos(aSubspriteIndex)];
        if (subsprite.occupied) {
            HG_LOG_WARN(LOG_ID,
                        "addSubsprite() overwriting previously loaded subsprite at index "
                        "{} of sprite by ID {}.",
                        aSubspriteIndex,
                        aSpriteId);
        }
        subsprite.image    = std::move(loadedImage.image);
        subsprite.attrs    = loadedImage.attrs;
        subsprite.occupied = true;

        return this;
    }

    NeverNull<TextureBuilder*> addSubsprite(const SpriteIdTextual&       aSpriteId,
                                            const std::filesystem::path& aFilePath) override {
        _assertNotFinalized();

        auto loadedImage = _loadImage(aFilePath);

        auto& req = _mappedRequests[aSpriteId];
        req.subsprites.emplace_back();
        req.subsprites.back().image    = std::move(loadedImage.image);
        req.subsprites.back().attrs    = loadedImage.attrs;
        req.subsprites.back().occupied = true;

        return this;
    }

    const Texture& finalize(TexturePackingHeuristic aHeuristic, float* aOccupancy) override {
        _assertNotFinalized();

        const Texture* result = _texture.get();

        // Part 1: Pack the texture
        {
            std::vector<SubspriteData*> subspritePointers;
            std::vector<Image*>         imagePointers;

            for (auto& pair : _indexedRequests) {
                for (auto& subsprite : pair.second.subsprites) {
                    if (!subsprite.occupied) {
                        HG_LOG_WARN(LOG_ID,
                                    "Sprite or multisprite with index {} has undefined subsprite with "
                                    "defined subsprites after it.",
                                    pair.first);
                        continue;
                    }
                    subspritePointers.push_back(std::addressof(subsprite));
                    imagePointers.push_back(subsprite.image.get());
                }
            }

            for (auto& pair : _mappedRequests) {
                for (auto& subsprite : pair.second.subsprites) {
                    if (!subsprite.occupied) {
                        HG_LOG_WARN(LOG_ID,
                                    "Sprite or multisprite with ID '{}' has undefined subsprite with "
                                    "defined subsprites after it.",
                                    pair.first);
                        continue;
                    }
                    subspritePointers.push_back(std::addressof(subsprite));
                    imagePointers.push_back(subsprite.image.get());
                }
            }

            // Invoke packing algorithm
            const auto textureRects = PackTexture(*_texture, imagePointers, aHeuristic, aOccupancy);
            assert(subspritePointers.size() == imagePointers.size() &&
                   imagePointers.size() == textureRects.size());

            for (std::size_t i = 0; i < textureRects.size(); i += 1) {
                subspritePointers[i]->rect = textureRects[i];
            }
        }

        // Part 2: Make blueprints and push to the loader
        {
            std::vector<TextureRect> rects;

            for (auto& pair : _indexedRequests) {
                rects.clear();

                SpriteAttributes attrs;
                for (const auto& subsprite : pair.second.subsprites) {
                    rects.push_back(subsprite.rect);
                    if (attrs.isDefault()) {
                        attrs = subsprite.attrs;
                    }
                }

                _loader._pushBlueprint(pair.first, {*_texture, rects, attrs});
            }

            for (auto& pair : _mappedRequests) {
                rects.clear();

                SpriteAttributes attrs;
                for (const auto& subsprite : pair.second.subsprites) {
                    rects.push_back(subsprite.rect);
                    if (attrs.isDefault()) {
                        attrs = subsprite.attrs;
                    }
                }

                _loader._pushBlueprint(pair.first, {*_texture, rects, attrs});
            }

            _loader._pushTexture(std::move(_texture));
        }

        _indexedRequests.clear();
        _mappedRequests.clear();
        _finalized = true;

        return *result;
    }

    ~TextureBuilderImpl() override {
        if (!_finalized) {
            HG_LOG_WARN(LOG_ID, "Non-finalized TextureBuilder being destroyed.");
        }
    }

private:
    SpriteLoader& _loader;

    std::unique_ptr<Texture> _texture;

    struct SubspriteData {
        std::unique_ptr<Image> image;
        TextureRect            rect;
        SpriteAttributes       attrs;
        bool                   occupied = false;
    };

    struct AddMultispriteRequest {
        std::vector<SubspriteData> subsprites;
    };

    std::unordered_map<SpriteIdNumerical, AddMultispriteRequest> _indexedRequests;
    std::unordered_map<SpriteIdTextual, AddMultispriteRequest>   _mappedRequests;

    bool _finalized = false;

    void _assertNotFinalized() const {
        HG_HARD_ASSERT(!_finalized);
    }

    struct LoadedImage {
        std::unique_ptr<Image> image;
        SpriteAttributes       attrs;
    };

    LoadedImage _loadImage(std::filesystem::path aPath) {
        LoadedImage result;
        result.image = _loader._system->createImage(aPath);

        // Try to load sprite properties from a .sattr file:
        aPath.replace_extension(".sattr");
        result.attrs = ReadSpriteAttributesFromFile(aPath);
        // If none found, try with an .origin file (for legacy support):
        if (result.attrs.isDefault()) {
            aPath.replace_extension(".origin");
            result.attrs = ReadSpriteAttributesFromFile(aPath);
        }

        return result;
    }
};
} // namespace detail

///////////////////////////////////////////////////////////////////////////
// MARK: SPRITE LOADER                                                   //
///////////////////////////////////////////////////////////////////////////

SpriteLoader::SpriteLoader(const System& aSystem)
    : _system{&aSystem} {}

AvoidNull<std::unique_ptr<SpriteLoader::TextureBuilder>> SpriteLoader::startTexture(PZInteger aWidth,
                                                                                    PZInteger aHeight) {
    return std::make_unique<detail::TextureBuilderImpl>(SELF, aWidth, aHeight);
}

void SpriteLoader::removeTexture(Texture& aTextureToRemove) {
    const auto iter =
        std::remove_if(_textures.begin(), _textures.end(), [&](const std::unique_ptr<Texture>& aElem) {
            return (aElem.get() == &aTextureToRemove);
        });
    _textures.erase(iter, _textures.end());
}

void SpriteLoader::loadSpriteManifest(const std::filesystem::path&  aManifestFilePath,
                                      SpriteManifestEnumerationMap* aSpriteManifestEnumerationMap) {
    ParseSpriteManifestFile(aManifestFilePath, SELF, aSpriteManifestEnumerationMap);
}

SpriteBlueprint SpriteLoader::getBlueprint(SpriteIdNumerical aSpriteId) const {
    const auto iter = _indexedBlueprints.find(aSpriteId);
    if (iter == _indexedBlueprints.end()) {
        HG_THROW_TRACED(TracedRuntimeError, 0, "No blueprint with this ID ({}) was found!", aSpriteId);
    }

    return iter->second;
}

SpriteBlueprint SpriteLoader::getBlueprint(const SpriteIdTextual& aSpriteId) const {
    const auto iter = _mappedBlueprints.find(aSpriteId);
    if (iter == _mappedBlueprints.end()) {
        HG_THROW_TRACED(TracedRuntimeError, 0, "No blueprint with this ID ({}) was found!", aSpriteId);
    }

    return iter->second;
}

void SpriteLoader::clear() {
    _indexedBlueprints.clear();
    _mappedBlueprints.clear();
    _textures.clear();
}

void SpriteLoader::_pushBlueprint(SpriteIdNumerical aSpriteId, SpriteBlueprint aBlueprint) {
    const auto iter = _indexedBlueprints.find(aSpriteId);
    if (iter != _indexedBlueprints.end()) {
        HG_THROW_TRACED(TracedRuntimeError,
                        0,
                        "Blueprint for sprite with this ID ({}) already exists!",
                        aSpriteId);
    }

    _indexedBlueprints.emplace(aSpriteId, std::move(aBlueprint));
}

void SpriteLoader::_pushBlueprint(const SpriteIdTextual& aSpriteId, SpriteBlueprint aBlueprint) {
    const auto iter = _mappedBlueprints.find(aSpriteId);
    if (iter != _mappedBlueprints.end()) {
        HG_THROW_TRACED(TracedRuntimeError,
                        0,
                        "Blueprint for sprite with this ID ({}) already exists!",
                        aSpriteId);
    }

    _mappedBlueprints.emplace(aSpriteId, std::move(aBlueprint));
}

void SpriteLoader::_pushTexture(std::unique_ptr<Texture> aTexture) {
    _textures.push_back(std::move(aTexture));
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

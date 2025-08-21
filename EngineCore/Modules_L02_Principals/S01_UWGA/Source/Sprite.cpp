// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Sprite.hpp>

#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/System.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

namespace {
//! Implements the proper mathematical modulus operator which can also handle ther case when a < 0.
//! \warning UB if b <= 0.
int Modulus(int aA, int aB) {
    return (aA % aB + aB) % aB;
}
} // namespace

Sprite::Sprite(const System& aSystem, const Texture* aTexture)
    : Transformable{_createTransform(aSystem)}
    , _system{&aSystem}
    , _texture{aTexture} {}

Sprite::Sprite(const System& aSystem, const Texture* aTexture, const Subsprite& aSubsprite)
    : Transformable{_createTransform(aSystem)}
    , _system{&aSystem}
    , _texture{aTexture} //
{
    addSubsprite(aSubsprite);
}

Sprite::Sprite(const Sprite& aOther)
    : Transformable(aOther)
    , subsprites{aOther.subsprites}
    , _system{aOther._system}
    , _texture{aOther._texture}
    , _subspriteSelector{aOther._subspriteSelector}
    , _color{aOther._color} {}

Sprite& Sprite::operator=(const Sprite& aOther) {
    if (this != &aOther) {
        static_cast<Transformable&>(SELF) = static_cast<const Transformable&>(aOther);

        subsprites = aOther.subsprites;

        _system            = aOther._system;
        _texture           = aOther._texture;
        _subspriteSelector = aOther._subspriteSelector;
        _color             = aOther._color;
    }
    return SELF;
}

Sprite::Sprite(Sprite&& aOther)
    : Transformable(std::move(aOther))
    , subsprites{std::move(aOther.subsprites)}
    , _system{aOther._system}
    , _texture{aOther._texture}
    , _subspriteSelector{aOther._subspriteSelector}
    , _color{aOther._color} {}

Sprite& Sprite::operator=(Sprite&& aOther) {
    if (this != &aOther) {
        static_cast<Transformable&>(SELF) = std::move(static_cast<Transformable&&>(aOther));

        subsprites = std::move(aOther.subsprites);

        _system            = aOther._system;
        _texture           = aOther._texture;
        _subspriteSelector = aOther._subspriteSelector;
        _color             = aOther._color;
    }
    return SELF;
}

const System& Sprite::getSystem() const {
    return *_system;
}

///////////////////////////////////////////////////////////////////////////
// MARK: TEXTURE                                                         //
///////////////////////////////////////////////////////////////////////////

void Sprite::setTexture(const Texture* aTexture) {
    _texture = aTexture;
}

const Texture* Sprite::getTexture() const {
    return _texture;
}

///////////////////////////////////////////////////////////////////////////
// MARK: SUBSPRITES                                                      //
///////////////////////////////////////////////////////////////////////////

Sprite::Subsprite::Subsprite(TextureRect aTextureRect)
    : _textureRect{aTextureRect} {
    // World positions
    {
        const auto bounds = getLocalBounds();

        _vertices[0].position = math::Vector2f{0.f, 0.f};
        _vertices[1].position = math::Vector2f{bounds.w, 0.f};
        _vertices[2].position = math::Vector2f{0.f, bounds.h};
        _vertices[3].position = math::Vector2f{bounds.w, 0.f};
        _vertices[4].position = math::Vector2f{bounds.w, bounds.h};
        _vertices[5].position = math::Vector2f{0.f, bounds.h};
    }

    // Texture positions
    {
        const float left   = static_cast<float>(aTextureRect.getLeft());
        const float right  = static_cast<float>(aTextureRect.getRight());
        const float top    = static_cast<float>(aTextureRect.getTop());
        const float bottom = static_cast<float>(aTextureRect.getBottom());

        _vertices[0].texCoords = math::Vector2f{left, top};
        _vertices[1].texCoords = math::Vector2f{right, top};
        _vertices[2].texCoords = math::Vector2f{left, bottom};
        _vertices[3].texCoords = math::Vector2f{right, top};
        _vertices[4].texCoords = math::Vector2f{right, bottom};
        _vertices[5].texCoords = math::Vector2f{left, bottom};
    }
}

math::Rectangle<float> Sprite::Subsprite::getLocalBounds() const {
    // clang-format off
    return {
        0.f,
        0.f,
        static_cast<float>(_textureRect.w),
        static_cast<float>(_textureRect.h)
    };
    // clang-format on
}

void Sprite::addSubsprite(const Subsprite& aSubsprite) {
    subsprites.emplace_back(aSubsprite);
}

const Sprite::Subsprite& Sprite::getSubsprite(PZInteger aSubspriteIndex) const {
    return subsprites.at(pztos(aSubspriteIndex));
}

void Sprite::removeSubsprite(PZInteger aSubspriteIndex) {
    const auto subspriteCount = getSubspriteCount();

    HG_VALIDATE_ARGUMENT(aSubspriteIndex >= 0 && aSubspriteIndex < subspriteCount,
                         "Subsprite index ({}) out of bounds.",
                         aSubspriteIndex);

    subsprites.erase(subsprites.begin() + aSubspriteIndex);
}

PZInteger Sprite::getSubspriteCount() const {
    return stopz(subsprites.size());
}

float Sprite::getSubspriteSelector() const {
    return _subspriteSelector;
}

PZInteger Sprite::getCurrentSubspriteIndex() const {
    const auto subspriteCount = getSubspriteCount();

    HG_VALIDATE_PRECONDITION(subspriteCount > 0);

    return Modulus(static_cast<int>(std::floor(_subspriteSelector)), subspriteCount);
}

void Sprite::selectSubsprite(int aSubspriteIndex) {
    _subspriteSelector = static_cast<float>(aSubspriteIndex);
}

void Sprite::selectSubsprite(float aSubspriteIndex) {
    _subspriteSelector = aSubspriteIndex;
}

void Sprite::advanceSubsprite(int aSubspriteCount) {
    _subspriteSelector += static_cast<float>(aSubspriteCount);
}

void Sprite::advanceSubsprite(float aSubspriteCount) {
    _subspriteSelector += aSubspriteCount;
}

///////////////////////////////////////////////////////////////////////////
// MARK: COLOR                                                           //
///////////////////////////////////////////////////////////////////////////

void Sprite::setColor(Color aColor) {
    _color = aColor;
}

Color Sprite::getColor() const {
    return _color;
}

///////////////////////////////////////////////////////////////////////////
// MARK: BOUNDS                                                          //
///////////////////////////////////////////////////////////////////////////

math::Rectangle<float> Sprite::getLocalBounds(PZInteger aSubspriteIndex) const {
    const auto subspriteCount = getSubspriteCount();

    HG_VALIDATE_ARGUMENT(aSubspriteIndex >= 0 && aSubspriteIndex < subspriteCount,
                         "Subsprite index ({}) out of bounds.",
                         aSubspriteIndex);

    return subsprites[pztos(aSubspriteIndex)].getLocalBounds();
}

math::Rectangle<float> Sprite::getLocalBounds() const {
    math::Rectangle<float> rect; // default-initializes to all zeros

    if (!subsprites.empty()) {
        rect = subsprites[pztos(getCurrentSubspriteIndex())].getLocalBounds();
    }

    return rect;
}

math::Rectangle<float> Sprite::getGlobalBounds(PZInteger aSubspriteIndex) const {
    const auto subspriteCount = getSubspriteCount();

    HG_VALIDATE_ARGUMENT(aSubspriteIndex >= 0 && aSubspriteIndex < subspriteCount,
                         "Subsprite index ({}) out of bounds.",
                         aSubspriteIndex);

    return getTransform().transformRect(subsprites[pztos(aSubspriteIndex)].getLocalBounds());
}

math::Rectangle<float> Sprite::getGlobalBounds() const {
    math::Rectangle<float> rect; // default-initializes to all zeros

    if (!subsprites.empty()) {
        rect = subsprites[pztos(getCurrentSubspriteIndex())].getLocalBounds();
    }

    return getTransform().transformRect(rect);
}

bool Sprite::isNormalized() const {
    const auto subspriteCount = pztos(getSubspriteCount());

    for (std::size_t i = 1; i < subspriteCount; i += 1) {
        if (subsprites[i].getLocalBounds() != subsprites[i - 1].getLocalBounds()) {
            return false;
        }
    }

    return true;
}

bool Sprite::areAllSubspritesOfSameSize() const {
    return isNormalized();
}

///////////////////////////////////////////////////////////////////////////
// MARK: DRAW                                                            //
///////////////////////////////////////////////////////////////////////////

void Sprite::drawOnto(Canvas& aCanvas, const RenderStates& aRenderStates) const {
    assert(&aCanvas.getSystem() == &getSystem());

    if (subsprites.empty() || _texture == nullptr) {
        return;
    }

    const auto& subspr = subsprites[pztos(getCurrentSubspriteIndex())];

    // Prepare vertices
    std::array<Vertex, Subsprite::VERTEX_COUNT> vertices;
    std::memcpy(vertices.data(), subspr._vertices.data(), sizeof(vertices));

    const auto& transform = getTransform();
    for (auto& vertex : vertices) {
        vertex.color    = _color;
        vertex.position = transform.transformPoint(vertex.position);
    }

    const auto renderStates = RenderStates{.texture   = _texture,
                                           .shader    = aRenderStates.shader,
                                           .transform = aRenderStates.transform,
                                           .blendMode = aRenderStates.blendMode};

    aCanvas.draw(vertices.data(),
                 stopz(Subsprite::VERTEX_COUNT),
                 PrimitiveType::TRIANGLES,
                 getAnchor(),
                 renderStates);
}

///////////////////////////////////////////////////////////////////////////
// MARK: PRIVATE                                                         //
///////////////////////////////////////////////////////////////////////////

std::unique_ptr<Transform> Sprite::_createTransform(const System& aSystem) {
    return aSystem.createTransform();
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

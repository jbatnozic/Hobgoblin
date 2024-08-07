// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off


#include <Hobgoblin/Graphics/Sprite.hpp>
#include <Hobgoblin/HGExcept.hpp>

#include <Hobgoblin/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "Draw_bridge.hpp"
#include "SFML_conversions.hpp"

#include <cassert>
#include <new>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

using ImplType = sf::Sprite;
constexpr auto IMPL_SIZE  = sizeof(ImplType);
constexpr auto IMPL_ALIGN = alignof(ImplType);
#define  IMPLOF(_obj_) (reinterpret_cast<ImplType*>(&((_obj_)._storage)))
#define CIMPLOF(_obj_) (reinterpret_cast<const ImplType*>(&((_obj_)._storage)))
#define  SELF_IMPL (IMPLOF(SELF))
#define SELF_CIMPL (CIMPLOF(SELF))

static const sf::Texture* DUMMY_SFML_TEXTURE = reinterpret_cast<const sf::Texture*>(0x12345678);

Sprite::Sprite() {
    static_assert(STORAGE_SIZE  == IMPL_SIZE,  "Sprite::STORAGE_SIZE is inadequate.");
    static_assert(STORAGE_ALIGN == IMPL_ALIGN, "Sprite::STORAGE_ALIGN is inadequate.");

    new (&_storage) ImplType();
}

Sprite::Sprite(const Sprite& aOther) {
    new (&_storage) ImplType(*CIMPLOF(aOther));
    _texture = aOther._texture;
}

Sprite& Sprite::operator=(const Sprite& aOther) {
    if (this != &aOther) {
        *SELF_IMPL = *CIMPLOF(aOther);
        _texture = aOther._texture;
    }
    return SELF;
}

Sprite::Sprite(const Texture* aTexture) {
    if (aTexture) {
        new (&_storage) ImplType(ToSf(*aTexture));
    }
    else {
        new (&_storage) ImplType();
    }
    _texture = aTexture;
}

Sprite::Sprite(const Texture* aTexture, TextureRect aTextureRect) {
    if (aTexture) {
        new (&_storage) ImplType(ToSf(*aTexture), ConvertTextureRect(aTextureRect));
    }
    else {
        new (&_storage) ImplType(*DUMMY_SFML_TEXTURE, ConvertTextureRect(aTextureRect));
    }
    _texture = aTexture;
}

Sprite::~Sprite() {
    SELF_IMPL->~ImplType();
}

void Sprite::setTexture(const Texture* aTexture, bool aResetRect) {
    if (aTexture) {
        SELF_IMPL->setTexture(ToSf(*aTexture), aResetRect);
    }
    else {
        HG_VALIDATE_ARGUMENT(!aResetRect, "Cannot infer size from null texture.");
        SELF_IMPL->setTexture(*DUMMY_SFML_TEXTURE, aResetRect);
    }
    _texture = aTexture;
}

void Sprite::setTextureRect(TextureRect aTextureRect) {
    SELF_IMPL->setTextureRect(ConvertTextureRect(aTextureRect));
}

void Sprite::setColor(const Color& aColor) {
    SELF_IMPL->setColor(ToSf(aColor));
}

const Texture* Sprite::getTexture() const {
    return _texture;
}

TextureRect Sprite::getTextureRect() const {
    const auto rect = SELF_CIMPL->getTextureRect();
    return {
        static_cast<std::uint16_t>(rect.left),
        static_cast<std::uint16_t>(rect.top),
        static_cast<std::uint16_t>(rect.width),
        static_cast<std::uint16_t>(rect.height)
    };
}

Color Sprite::getColor() const {
    return ToHg(SELF_CIMPL->getColor());
}

math::Rectangle<float> Sprite::getLocalBounds() const {
    const auto bounds = SELF_CIMPL->getLocalBounds();
    return {bounds.left, bounds.top, bounds.width, bounds.height};
}

math::Rectangle<float> Sprite::getGlobalBounds() const {
    const auto bounds = SELF_CIMPL->getGlobalBounds();
    return {bounds.left, bounds.top, bounds.width, bounds.height};
}

void Sprite::_drawOnto(Canvas& aCanvas, const RenderStates& aStates) const {
    if (_texture == nullptr) {
        return;
    }

    const auto drawingWasSuccessful = 
        Draw(aCanvas, [this, &aStates](sf::RenderTarget& aSfRenderTarget) {
            aSfRenderTarget.draw(*CIMPLOF(*this), ToSf(aStates));
        });
    assert(drawingWasSuccessful);
}

Drawable::BatchingType Sprite::getBatchingType() const {
    return Drawable::BatchingType::Sprite;
}

///////////////////////////////////////////////////////////////////////////
// TRANSFORMABLE                                                         //
///////////////////////////////////////////////////////////////////////////

void Sprite::setPosition(float aX, float aY) {
    SELF_IMPL->setPosition(aX, aY);
}

void Sprite::setPosition(const math::Vector2f& aPosition) {
    SELF_IMPL->setPosition(ToSf(aPosition));
}

void Sprite::setRotation(math::AngleF aAngle) {
    SELF_IMPL->setRotation(-aAngle.asDeg());
}

void Sprite::setScale(float aFactorX, float aFactorY) {
    SELF_IMPL->setScale(aFactorX, aFactorY);
}

void Sprite::setScale(const math::Vector2f& aFactors) {
    SELF_IMPL->setScale(ToSf(aFactors));
}

void Sprite::setOrigin(float aX, float aY) {
    SELF_IMPL->setOrigin(aX, aY);
}

void Sprite::setOrigin(const math::Vector2f& aOrigin) {
    SELF_IMPL->setOrigin(ToSf(aOrigin));
}

math::Vector2f Sprite::getPosition() const {
    return ToHg(SELF_CIMPL->getPosition());
}

math::AngleF Sprite::getRotation() const {
    return math::AngleF::fromDegrees(-(SELF_CIMPL->getRotation()));
}

math::Vector2f Sprite::getScale() const {
    return ToHg(SELF_CIMPL->getScale());
}

math::Vector2f Sprite::getOrigin() const {
    return ToHg(SELF_CIMPL->getOrigin());
}

void Sprite::move(float aOffsetX, float aOffsetY) {
    SELF_IMPL->move(aOffsetX, aOffsetY);
}

void Sprite::move(const math::Vector2f& aOffset) {
    SELF_IMPL->move(ToSf(aOffset));
}

void Sprite::rotate(math::AngleF aAngle) {
    SELF_IMPL->rotate(-aAngle.asDeg());
}

void Sprite::scale(float aFactorX, float aFactorY) {
    SELF_IMPL->scale(aFactorX, aFactorY);
}

void Sprite::scale(const math::Vector2f& aFactor) {
    SELF_IMPL->scale(ToSf(aFactor));
}

Transform Sprite::getTransform() const {
    return ToHg(SELF_CIMPL->getTransform());
}

Transform Sprite::getInverseTransform() const {
    return ToHg(SELF_CIMPL->getInverseTransform());
}

///////////////////////////////////////////////////////////////////////////
// SPRITE - PRIVATE                                                      //
///////////////////////////////////////////////////////////////////////////

void* Sprite::_getSFMLImpl() {
    return SELF_IMPL;
}

const void* Sprite::_getSFMLImpl() const {
    return SELF_CIMPL;
}

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// clang-format on

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Transform_impl.hpp"

#include "SFML_conversions.hpp"

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

SFMLTransformImpl::SFMLTransformImpl(const System& aSystem)
    : _system{&aSystem} {}

SFMLTransformImpl::SFMLTransformImpl(const System& aSystem, const sf::Transform& aSfmlTransform)
    : _system{&aSystem}
    , _renderStates{aSfmlTransform} {}

sf::RenderStates& SFMLTransformImpl::getUnderlyingRenderStates() const {
    return _renderStates;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Element                                                         //
///////////////////////////////////////////////////////////////////////////

const System& SFMLTransformImpl::getSystem() const {
    return *_system;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Transform                                                       //
///////////////////////////////////////////////////////////////////////////

std::unique_ptr<Transform> SFMLTransformImpl::clone() const {
    return std::make_unique<SFMLTransformImpl>(*_system, _getTransform());
}

// Setters

// clang-format off
void SFMLTransformImpl::setTo3x3Matrix(float a00, float a01, float a02,
                                       float a10, float a11, float a12,
                                       float a20, float a21, float a22) {
    _getTransform() = sf::Transform{a00, a01, a02, 
                                    a10, a11, a12,
                                    a20, a21, a22};
}
// clang-format on

void SFMLTransformImpl::setToIdentity() {
    _getTransform() = sf::Transform::Identity;
}

void SFMLTransformImpl::setToCopyOf(const Transform& aOther) {
    assert(&aOther.getSystem() == _system);

    const auto& otherImpl = static_cast<const SFMLTransformImpl&>(aOther);

    _getTransform() = otherImpl._getTransform();
}

void SFMLTransformImpl::setToInverse() {
    _getTransform() = _getTransform().getInverse();
}

void SFMLTransformImpl::setToInverseOf(const Transform& aOther) {
    assert(&aOther.getSystem() == _system);

    const auto& otherImpl = static_cast<const SFMLTransformImpl&>(aOther);

    _getTransform() = otherImpl._getTransform().getInverse();
}

// Applying to objects

math::Vector2f SFMLTransformImpl::transformPoint(float aX, float aY) const {
    return ToHg(_getTransform().transformPoint(aX, aY));
}

math::Vector2f SFMLTransformImpl::transformPoint(const math::Vector2f& aPoint) const {
    return ToHg(_getTransform().transformPoint(aPoint.x, aPoint.y));
}

math::Rectangle<float> SFMLTransformImpl::transformRect(const math::Rectangle<float>& aRectangle) const {
    const auto rect = _getTransform().transformRect({
        aRectangle.getLeft(),
        aRectangle.getTop(),
        aRectangle.w,
        aRectangle.h,
    });
    return {rect.left, rect.top, rect.width, rect.height};
}

// Mutators

Transform& SFMLTransformImpl::combine(const Transform& aOther) {
    assert(&aOther.getSystem() == _system);

    const auto& otherImpl = static_cast<const SFMLTransformImpl&>(aOther);
    _getTransform().combine(otherImpl._getTransform());

    return SELF;
}

Transform& SFMLTransformImpl::translate(float aX, float aY) {
    _getTransform().translate(aX, aY);
    return SELF;
}

Transform& SFMLTransformImpl::translate(math::Vector2f aOffset) {
    _getTransform().translate(aOffset.x, aOffset.y);
    return SELF;
}

Transform& SFMLTransformImpl::rotate(float aAngle) {
    _getTransform().rotate(aAngle);
    return SELF;
}

Transform& SFMLTransformImpl::rotate(math::AngleF aAngle) {
    _getTransform().rotate(aAngle.asDegrees());
    return SELF;
}

Transform& SFMLTransformImpl::rotate(float aAngle, float aCenterX, float aCenterY) {
    _getTransform().rotate(aAngle, aCenterX, aCenterY);
    return SELF;
}

Transform& SFMLTransformImpl::rotate(math::AngleF aAngle, float aCenterX, float aCenterY) {
    _getTransform().rotate(aAngle.asDegrees(), aCenterX, aCenterY);
    return SELF;
}

Transform& SFMLTransformImpl::rotate(float aAngle, math::Vector2f aCenter) {
    _getTransform().rotate(aAngle, aCenter.x, aCenter.y);
    return SELF;
}

Transform& SFMLTransformImpl::rotate(math::AngleF aAngle, math::Vector2f aCenter) {
    _getTransform().rotate(aAngle.asDegrees(), aCenter.x, aCenter.y);
    return SELF;
}

Transform& SFMLTransformImpl::scale(float aScaleX, float aScaleY) {
    _getTransform().scale(aScaleX, aScaleY);
    return SELF;
}

Transform& SFMLTransformImpl::scale(float aScaleX, float aScaleY, float aCenterX, float aCenterY) {
    _getTransform().scale(aScaleX, aScaleY, aCenterX, aCenterY);
    return SELF;
}

Transform& SFMLTransformImpl::scale(const math::Vector2f& aFactors) {
    _getTransform().scale(aFactors.x, aFactors.y);
    return SELF;
}

Transform& SFMLTransformImpl::scale(const math::Vector2f& aFactors, const math::Vector2f& aCenter) {
    _getTransform().scale(aFactors.x, aFactors.y, aCenter.x, aCenter.y);
    return SELF;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Private                                                         //
///////////////////////////////////////////////////////////////////////////

sf::Transform& SFMLTransformImpl::_getTransform() {
    return _renderStates.transform;
}

const sf::Transform& SFMLTransformImpl::_getTransform() const {
    return _renderStates.transform;
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

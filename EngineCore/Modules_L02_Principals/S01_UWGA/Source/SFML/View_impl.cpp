// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "View_impl.hpp"

#include "SFML_conversions.hpp"

#include <SFML/Graphics/View.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

SFMLViewImpl::SFMLViewImpl(const System& aSystem)
    : _system{&aSystem} {}

SFMLViewImpl::SFMLViewImpl(const System& aSystem, const sf::View& aView, math::Vector2d aAnchor)
    : _system{&aSystem}, _view{aView}, _anchor{aAnchor} {}

std::unique_ptr<View> SFMLViewImpl::clone() const {
    return std::make_unique<SFMLViewImpl>(*_system, _view, _anchor);
}

const sf::View& SFMLViewImpl::getUnderlyingView() const {
    return _view;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Element                                                         //
///////////////////////////////////////////////////////////////////////////

const System& SFMLViewImpl::getSystem() const {
    return *_system;
}

///////////////////////////////////////////////////////////////////////////
// MARK: View                                                            //
///////////////////////////////////////////////////////////////////////////

// Size

void SFMLViewImpl::setSize(math::Vector2f aSize) {
    _view.setSize(aSize.x, aSize.y);
}

void SFMLViewImpl::setSize(float aWidth, float aHeight) {
    _view.setSize(aWidth, aHeight);
}

math::Vector2f SFMLViewImpl::getSize() const {
    return ToHg(_view.getSize());
}

// Viewport

void SFMLViewImpl::setViewport(const math::Rectangle<float>& aViewport) {
    _view.setViewport(ToSf(aViewport));
}

math::Rectangle<float> SFMLViewImpl::getViewport() const {
    return ToHg(_view.getViewport());
}

// Anchor

void SFMLViewImpl::setAnchor(math::Vector2d aAnchor) {
    _anchor = aAnchor;
}

void SFMLViewImpl::setAnchor(double aX, double aY) {
    _anchor.x = aX;
    _anchor.y = aY;
}

math::Vector2d SFMLViewImpl::getAnchor() const {
    return _anchor;
}

// Center

void SFMLViewImpl::setCenter(math::Vector2f aCenter) {
    _view.setCenter(aCenter.x, aCenter.y);
}

void SFMLViewImpl::setCenter(float aX, float aY) {
    _view.setCenter(aX, aY);
}

math::Vector2f SFMLViewImpl::getCenter() const {
    return ToHg(_view.getCenter());
}

// Rotation

void SFMLViewImpl::setRotation(math::AngleF aAngle) {
    _view.setRotation(-aAngle.asDegrees());
}

void SFMLViewImpl::rotate(math::AngleF aAngle) {
    _view.rotate(-aAngle.asDegrees());
}

math::AngleF SFMLViewImpl::getRotation() const {
    return math::AngleF::fromDegrees(-_view.getRotation());
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

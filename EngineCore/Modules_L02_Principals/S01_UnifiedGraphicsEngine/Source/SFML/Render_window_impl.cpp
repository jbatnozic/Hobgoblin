// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Render_window_impl.hpp"

#include "SFML_conversions.hpp"
#include "SFML_vertices.hpp"
#include "SFML_window_event_conversion.hpp"
#include "View_impl.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <SFML/Window/Event.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

namespace {
sf::String UnicodeStringToSfmlString(const UnicodeString& aUnicodeString) {
    sf::String sfmlString;
    hobgoblin::detail::StoreUStringInSfString(aUnicodeString, &sfmlString);
    return sfmlString;
}
} // namespace

SFMLRenderWindowImpl::SFMLRenderWindowImpl(const System&         aSystem,
                                           const win::VideoMode& aVideoMode,
                                           const UnicodeString&  aTitle,
                                           win::WindowStyle      aStyle)
    : _system{aSystem}
    , _window{sf::VideoMode{(unsigned)aVideoMode.width,
                            (unsigned)aVideoMode.height,
                            (unsigned)aVideoMode.bitsPerPixel},
              UnicodeStringToSfmlString(aTitle)}
    , _activeView{_system, _window.getDefaultView(), {0.0, 0.0}} {}

///////////////////////////////////////////////////////////////////////////
// MARK: Element                                                         //
///////////////////////////////////////////////////////////////////////////

const System& SFMLRenderWindowImpl::getSystem() const {
    return _system;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Window                                                          //
///////////////////////////////////////////////////////////////////////////

// Title

void SFMLRenderWindowImpl::setTitle(const std::string& aTitle) {
    _window.setTitle(aTitle);
}

// Event handling

bool SFMLRenderWindowImpl::pollEvent(WindowEvent& aEvent) {
    sf::Event  ev;
    const bool success = _window.pollEvent(ev);
    if (success) {
        aEvent = ToHg(ev);
    }
    return success;
}

bool SFMLRenderWindowImpl::waitEvent(WindowEvent& aEvent) {
    sf::Event  ev;
    const bool success = _window.waitEvent(ev);
    if (success) {
        aEvent = ToHg(ev);
    }
    return success;
}

// Size

void SFMLRenderWindowImpl::setSize(math::Vector2pz aSize) {
    _window.setSize({static_cast<unsigned>(aSize.x), static_cast<unsigned>(aSize.y)});
}

math::Vector2pz SFMLRenderWindowImpl::getSize() const {
    const auto size = _window.getSize();
    return {ToPz(size.x), ToPz(size.y)};
}

// Position

void SFMLRenderWindowImpl::setPosition(math::Vector2i aPosition) {
    _window.setPosition({aPosition.x, aPosition.y});
}

math::Vector2i SFMLRenderWindowImpl::getPosition() const {
    const auto pos = _window.getPosition();
    return {pos.x, pos.y};
}

// Focus

void SFMLRenderWindowImpl::requestFocus() {
    _window.requestFocus();
}

bool SFMLRenderWindowImpl::hasFocus() const {
    return _window.hasFocus();
}

// Cursor

void SFMLRenderWindowImpl::setMouseCursorVisible(bool aVisible) {
    _window.setMouseCursorVisible(aVisible);
}

void SFMLRenderWindowImpl::setMouseCursorGrabbed(bool aGrabbed) {
    _window.setMouseCursorGrabbed(aGrabbed);
}

// Contents

void SFMLRenderWindowImpl::display() {
    _window.display();
}

// Other

void SFMLRenderWindowImpl::setKeyRepeatEnabled(bool aEnabled) {
    _window.setKeyRepeatEnabled(aEnabled);
}

void SFMLRenderWindowImpl::setVisible(bool aVisible) {
    _window.setVisible(aVisible);
}

void SFMLRenderWindowImpl::setVerticalSyncEnabled(bool aEnabled) {
    _window.setVerticalSyncEnabled(aEnabled);
}

void SFMLRenderWindowImpl::setFramerateLimit(PZInteger aLimit) {
    _window.setFramerateLimit(static_cast<unsigned>(aLimit));
}

///////////////////////////////////////////////////////////////////////////
// MARK: Canvas                                                          //
///////////////////////////////////////////////////////////////////////////

// Views

void SFMLRenderWindowImpl::setView(const View& aView) {
    assert(&aView.getSystem() == &getSystem());

    const auto& viewImpl = static_cast<const SFMLViewImpl&>(aView);
    _window.setView(viewImpl.getUnderlyingView());
    _activeView       = viewImpl;
    _activeViewAnchor = aView.getAnchor();
}

void SFMLRenderWindowImpl::setDefaultView() {
    _window.setView(_window.getDefaultView());
}

const View& SFMLRenderWindowImpl::getView() const {
    return _activeView;
}

const sf::View& SFMLRenderWindowImpl::getDefaultView() const {
    return _window.getDefaultView();
}

// Drawing

void SFMLRenderWindowImpl::clear(Color aColor) {
    _window.clear(ToSf(aColor));
}

void SFMLRenderWindowImpl::draw(const Vertex*      aVertices,
                                PZInteger          aVertexCount,
                                PrimitiveType      aPrimitiveType,
                                math::Vector2d     aAnchor,
                                RenderStatesOptRef aStates) {
    SFMLVertices sfVertices{aVertices, pztos(aVertexCount)};

    if (aStates) {
        HG_NOT_IMPLEMENTED();
    } else {
        auto* transformMatrix = const_cast<float*>(_defaultRenderStates.transform.getMatrix());
        transformMatrix[12]   = static_cast<float>(aAnchor.x - _activeViewAnchor.x);
        transformMatrix[13]   = static_cast<float>(aAnchor.y - _activeViewAnchor.y);

        _window.draw(sfVertices.getVertices(),
                     sfVertices.getVertexCount(),
                     ToSf(aPrimitiveType),
                     _defaultRenderStates);
    }
}

void SFMLRenderWindowImpl::flush() {
    /* Nothing to do (there is no batching in SFML). */
}

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

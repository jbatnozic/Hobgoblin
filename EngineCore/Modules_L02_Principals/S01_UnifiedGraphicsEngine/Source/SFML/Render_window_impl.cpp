// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Render_window_impl.hpp"

#include "SFML_conversions.hpp"
#include "SFML_vertices.hpp"
#include "SFML_window_event_conversion.hpp"

#include <SFML/Window/Event.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

SFMLRenderWindowImpl::SFMLRenderWindowImpl(const System&         aSystem,
                                           const win::VideoMode& aVideoMode,
                                           const UnicodeString&  aTitle,
                                           win::WindowStyle      aStyle)
    : _system{aSystem} //
{
    sf::String sfTitle;
    hobgoblin::detail::StoreUStringInSfString(aTitle, &sfTitle);
    _window.create(sf::VideoMode{(unsigned)aVideoMode.width,
                                 (unsigned)aVideoMode.height,
                                 (unsigned)aVideoMode.bitsPerPixel},
                   sfTitle);
}

///////////////////////////////////////////////////////////////////////////
// MARK: Element                                                         //
///////////////////////////////////////////////////////////////////////////

const System& SFMLRenderWindowImpl::getSystem() const {
    return _system;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Window                                                          //
///////////////////////////////////////////////////////////////////////////

bool SFMLRenderWindowImpl::pollEvent(WindowEvent& aEvent) {
    sf::Event  ev;
    const bool success = _window.pollEvent(ev);
    if (success) {
        aEvent = ToHg(ev);
    }
    return success;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Canvas                                                          //
///////////////////////////////////////////////////////////////////////////

math::Vector2pz SFMLRenderWindowImpl::getSize() const {
    const auto size = _window.getSize();
    return {ToPz(size.x), ToPz(size.y)};
}

void SFMLRenderWindowImpl::clear(const Color& aColor) {
    _window.clear(ToSf(aColor));
}

void SFMLRenderWindowImpl::draw(const Vertex*       aVertices,
                                PZInteger           aVertexCount,
                                PrimitiveType       aPrimitiveType,
                                math::Vector2d      aAnchor,
                                const RenderStates& aStates) {
    SFMLVertices sfVertices{aVertices, pztos(aVertexCount)};
    _window.draw(sfVertices.getVertices(), sfVertices.getVertexCount(), ToSf(aPrimitiveType));
}

void SFMLRenderWindowImpl::flush() {
    /* Nothing to do (there is no batching in SFML). */
}

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

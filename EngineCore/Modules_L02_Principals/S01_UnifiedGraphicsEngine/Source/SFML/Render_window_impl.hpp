// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_SFML_RENDER_WINDOW_IMPL_HPP
#define UHOBGOBLIN_UGE_SFML_RENDER_WINDOW_IMPL_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/Render_window.hpp>
#include <Hobgoblin/Window/Video_mode.hpp>
#include <Hobgoblin/Window/Window_style.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class System;

class SFMLRenderWindowImpl : public RenderWindow {
public:
    SFMLRenderWindowImpl(const System&         aSystem,
                         const win::VideoMode& aVideoMode,
                         const UnicodeString&  aTitle = HG_UNISTR("Window"),
                         win::WindowStyle      aStyle = win::WindowStyle::Default);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Window                                                          //
    ///////////////////////////////////////////////////////////////////////////

    bool pollEvent(WindowEvent& aEvent) override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Canvas                                                          //
    ///////////////////////////////////////////////////////////////////////////

    math::Vector2pz getSize() const override;

    void clear(const Color& aColor = COLOR_BLACK) override;

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates = RENDER_STATES_DEFAULT) override;

    void flush() override;

    virtual void* getRenderingBackend() override {
        return nullptr;
    }

private:
    const System&    _system;
    sf::RenderWindow _window;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UGE_SFML_RENDER_WINDOW_IMPL_HPP

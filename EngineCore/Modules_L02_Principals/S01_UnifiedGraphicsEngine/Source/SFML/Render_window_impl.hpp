// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_SFML_RENDER_WINDOW_IMPL_HPP
#define UHOBGOBLIN_UGE_SFML_RENDER_WINDOW_IMPL_HPP

#include <Hobgoblin/Unicode.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Blend_mode.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Render_states.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Render_window.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Shader.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Window_style.hpp>

#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "View_impl.hpp"

#include <optional>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class System;

class SFMLRenderWindowImpl : public RenderWindow {
public:
    SFMLRenderWindowImpl(const System&        aSystem,
                         PZInteger            aWidth,
                         PZInteger            aHeight,
                         WindowStyle          aStyle,
                         const UnicodeString& aTitle);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Window                                                          //
    ///////////////////////////////////////////////////////////////////////////

    // Title

    void setTitle(const UnicodeString& aTitle) override;

    // Event handling

    bool pollEvent(WindowEvent& aEvent) override;
    bool waitEvent(WindowEvent& aEvent) override;

    // Size

    void setSize(math::Vector2pz aSize) override;
    math::Vector2pz getSize() const override;

    // Position

    void setPosition(math::Vector2i aPosition) override;
    math::Vector2i getPosition() const override;

    // Focus

    void requestFocus() override;
    bool hasFocus() const override;

    // Cursor

    void setMouseCursorVisible(bool aVisible) override;
    void setMouseCursorGrabbed(bool aGrabbed) override;

    // Contents

    void display() override;

    // Other

    void setKeyRepeatEnabled(bool aEnabled) override;
    void setVisible(bool aVisible) override;
    void setVerticalSyncEnabled(bool aEnabled) override;
    void setFramerateLimit(PZInteger aLimit) override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Canvas                                                          //
    ///////////////////////////////////////////////////////////////////////////

    // math::Vector2pz getSize() const override; -- see section 'Window'

    // Viwes

    void setView(const View& aView) override;

    void setDefaultView() override;

    const View& getView() const override;

    const sf::View& getDefaultView() const;

    // Drawing

    void clear(Color aColor = COLOR_BLACK) override;

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates = RENDER_STATES_DEFAULT) override;

    void flush() override;

private:
    const System&    _system;
    sf::RenderWindow _window;
    sf::RenderStates _defaultRenderStates = sf::RenderStates::Default;
    SFMLViewImpl     _activeView;
    math::Vector2d   _activeViewAnchor;

    const sf::Texture*         _getSfmlTexture(const Texture* aTexture) const;
    const sf::Shader*          _getSfmlShader(const Shader* aShader) const;
    static const sf::BlendMode _getSfmlBlendMode(std::optional<BlendMode> aBlendMode);
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UGE_SFML_RENDER_WINDOW_IMPL_HPP

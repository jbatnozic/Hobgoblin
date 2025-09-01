// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_RENDER_WINDOW_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_RENDER_WINDOW_IMPL_HPP

#include <Hobgoblin/UWGA/Blend_mode.hpp>
#include <Hobgoblin/UWGA/Render_states.hpp>
#include <Hobgoblin/UWGA/Render_window.hpp>
#include <Hobgoblin/UWGA/Shader.hpp>
#include <Hobgoblin/UWGA/Window_style.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "SFML/View_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

class SFMLRenderWindowImpl : public RenderWindow {
public:
    SFMLRenderWindowImpl(const System&        aSystem,
                         PZInteger            aWidth,
                         PZInteger            aHeight,
                         WindowStyle          aStyle,
                         const UnicodeString& aTitle,
                         bool                 aEnableSRgb);

    const sf::RenderWindow& getUnderlyingRenderWindow() const;

    sf::RenderWindow& getUnderlyingRenderWindow();

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

    void            setSize(math::Vector2pz aSize) override;
    math::Vector2pz getSize() const override;

    // Position

    void           setPosition(math::Vector2i aPosition) override;
    math::Vector2i getPosition() const override;

    // Focus

    void requestFocus() override;
    bool hasFocus() const override;

    // Cursor

    void setMouseCursorVisible(bool aVisible) override;
    void setMouseCursorGrabbed(bool aGrabbed) override;

    math::Vector2f getRelativeCursorPosition() const override;

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

    const View& getView() const override;

    void setDefaultView() override;

    std::unique_ptr<View> createDefaultView() const override;

    math::Rectangle<int> viewportToPixels(const View& aView) const override;

    // Drawing

    void clear(Color aColor = COLOR_BLACK) override;

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates = RENDER_STATES_DEFAULT) override;

    void flush() override;

    const PerformanceCounters& getPerformanceCounters() const override;

    void resetPerformanceCounters() override;

    PerformanceCounters getAndResetPerformanceCounters() override;

private:
    const System&       _system;
    sf::RenderWindow    _window;
    sf::RenderStates    _defaultRenderStates = sf::RenderStates::Default;
    SFMLViewImpl        _activeView;
    math::Vector2d      _activeViewAnchor;
    PerformanceCounters _perfCnt;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_RENDER_WINDOW_IMPL_HPP

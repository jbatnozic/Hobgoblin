// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_MANAGERS_WINDOW_MANAGER_DEFAULT_HPP
#define SPEMPE_MANAGERS_WINDOW_MANAGER_DEFAULT_HPP

#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/RmlUi.hpp>
#include <Hobgoblin/UWGA/Render_texture.hpp>
#include <Hobgoblin/UWGA/Render_window.hpp>
#include <Hobgoblin/UWGA/Sprite.hpp>
#include <Hobgoblin/UWGA/System.hpp>
#include <Hobgoblin/Utility/Time_utils.hpp>

#include <SPeMPE/GameObjectFramework/Game_object_bases.hpp>
#include <SPeMPE/Managers/Window_manager_interface.hpp>
#include <SPeMPE/Utility/Timing.hpp>
#include <SPeMPE/Utility/Window_frame_input_view.hpp>
#include <SPeMPE/Utility/Window_input_tracker.hpp>

#include <memory>

namespace jbatnozic {
namespace spempe {

namespace hg = ::jbatnozic::hobgoblin;

class DefaultWindowManager
    : public WindowManagerInterface
    , public NonstateObject {
public:
    DefaultWindowManager(hobgoblin::QAO_InstGuard aInstGuard, int aExecutionPriority);

    ///////////////////////////////////////////////////////////////////////////
    // CONFIGURATION                                                         //
    ///////////////////////////////////////////////////////////////////////////

    void setToHeadlessMode(const TimingConfig& aTimingConfig) override;

    void setToNormalMode(hg::AvoidNull<std::shared_ptr<hg::uwga::System>> aGraphicsSystem,
                         const WindowConfig&                              aWindowConfig,
                         const MainRenderTextureConfig&                   aMainRenderTextureConfig,
                         const TimingConfig&                              aTimingConfig) override;

    Mode getMode() const override;

    ///////////////////////////////////////////////////////////////////////////
    // WINDOW MANAGEMENT                                                     //
    ///////////////////////////////////////////////////////////////////////////

    hg::math::Vector2pz getWindowSize() const override;

    void setStopIfCloseClicked(bool aStop) override;

    ///////////////////////////////////////////////////////////////////////////
    // GRAPHICS & DRAWING                                                    //
    ///////////////////////////////////////////////////////////////////////////

    hg::uwga::System& getGraphicsSystem() const override;

    const hg::uwga::RenderWindow& getWindow() const override;

    const hg::uwga::RenderTexture& getMainRenderTexture() const override;

    hg::uwga::Canvas& getActiveCanvas() override;

    void setMainRenderTextureDrawPosition(DrawPosition aDrawPosition) override;

    ///////////////////////////////////////////////////////////////////////////
    // VIEWS                                                                 //
    ///////////////////////////////////////////////////////////////////////////

    hg::math::Vector2d mapPixelToCoords(const hg::math::Vector2f& aPoint,
                                        const hg::uwga::View&     aView) const override;

    hg::math::Vector2d mapPixelToCoords(const hg::math::Vector2f& aPoint) const override;

    hg::math::Vector2f mapCoordsToPixel(const hg::math::Vector2d& aPoint,
                                        const hg::uwga::View&     aView) const override;

    hg::math::Vector2f mapCoordsToPixel(const hg::math::Vector2d& aPoint) const override;

    ///////////////////////////////////////////////////////////////////////////
    // GUI                                                                   //
    ///////////////////////////////////////////////////////////////////////////

    Rml::Context& getGUIContext() override;

    ///////////////////////////////////////////////////////////////////////////
    // INPUT                                                                 //
    ///////////////////////////////////////////////////////////////////////////

    WindowFrameInputView getInput() const override;

private:
    struct MainRenderTexturePositioningData {
        sf::Vector2f position;
        sf::Vector2f origin;
        sf::Vector2f scale;
    };

    // Configuration:
    Mode         _mode = Mode::UNINITIALIZED;
    TimingConfig _timingConfig;

    // Graphics system:
    std::shared_ptr<hg::uwga::System> _graphicsSystem;

    // Window management:
    std::unique_ptr<hg::uwga::RenderWindow> _window;
    std::optional<hg::uwga::Color>          _windowClearingColor;
    hg::util::Stopwatch                     _timeSinceLastDisplay;
    bool                                    _stopIfCloseClicked = false;

    std::vector<hg::uwga::WindowEvent> _events;

    // Main render texture:
    std::unique_ptr<hg::uwga::RenderTexture> _mainRenderTexture;
    std::optional<hg::uwga::Color>           _mrtClearingColor;
    std::unique_ptr<hg::uwga::Sprite>        _mrtSprite;

    DrawPosition _mainRenderTextureDrawPos = DrawPosition::FIT;

    // GUI:
    std::unique_ptr<hg::rml::HobgoblinBackend::BackendLifecycleGuard> _rmlUiBackendLifecycleGuard;
    std::optional<hg::rml::ContextDriver>                             _rmlUiContextDriver;

    // Input:
    detail::WindowInputTracker _inputTracker;

    bool _frameReady = false;

    void _eventPreUpdate() override;
    void _eventPreDraw() override;
    void _eventDraw2() override;
    void _eventDrawGUI() override;
    void _eventDisplay() override;

    MainRenderTexturePositioningData _getMainRenderTexturePositioningData() const;

    void _drawMainRenderTexture();
    void _displayWindowAndPollEvents();
    void _sleepUntilNextStep();

    FloatSeconds _getTickDeltaTime() const;
    FloatSeconds _getFrameDeltaTime() const;

    hg::math::Vector2d _getViewRelativeMousePos(const hg::uwga::View* aView) const;
    hg::math::Vector2f _getWindowRelativeMousePos() const;
};

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_MANAGERS_WINDOW_MANAGER_DEFAULT_HPP

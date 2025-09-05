// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_MANAGERS_WINDOW_MANAGER_INTERFACE_HPP
#define SPEMPE_MANAGERS_WINDOW_MANAGER_INTERFACE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/RmlUi.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Render_texture.hpp>
#include <Hobgoblin/UWGA/Render_window.hpp>
#include <Hobgoblin/UWGA/System.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <SPeMPE/GameContext/Context_components.hpp>
#include <SPeMPE/Utility/Timing.hpp>
#include <SPeMPE/Utility/Window_frame_input_view.hpp>
#include <SPeMPE/Utility/Window_input_tracker.hpp>

#include <optional>

namespace jbatnozic {
namespace spempe {

constexpr bool PREVENT_BUSY_WAIT_ON  = true;
constexpr bool PREVENT_BUSY_WAIT_OFF = false;

constexpr bool VSYNC_ON  = true;
constexpr bool VSYNC_OFF = false;

class WindowManagerInterface : public ContextComponent {
public:
    virtual ~WindowManagerInterface() = default;

    ///////////////////////////////////////////////////////////////////////////
    // CONFIGURATION                                                         //
    ///////////////////////////////////////////////////////////////////////////

    enum class Mode {
        //! In this mode no window is actually opened.
        //! (This mode exists to make writing headless servers and
        //!  their clients more uniform.)
        HEADLESS,

        //! The 'usual' mode in which a single window is opened and
        //! operated by the WindowManager to display the game.
        NORMAL
    };

    struct WindowConfig {
        hg::math::Vector2pz   size = {640, 480};
        hg::UnicodeString     title;
        hg::uwga::WindowStyle style;
    };

    struct MainRenderTextureConfig {
        hg::math::Vector2pz size   = {640, 480};
        bool                smooth = true;
    };

    struct TimingConfig {
        //! Constructs the configuration with the 'usual' framerate limiter.
        //!
        //! \param aFramerateLimit The maximum number of times per second that the window can
        //!                        be displayed (refreshed by the game). This is decoupled from the
        //!                        tick rate of the game and won't affect simulation speed.
        //! \param aBusyWaitPreventionEnabled When set to `true` (PREVENT_BUSY_WAIT_ON), the
        //!                                   WindowManager will sleep between GameContext/QAO
        //!                                   iterations to preserve system resources.
        //! \param aVerticalSyncEnabled When set to `true` (VSYNC_ON), the window will use
        //!                             vertical synchronization to prevent screen tearing. It is
        //!                             recommended to only turn this on in fullscreen mode (real
        //!                             or windowed fullscreen).
        //!
        //! \note This is the recommended constructor to use when running on Windows.
        TimingConfig(FrameRate aFramerateLimit,
                     bool      aBusyWaitPreventionEnabled = PREVENT_BUSY_WAIT_ON,
                     bool      aVerticalSyncEnabled       = VSYNC_OFF);

        //! Constructs the configuration with the low level framerate limiter, which will block
        //! the application if the window is being displayed too quickly.
        //!
        //! \param aLowLevelFramerateLimiter The maximum number of times per second that the window can
        //!                                  be displayed (refreshed by the game). If this is set lower
        //!                                  than the tick rate of the game, it could slow down the
        //!                                  simulation. A value of 0 will leave the framerate
        //!                                  unlimited (which is not recommended).
        //! \param aBusyWaitPreventionEnabled When set to `true` (PREVENT_BUSY_WAIT_ON), the
        //!                                   WindowManager will sleep between GameContext/QAO
        //!                                   iterations to preserve system resources.
        //! \param aVerticalSyncEnabled When set to `true` (VSYNC_ON), the window will use
        //!                             vertical synchronization to prevent screen tearing. It is
        //!                             recommended to only turn this on in fullscreen mode (real
        //!                             or windowed fullscreen).
        //!
        //! \note When using this blocking low-level framerate limiter, it's recommended to leave
        //!       both busy wait prevention and vsync off, as they can interfere with each other.
        //!
        //! \note This is the recommended constructor to use when running on Mac.
        TimingConfig(hg::PZInteger aLowLevelFramerateLimiter  = 60,
                     bool          aBusyWaitPreventionEnabled = PREVENT_BUSY_WAIT_OFF,
                     bool          aVerticalSyncEnabled       = VSYNC_OFF);

        std::optional<FrameRate> framerateLimit;
        hg::PZInteger            lowLevelFramerateLimiter;
        bool                     busyWaitPreventionEnabled;
        bool                     verticalSyncEnabled;
    };

    virtual void setToHeadlessMode(const TimingConfig& aTimingConfig) = 0;

    virtual void setToNormalMode(hg::AvoidNull<std::shared_ptr<hg::uwga::System>> aGraphicsSystem,
                                 const WindowConfig&                              aWindowConfig,
                                 const MainRenderTextureConfig& aMainRenderTextureConfig,
                                 const TimingConfig&            aTimingConfig) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // WINDOW MANAGEMENT                                                     //
    ///////////////////////////////////////////////////////////////////////////

    //_window.getSettings();
    //_window.getPosition();
    virtual hg::math::Vector2pz getWindowSize() const = 0;
    //_window.hasFocus();
    //_window.mapCoordsToPixel();
    //_window.mapPixelToCoords();
    //_window.setFramerateLimit();
    //_window.setIcon();
    //_window.setPosition();
    //_window.setVerticalSyncEnabled();
    //_window.setTitle();
    //_window.setSize();
    //_window.setMouseCursor();
    //_window.setMouseCursorGrabbed();
    //_window.setMouseCursorVisible();
    // framerate limiter

    virtual void setStopIfCloseClicked(bool aStop) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // GRAPHICS & DRAWING                                                    //
    ///////////////////////////////////////////////////////////////////////////

    virtual hg::uwga::System& getGraphicsSystem() const = 0;

    virtual const hg::uwga::RenderWindow& getWindow() const = 0;

    virtual const hg::uwga::RenderTexture& getMainRenderTexture() const = 0;

    //! Return a reference to the currently active canvas object:
    //! - during the `DRAW_GUI` QAO event, this is the window itself (as returned by `getWindow()`).
    //! - during other events, this is the main render texture (as returned by `getMainRenderTexture()`).
    //!
    //! \throws if the manager is in Headless mode.
    virtual hg::uwga::Canvas& getActiveCanvas() = 0;

    enum class DrawPosition {
        NONE,     //!< Not drawn at all.
        FILL,     //!< Scaled (keeping aspect ratio) and centred so that the whole window is filled.
        FIT,      //!< Scaled (keeping aspect ratio) and centred so that the whole texture is visible.
        CENTER,   //!< Kept at original size and just centred in the window.
        STRETCH,  //!< Stretched (or compressed) to the window's exact size.
        TOP_LEFT, //!< Kept at original size and top left corner aligned with window's top left corner.
    };

    //! Default draw position is FIT.
    virtual void setMainRenderTextureDrawPosition(DrawPosition aDrawPosition) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // VIEWS                                                                 //
    ///////////////////////////////////////////////////////////////////////////

    //! TODO
    virtual hg::math::Vector2d mapPixelToCoords(const hg::math::Vector2f& aPoint,
                                                const hg::uwga::View&     aView) const = 0;

    //! TODO
    virtual hg::math::Vector2d mapPixelToCoords(const hg::math::Vector2f& aPoint) const = 0;

    //! TODO
    virtual hg::math::Vector2f mapCoordsToPixel(const hg::math::Vector2d& aPoint,
                                                const hg::uwga::View&     aView) const = 0;

    //! TODO
    virtual hg::math::Vector2f mapCoordsToPixel(const hg::math::Vector2d& aPoint) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // GUI                                                                   //
    ///////////////////////////////////////////////////////////////////////////

    virtual Rml::Context& getGUIContext() = 0;

    ///////////////////////////////////////////////////////////////////////////
    // KEYBOARD & MOUSE INPUT                                                //
    ///////////////////////////////////////////////////////////////////////////

    virtual WindowFrameInputView getInput() const = 0;

private:
    SPEMPE_CTXCOMP_TAG("jbatnozic::spempe::WindowManagerInterface");
};

inline WindowManagerInterface::TimingConfig::TimingConfig(FrameRate aFrameRateLimit,
                                                          bool      aBusyWaitPreventionEnabled,
                                                          bool      aVerticalSyncEnabled)
    : framerateLimit{aFrameRateLimit}
    , lowLevelFramerateLimiter{0}
    , busyWaitPreventionEnabled{aBusyWaitPreventionEnabled}
    , verticalSyncEnabled{aVerticalSyncEnabled} {}

inline WindowManagerInterface::TimingConfig::TimingConfig(hg::PZInteger aLowLevelFramerateLimiter,
                                                          bool          aBusyWaitPreventionEnabled,
                                                          bool          aVerticalSyncEnabled)
    : framerateLimit{std::nullopt}
    , lowLevelFramerateLimiter{aLowLevelFramerateLimiter}
    , busyWaitPreventionEnabled{aBusyWaitPreventionEnabled}
    , verticalSyncEnabled{aVerticalSyncEnabled} {}

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_MANAGERS_WINDOW_MANAGER_INTERFACE_HPP

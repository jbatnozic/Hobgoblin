// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/Managers/Window_manager_default.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Vector.hpp>

namespace jbatnozic {
namespace spempe {

DefaultWindowManager::DefaultWindowManager(hobgoblin::QAO_InstGuard aInstGuard, int aExecutionPriority)
    : NonstateObject{aInstGuard,
                     SPEMPE_TYPEID_SELF,
                     aExecutionPriority,
                     "::jbatnozic::spempe::DefaultWindowManager"}
    , _rmlUiContextDriver{}
    , _inputTracker{[this](const hg::uwga::View* aView) -> hg::math::Vector2d {
                        return _getViewRelativeMousePos(aView);
                    },
                    [this]() -> hg::math::Vector2f {
                        return _getWindowRelativeMousePos();
                    }} {}

///////////////////////////////////////////////////////////////////////////
// CONFIGURATION                                                         //
///////////////////////////////////////////////////////////////////////////

void DefaultWindowManager::setToHeadlessMode(const TimingConfig& aTimingConfig) {
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), headless == true);
    HG_HARD_ASSERT(_mode == Mode::UNINITIALIZED);

    _mode         = Mode::HEADLESS;
    _timingConfig = aTimingConfig;

    _mainRenderTexture.reset();
    _window.reset();
}

void DefaultWindowManager::setToNormalMode(
    hg::AvoidNull<std::shared_ptr<hg::uwga::System>> aGraphicsSystem,
    const WindowConfig&                              aWindowConfig,
    const MainRenderTextureConfig&                   aMainRenderTextureConfig,
    const TimingConfig&                              aTimingConfig) //
{
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), headless == false);
    HG_HARD_ASSERT(_mode == Mode::UNINITIALIZED);

    _graphicsSystem = std::move(aGraphicsSystem);

    _mode         = Mode::NORMAL;
    _timingConfig = aTimingConfig;

    // Create window:
    _window = _graphicsSystem->createRenderWindow(aWindowConfig.batchingConfig,
                                                  aWindowConfig.size.x,
                                                  aWindowConfig.size.y,
                                                  aWindowConfig.style,
                                                  aWindowConfig.title);

    _window->setFramerateLimit(_timingConfig.lowLevelFramerateLimiter);
    _window->setVerticalSyncEnabled(_timingConfig.verticalSyncEnabled);

    _windowClearingColor = aWindowConfig.clearingColor;

    // Create main render texture:
    _mainRenderTexture = _graphicsSystem->createRenderTexture(aMainRenderTextureConfig.batchingConfig,
                                                              aMainRenderTextureConfig.size);
    _mainRenderTexture->setSmooth(aMainRenderTextureConfig.smooth);

    _mrtSprite = std::make_unique<hg::uwga::Sprite>(*_graphicsSystem);
    _mrtSprite->addSubsprite(hg::uwga::TextureRect{});

    _mrtClearingColor = aMainRenderTextureConfig.clearingColor;

    // Create GUI:
    _rmlUiBackendLifecycleGuard = hg::rml::HobgoblinBackend::initialize(_graphicsSystem);
    _rmlUiContextDriver.emplace("DefaultWindowManager::RmlContext", *_window);
}

DefaultWindowManager::Mode DefaultWindowManager::getMode() const {
    return _mode;
}

///////////////////////////////////////////////////////////////////////////
// WINDOW MANAGEMENT                                                     //
///////////////////////////////////////////////////////////////////////////

hobgoblin::math::Vector2pz DefaultWindowManager::getWindowSize() const {
    return GetSize(*_window);
}

void DefaultWindowManager::setStopIfCloseClicked(bool aStop) {
    _stopIfCloseClicked = aStop;
}

///////////////////////////////////////////////////////////////////////////
// GRAPHICS & DRAWING                                                    //
///////////////////////////////////////////////////////////////////////////

hg::uwga::System& DefaultWindowManager::getGraphicsSystem() const {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");
    return *_graphicsSystem;
}

const hg::uwga::RenderWindow& DefaultWindowManager::getWindow() const {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");
    return *_window;
}

const hg::uwga::RenderTexture& DefaultWindowManager::getMainRenderTexture() const {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");
    return *_mainRenderTexture;
}

hg::uwga::Canvas& DefaultWindowManager::getActiveCanvas() {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");
    if (getRuntime()->getCurrentEvent() == hg::QAO_Event::DRAW_GUI) {
        return *_window;
    } else {
        return *_mainRenderTexture;
    }
}

void DefaultWindowManager::setMainRenderTextureDrawPosition(DrawPosition aDrawPosition) {
    _mainRenderTextureDrawPos = aDrawPosition;
}

///////////////////////////////////////////////////////////////////////////
// VIEWS                                                                 //
///////////////////////////////////////////////////////////////////////////

hg::math::Vector2d DefaultWindowManager::mapPixelToCoords(const hg::math::Vector2f& aPixel,
                                                          const hg::uwga::View&     aView) const {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");

    const auto mrtPositioning = _getMainRenderTexturePositioningData();

    auto windowPos = _window->mapPixelToCoords(aPixel, _window->getView());
    windowPos.x =
        (windowPos.x - mrtPositioning.position.x) / mrtPositioning.scale.x + mrtPositioning.origin.x;
    windowPos.y =
        (windowPos.y - mrtPositioning.position.y) / mrtPositioning.scale.y + mrtPositioning.origin.y;

    return _mainRenderTexture->mapPixelToCoords(windowPos.cast<float>(), aView);
}

hg::math::Vector2d DefaultWindowManager::mapPixelToCoords(const hg::math::Vector2f& aPixel) const {
    return mapPixelToCoords(aPixel, _mainRenderTexture->getView());
}

hg::math::Vector2f DefaultWindowManager::mapCoordsToPixel(const hg::math::Vector2d& aCoords,
                                                          const hg::uwga::View&     aView) const {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");

    const auto mrtPositioning = _getMainRenderTexturePositioningData();

    auto       mrtPos = _mainRenderTexture->mapCoordsToPixel(aCoords, aView);
    const auto xx =
        mrtPositioning.position.x + (mrtPos.x - mrtPositioning.origin.x) * mrtPositioning.scale.x;
    const auto yy =
        mrtPositioning.position.y + (mrtPos.y - mrtPositioning.origin.y) * mrtPositioning.scale.y;

    return _window->mapCoordsToPixel({xx, yy}, _window->getView());
}

hg::math::Vector2f DefaultWindowManager::mapCoordsToPixel(const hg::math::Vector2d& aCoords) const {
    return mapCoordsToPixel(aCoords, _mainRenderTexture->getView());
}

///////////////////////////////////////////////////////////////////////////
// GUI                                                                   //
///////////////////////////////////////////////////////////////////////////

Rml::Context& DefaultWindowManager::getGUIContext() {
    HG_HARD_ASSERT(_mode == Mode::NORMAL && "Method only available in NORMAL mode.");
    return *(*_rmlUiContextDriver);
}

///////////////////////////////////////////////////////////////////////////
// KEYBOARD & MOUSE INPUT                                                //
///////////////////////////////////////////////////////////////////////////

WindowFrameInputView DefaultWindowManager::getInput() const {
    return _inputTracker.getInputView();
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS                                                       //
///////////////////////////////////////////////////////////////////////////

void DefaultWindowManager::_eventPreUpdate() {
    _inputTracker.prepForEvents();

    for (const auto& ev : _events) {
        const auto rmlUiDidConsumeEvent = _rmlUiContextDriver->processEvent(ev);
        if (rmlUiDidConsumeEvent) {
            continue;
        }

        _inputTracker.eventOccurred(ev);

        ev.visit(
            [this](const hg::uwga::WindowEvent::Closed& aEventData) {
                if (_stopIfCloseClicked) {
                    ctx().stop();
                }
            },
            [this](const hg::uwga::WindowEvent::Resized& aEventData) {
                // clang-format off
                // _window->setView(hg::uwga::View{{ // TODO
                //     0.f,
                //     0.f,
                //     static_cast<float>(aEventData.width),
                //     static_cast<float>(aEventData.height)
                // }});
                // clang-format on
            });
    }
    _events.clear();
}

void DefaultWindowManager::_eventPreDraw() {
    if (_mode == Mode::NORMAL) {
        if (_mrtClearingColor.has_value()) {
            _mainRenderTexture->clear(*_mrtClearingColor);
        }
    }
}

void DefaultWindowManager::_eventDraw2() {
    if (_mode == Mode::NORMAL) {
        if (_windowClearingColor.has_value()) {
            _window->clear(*_windowClearingColor);
        }
        _drawMainRenderTexture();
    }
}

void DefaultWindowManager::_eventDrawGUI() {
    if (_mode == Mode::NORMAL) {
        _window->flush();
        _rmlUiContextDriver->update();
        _rmlUiContextDriver->render();
        _frameReady = true;
    }
}

void DefaultWindowManager::_eventDisplay() {
    if (_mode == Mode::NORMAL) {
        _displayWindowAndPollEvents();
    }
    if (_timingConfig.busyWaitPreventionEnabled) {
        _sleepUntilNextStep();
    }
}

DefaultWindowManager::MainRenderTexturePositioningData DefaultWindowManager::
    _getMainRenderTexturePositioningData() const //
{
    MainRenderTexturePositioningData result;

    const auto mrtSize = GetSize(*_mainRenderTexture);
    const auto winSize = _window->getView().getSize();

    switch (_mainRenderTextureDrawPos) {
    case DrawPosition::NONE:
        result.position = {0.f, 0.f};
        result.origin   = {0.f, 0.f};
        result.scale    = {0.f, 0.f};
        break;

    case DrawPosition::FILL:
    case DrawPosition::FIT:
        {
            float scale;
            if (_mainRenderTextureDrawPos == DrawPosition::FILL) {
                scale = std::max(static_cast<float>(winSize.x) / mrtSize.x,
                                 static_cast<float>(winSize.y) / mrtSize.y);
            } else {
                scale = std::min(static_cast<float>(winSize.x) / mrtSize.x,
                                 static_cast<float>(winSize.y) / mrtSize.y);
            }
            result.scale = {scale, scale};
        }
        // FALLTHROUGH

    case DrawPosition::CENTER:
        if (_mainRenderTextureDrawPos == DrawPosition::CENTER) {
            result.scale = {1.f, 1.f};
        }

        result.position = {static_cast<float>(winSize.x) * 0.5f, static_cast<float>(winSize.y) * 0.5f};
        result.origin   = {static_cast<float>(mrtSize.x) * 0.5f, static_cast<float>(mrtSize.y) * 0.5f};
        break;

    case DrawPosition::STRETCH:
        result.position = {0.f, 0.f};
        result.origin   = {0.f, 0.f};
        result.scale    = {static_cast<float>(winSize.x) / mrtSize.x,
                           static_cast<float>(winSize.y) / mrtSize.y};
        break;

    case DrawPosition::TOP_LEFT:
        result.position = {0.f, 0.f};
        result.origin   = {0.f, 0.f};
        result.scale    = {1.f, 1.f};
        break;

    default:
        HG_UNREACHABLE();
    }

    return result;
}

void DefaultWindowManager::_drawMainRenderTexture() {
    _mainRenderTexture->flush();
    _mainRenderTexture->display();

    _mrtSprite->setTexture(_mainRenderTexture.get());
    _mrtSprite->subsprites[0] = hg::uwga::Sprite::Subsprite{*_mainRenderTexture};

    const auto mrtPositioning = _getMainRenderTexturePositioningData();

    _mrtSprite->setPosition(mrtPositioning.position);
    _mrtSprite->setOrigin(mrtPositioning.origin);
    _mrtSprite->setScale(mrtPositioning.scale);

    _window->draw(*_mrtSprite);
}

void DefaultWindowManager::_displayWindowAndPollEvents() {
    if (_timingConfig.framerateLimit.has_value() &&
        _timeSinceLastDisplay.getElapsedTime() < _getFrameDeltaTime()) {
        goto EVENTS;
    }

    _timeSinceLastDisplay.restart();
    if (_frameReady) {
        _window->flush();
        _window->display();
    }
    _frameReady = false;

EVENTS:
    hg::uwga::WindowEvent ev;
    while (_window->pollEvent(ev)) {
        _events.push_back(std::move(ev));
    }
}

void DefaultWindowManager::_sleepUntilNextStep() {
    using std::chrono::duration_cast;
    using Duration = std::chrono::microseconds;
    const auto now = std::chrono::steady_clock::now();

    const auto updateDeltaTime = duration_cast<Duration>(_getTickDeltaTime());
    auto       timeUntilUpdate =
        updateDeltaTime - duration_cast<Duration>(now - ctx().getPerformanceInfo().updateStart);

    if (_mode == Mode::NORMAL) {
        if (_timingConfig.framerateLimit.has_value()) {
            const auto displayDeltaTime = duration_cast<Duration>(_getFrameDeltaTime());
            auto       timeUntilDisplay =
                displayDeltaTime -
                duration_cast<Duration>(now - ctx().getPerformanceInfo().displayStart);

            const auto min_ = std::min(timeUntilUpdate, timeUntilDisplay);
            hg::util::SuperPreciseSleep(min_);
        }
    } else {
        hg::util::SuperPreciseSleep(timeUntilUpdate);
    }
}

FloatSeconds DefaultWindowManager::_getTickDeltaTime() const {
    return ctx().getRuntimeConfig().tickRate.getDeltaTime();
}

FloatSeconds DefaultWindowManager::_getFrameDeltaTime() const {
    return _timingConfig.framerateLimit.value().getDeltaTime();
}

hg::math::Vector2d DefaultWindowManager::_getViewRelativeMousePos(const hg::uwga::View* aView) const {
    if (_mode != Mode::NORMAL) {
        return {0.f, 0.f};
    }

    const auto mrtPositioning = _getMainRenderTexturePositioningData();
    const auto pixelPos       = _window->getRelativeCursorPosition();

    auto windowPos =
        _window->mapPixelToCoords(pixelPos.cast<float>(), aView ? *aView : _window->getView());
    windowPos.x =
        (windowPos.x - mrtPositioning.position.x) / mrtPositioning.scale.x + mrtPositioning.origin.x;
    windowPos.y =
        (windowPos.y - mrtPositioning.position.y) / mrtPositioning.scale.y + mrtPositioning.origin.y;

    return _mainRenderTexture->mapPixelToCoords(windowPos.cast<float>());
}

hg::math::Vector2f DefaultWindowManager::_getWindowRelativeMousePos() const {
    if (_mode != Mode::NORMAL) {
        return {0, 0};
    }

    return _window->getRelativeCursorPosition();
}

} // namespace spempe
} // namespace jbatnozic

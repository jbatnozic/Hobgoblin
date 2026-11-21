// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Interactivity_manager_default.hpp"

#include <CBR/QAOMessages/Handle_pncs_event.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <algorithm>

namespace cinnabar {

// MARK: HostData
    
class DefaultInteractivityManager::HostData {
public:
    HostData(hg::PZInteger aPlayerCount) {
        setPlayerCount(aPlayerCount);
    }

    void setPlayerCount(hg::PZInteger aPlayerCount) {
        const auto sz = hg::pztos(aPlayerCount);
        _playerControls.resize(sz);
        _clientControls.resize(sz);
    }
    
    void setAllControlsToDefault() {
        for (auto& pc : _playerControls) {
            pc = {};
        }
        for (auto& cc : _clientControls) {
            cc = {};
        }
    }

    void setControlsOfPlayer(hg::PZInteger aPlayerIndex, const PlayerControls& aControls) {
        const_cast<PlayerControls&>(getControlsOfPlayer(aPlayerIndex)) = aControls;
    }

    const PlayerControls& getControlsOfPlayer(hg::PZInteger aPlayerIndex) {
        const auto index = hg::pztos(aPlayerIndex);
        HG_ASSERT(index < _playerControls.size());
        return _playerControls[index];
    }

    void setControlsOfClient(int aClientIndex, const PlayerControls& aControls) {
        const_cast<PlayerControls&>(getControlsOfClient(aClientIndex)) = aControls;
    }

    const PlayerControls& getControlsOfClient(int aClientIndex) {
        const auto index = hg::pztos(aClientIndex + CLIENT_INDEX_OFFSET);
        HG_ASSERT(index < _clientControls.size());
        return _clientControls[index];
    }

private:
    static_assert(spe::CLIENT_INDEX_LOCAL == -1);
    constexpr static hg::PZInteger CLIENT_INDEX_OFFSET = 1;

    //! Entry [P] holds controls for player [P]
    std::vector<PlayerControls> _playerControls;

    //! Entry [C] holds controls of client [C-1] (offset of 1 because local player has index -1)
    std::vector<PlayerControls> _clientControls;
};

// MARK: DefaultInteractivityManager

#define MODE_EDGE spe::WindowFrameInputView::Mode::Edge

DefaultInteractivityManager::DefaultInteractivityManager(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard,
                          QAO_ExeCon::INTERACTIVITY,
                          PRIORITY_INTERACTIVITYMGR,
                          QAO_STATIC_NAME("cinnabar::DefaultInteractivityManager")} {}

DefaultInteractivityManager::~DefaultInteractivityManager() = default;

DefaultInteractivityManager::Mode DefaultInteractivityManager::getMode() const {
    return _mode;
}

void DefaultInteractivityManager::setToClientMode() {
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(),
                                       privileged == false,
                                       headless == false,
                                       networking == true);
    _mode = Mode::CLIENT;
    _initSecondaryFocusRenderTexture();
}

void DefaultInteractivityManager::setToHostMode(hg::PZInteger aPlayerCount) {
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged == true, headless == true, networking == true);
    _mode = Mode::HOST;

    _hostData = std::make_unique<HostData>(aPlayerCount);
}

void DefaultInteractivityManager::setToDevMode() {
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged == true, headless == false);
    _mode = Mode::DEV;
    _initSecondaryFocusRenderTexture();
}

auto DefaultInteractivityManager::getCurrentPlayerViewFocus() const -> PlayerViewFocus {
    return _playerViewFocus;
}

void DefaultInteractivityManager::pushClickableObject(
    QAO_GenericId                           aClickableId,
    int                                     aFinegrainedPriority,
    std::intptr_t                           aUserData,
    std::function<bool(hg::math::Vector2d)> aQuickMouseOverCheck,
    std::function<bool(hg::math::Vector2d)> aFullMouseOverCheck) //
{
    HG_ASSERT(getRuntime()->getCurrentEvent() == QAO_Event::BEGIN_UPDATE);

    if (!aQuickMouseOverCheck(_getMouseWorldPosition())) {
        return; // The object doesn't intersect with the mouse cursor (but no guarantee that it does!)
    }

    _clickables.push_back({.id                  = aClickableId,
                           .finegrainedPriority = aFinegrainedPriority,
                           .userData            = aUserData,
                           .fullMouseOverCheck  = std::move(aFullMouseOverCheck)});
}

const PlayerControls& DefaultInteractivityManager::getControlsOfPlayer(
    hg::PZInteger aPlayerIndex) const //
{
    return _hostData->getControlsOfPlayer(aPlayerIndex);
}

const PlayerControls& DefaultInteractivityManager::getControlsOfClient(int aClientIndex) const {
    return _hostData->getControlsOfClient(aClientIndex);
}

// MARK: Private

void DefaultInteractivityManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);
    _winMgr          = &ccomp<MWindow>();
    _lobbyBackendMgr = &ccomp<MLobbyBackend>();
}

void DefaultInteractivityManager::_eventPreUpdate() {
    _clickables.clear();
    _mouseWorldPos.reset();
}

void DefaultInteractivityManager::_eventBeginUpdate() {
    switch (_mode) {
    case Mode::INITIAL:
        // Do nothing
        break;

    case Mode::HOST:
        _gatherRemotePlayerControls();
        break;

    case Mode::CLIENT:
        _sendLocalPlayerControls();
        FALLTHROUGH;

    case Mode::DEV:
        _dispatchPNCSEvents();
        _switchPlayerViewFocusIfNeeded();
        break;

    default:
        HG_UNREACHABLE("Illegal value for enum InteractivityManager::Mode ({})!", (int)_mode);
    }
}

void DefaultInteractivityManager::_eventPreDraw() {
    // Here all the clickables are already sorted

    while (!_clickables.empty()) {
        auto& info = _clickables.back();

        auto instance = getRuntime()->find(info.id);
        if (instance.isNull()) {
            _clickables.pop_back();
            continue;
        }

        if (!info.fullMouseOverCheck(_getMouseWorldPosition())) {
            _clickables.pop_back(); // The object doesn't intersect with the mouse cursor
            continue;
        }

        PNCSEventData evData = {.mbLeftDown       = false,
                                .mbRightDown      = false,
                                .mbLeftUp         = false,
                                .mbRightUp        = false,
                                .localPlayerIndex = _lobbyBackendMgr->getLocalPlayerIndex(),
                                .userData         = info.userData};

        (void)QAO_SendMessage<HandlePNCSEvent>(*instance, &evData);

        break; // End the loop after the "foreground-most" object receives its message
    }

    if (_secondaryFocusRenderTexture != nullptr) {
        _secondaryFocusRenderTexture->clear(hg::uwga::COLOR_BLACK);
    }
}

void DefaultInteractivityManager::_initSecondaryFocusRenderTexture() {
    HG_ASSERT(getRuntime() != nullptr);

    if (_secondaryFocusRenderTexture == nullptr) {
        using hg::uwga::BatchingConfig;
        const BatchingConfig      batchingConfig{.strategy = BatchingConfig::Strategy::FAST_N_LOOSE};
        const hg::math::Vector2pz size{1920, 1080};

        HG_ASSERT(_winMgr != nullptr);

        _secondaryFocusRenderTexture =
            _winMgr->getGraphicsSystem().createRenderTexture(batchingConfig, size);
    }
}

hg::math::Vector2d DefaultInteractivityManager::_getMouseWorldPosition() {
    if (!_mouseWorldPos.has_value()) {
        _mouseWorldPos = _winMgr->getInput().getViewRelativeMousePos();
    }

    return *_mouseWorldPos;
}

void DefaultInteractivityManager::_gatherRemotePlayerControls() {
    HG_ASSERT(_mode == Mode::HOST);

    auto& lobbyBackendMgr = ccomp<MLobbyBackend>();
    auto& inputSyncMgr    = ccomp<MInput>();

    const auto playerCount = lobbyBackendMgr.getSize();

    _hostData->setPlayerCount(playerCount); // no-op if player count is already correct
    _hostData->setAllControlsToDefault();

    for (hg::PZInteger clientIndex = 0; clientIndex < playerCount - 1; ++clientIndex) {
        const int playerIndex = lobbyBackendMgr.clientIdxToPlayerIdx(clientIndex);

        const auto controls = PlayerControls::fromInputSyncManager(inputSyncMgr, clientIndex);

        _hostData->setControlsOfClient(clientIndex, controls);

        if (playerIndex != spe::PLAYER_INDEX_UNKNOWN) {
            _hostData->setControlsOfPlayer(clientIndex + 1, controls);
        }
    }
}

void DefaultInteractivityManager::_sendLocalPlayerControls() {
    HG_ASSERT(_mode == Mode::CLIENT);

    const auto controls = PlayerControls::fromLocalInput(_winMgr->getInput());
    controls.toInputSyncManager(ccomp<MInput>());
}

void DefaultInteractivityManager::_dispatchPNCSEvents() {
    std::stable_sort(_clickables.begin(), _clickables.end());

    const auto mouseWorldPos = _getMouseWorldPosition();
    while (!_clickables.empty()) {
        auto& info = _clickables.back();

        auto instance = getRuntime()->find(info.id);
        if (instance.isNull()) {
            _clickables.pop_back();
            continue;
        }

        if (!info.fullMouseOverCheck(mouseWorldPos)) {
            _clickables.pop_back(); // The object doesn't intersect with the mouse cursor
            continue;
        }

        const auto input = _winMgr->getInput();

        PNCSEventData evData = {.mbLeftDown       = input.checkPressed(hg::in::MB_LEFT, MODE_EDGE),
                                .mbRightDown      = input.checkPressed(hg::in::MB_RIGHT, MODE_EDGE),
                                .mbLeftUp         = input.checkReleased(hg::in::MB_LEFT, MODE_EDGE),
                                .mbRightUp        = input.checkReleased(hg::in::MB_RIGHT, MODE_EDGE),
                                .localPlayerIndex = _lobbyBackendMgr->getLocalPlayerIndex(),
                                .userData         = info.userData};

        (void)QAO_SendMessage<HandlePNCSEvent>(*instance, &evData);

        break; // End the loop after the "foreground-most" object receives its message
    }
}

void DefaultInteractivityManager::_switchPlayerViewFocusIfNeeded() {
    if (_winMgr->getInput().checkPressed(hg::in::PK_TAB, spe::WindowFrameInputView::Mode::Edge)) {
        switch (_playerViewFocus) {
        case PlayerViewFocus::SHIP_INTERIOR:
            _playerViewFocus = PlayerViewFocus::OVERWORLD;
            break;

        case PlayerViewFocus::OVERWORLD:
            _playerViewFocus = PlayerViewFocus::SHIP_INTERIOR;
            break;

        default:
            HG_UNREACHABLE("Illegal value for PlayerViewFocus ({})!", (int)_playerViewFocus);
        }
    }
}

hg::uwga::Canvas& DefaultInteractivityManager::_getActiveCanvasForShipInterior() {
    [[maybe_unused]] QAO_Event::Enum currentEvent;
    HG_ASSERT((currentEvent = getRuntime()->getCurrentEvent(),
               (currentEvent == QAO_Event::DRAW_1 || currentEvent == QAO_Event::DRAW_2)));

    switch (_playerViewFocus) {
        case PlayerViewFocus::SHIP_INTERIOR:
            return _winMgr->getActiveCanvas();
            break;

        case PlayerViewFocus::OVERWORLD:
            return *_secondaryFocusRenderTexture;
            break;

        default:
            HG_UNREACHABLE("Illegal value for PlayerViewFocus ({})!", (int)_playerViewFocus);
    }
}

hg::uwga::Canvas& DefaultInteractivityManager::_getActiveCanvasForOverworld() {
    [[maybe_unused]] QAO_Event::Enum currentEvent;
    HG_ASSERT((currentEvent = getRuntime()->getCurrentEvent(),
               (currentEvent == QAO_Event::DRAW_1 || currentEvent == QAO_Event::DRAW_2)));

   switch (_playerViewFocus) {
        case PlayerViewFocus::SHIP_INTERIOR:
            return *_secondaryFocusRenderTexture;
            break;
            
        case PlayerViewFocus::OVERWORLD:
            return _winMgr->getActiveCanvas();
            break;

        default:
            HG_UNREACHABLE("Illegal value for PlayerViewFocus ({})!", (int)_playerViewFocus);
    }
}

} // namespace cinnabar

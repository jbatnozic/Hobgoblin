// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Lobby_event_manager_default.hpp"

#include <algorithm>

namespace cinnabar {

DefaultLobbyEventManager::DefaultLobbyEventManager(QAO_InstGuard aInstGuard, int aExecutionPriority)
    : NonstateObject{aInstGuard,
                     QAO_ExeCon::SYNCHRONIZATION,
                     aExecutionPriority,
                     QAO_STATIC_NAME("cinnabar::DefaultLobbyEventManager")} {}

void DefaultLobbyEventManager::addEventListener(hg::NeverNull<LobbyEventListener*> aListener) {
    if (std::find_if(_lobbyEventListeners.begin(),
                     _lobbyEventListeners.end(),
                     [aListener](const auto aItem) {
                         return aItem == aListener;
                     }) != _lobbyEventListeners.end()) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Listener at address {} is already registered.");
    }
    _lobbyEventListeners.push_back(aListener);
}

void DefaultLobbyEventManager::removeEventListener(hg::NeverNull<LobbyEventListener*> aListener) {
    std::erase_if(_lobbyEventListeners, [aListener](const auto aItem) {
        return aItem == aListener;
    });
}

void DefaultLobbyEventManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);

    _networkingManager   = &ccomp<MNetworking>();
    _lobbyBackendManager = &ccomp<MLobbyBackend>();
    
    _networkingManager->addEventListener(this);
}

void DefaultLobbyEventManager::_willDetach(QAO_Runtime& aRuntime) {
    _networkingManager->removeEventListener(this);

    _lobbyBackendManager = nullptr;
    _networkingManager   = nullptr;

    spe::NonstateObject::_willDetach(aRuntime);
}

void DefaultLobbyEventManager::_eventBeginUpdate() {
    spe::LobbyBackendEvent lbEvent;
    while (_lobbyBackendManager->pollEvent(lbEvent)) {
        for (auto* listener : _lobbyEventListeners) {
            listener->onLobbyEvent(lbEvent);
        }
    }
}

void DefaultLobbyEventManager::onNetworkingEvent(const RN_Event& aEvent) {
    if (_networkingManager->isClient()) {
        aEvent.visit([this](const RN_Event::Connected&) {
            HG_LOG_INFO(LOG_ID, "Client lobby uploading local info to server.");
            _lobbyBackendManager->uploadLocalInfo();
        });
    }
}

} // namespace cinnabar

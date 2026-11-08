// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/Managers/Lobby/Lobby_event_manager.hpp>
#include <Engine.hpp>

#include <vector>

namespace cinnabar {

/*!
 * \brief Default implementation of `LobbyEventManager`.
 *
 * Dependencies:
 * - `LobbyBackendManager`
 * - `NetworkingManager`
 *
 * Event listing:
 * - _eventBeginUpdate() =>
 *
 * Execution priority hint:
 * - Right after `LobbyBackendManager`.
 */
class DefaultLobbyEventManager
    : public LobbyEventManager
    , public spe::NonstateObject
    , private spe::NetworkingEventListener {
public:
    DefaultLobbyEventManager(QAO_InstGuard aInstGuard, int aExecutionPriority);

    void addEventListener(hg::NeverNull<LobbyEventListener*> aListener) override;

    void removeEventListener(hg::NeverNull<LobbyEventListener*> aListener) override;

private:
    MNetworking*   _networkingManager   = nullptr;
    MLobbyBackend* _lobbyBackendManager = nullptr;

    std::vector<LobbyEventListener*> _lobbyEventListeners;

    void _didAttach(QAO_Runtime& aRuntime) override;
    void _willDetach(QAO_Runtime& aRuntime) override;

    void _eventBeginUpdate() override;

    void onNetworkingEvent(const RN_Event& aEvent) override;
};

QAO_REGISTER_CLASS(DefaultLobbyEventManager, cinnabar_DefaultLobbyEventManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar

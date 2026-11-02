// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/Managers/UserInterface/Lobby_frontend_manager.hpp>
#include <Engine.hpp>

#include <memory>
#include <string>

namespace cinnabar {

/*!
 * \brief Default implementation of `LobbyFrontendManager`.
 *
 * Dependencies:
 * - `AuthorizationManager`
 * - `LobbyBackendManager`
 * - `WindowManager`
 *
 * Event listing:
 * - NOTE: this manager does not implement _eventPreUpdate(), but it receives input events at the end of
 *         this event while `WindowManager` is processing said events.
 * - _eventBeginUpdate() => Poll events from `LobbyBackendManager` and adjust the frontend accordingly.
 * - _eventUpdate1()     => (Client only) Show/Hide the GUI.
 * - _eventDrawGUI()     => (Client only) Draw the GUI.
 *
 * Execution priority hint:
 * - n/a
 */
class DefaultLobbyFrontendManager
    : public LobbyFrontendManager
    , public spe::NonstateObject {
public:
    DefaultLobbyFrontendManager(QAO_InstGuard aInstGuard, int aExecutionPriority);
    ~DefaultLobbyFrontendManager() override;

    void setToHeadlessHostMode() override;
    void setToClientMode(const std::string& aName, const std::string& aUniqueId) override;

    Mode getMode() const override;

private:
    class Impl;
    friend Impl;
    std::unique_ptr<Impl> _impl;

    void _willDetach(QAO_Runtime& aRuntime) override;

    void _eventBeginUpdate() override;
    void _eventUpdate1() override;
    void _eventDrawGUI() override;

    friend void ActivateCommand(DefaultLobbyFrontendManager& aMgr, int aCommand, void* aArgs);
};

QAO_REGISTER_CLASS(DefaultLobbyFrontendManager, cinnabar_LobbyFrontendManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include "Engine.h"
#include "Lobby_frontend_service.hpp"

#include <memory>

class DefaultLobbyFrontendService
    : public LobbyFrontendService
    , public spe::NonstateObject {
public:
    DefaultLobbyFrontendService(QAO_InstGuard aInstGuard, int aExecutionPriority);
    ~DefaultLobbyFrontendService() override;

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

    friend void ActivateCommand(DefaultLobbyFrontendService& aSvc, int aCommand, void* aArgs);
};

QAO_REGISTER_CLASS(DefaultLobbyFrontendService, Example_LobbyFrontendService) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setSuperclass<spe::NonstateObject>();
}

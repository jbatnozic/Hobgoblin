// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Main_game_flow_manager_interface.hpp>

#include <cstdint>

namespace cinnabar {

class MainGameFlowManager
    : public spe::NonstateObject
    , public MainGameFlowManagerInterface {
public:
    MainGameFlowManager(QAO_InstGuard aInstGuard);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventDisplay() override;

    std::uint32_t _iterOrdinal = 0;
};

QAO_REGISTER_CLASS(MainGameFlowManager, cinnabar_MainGameFlowManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar

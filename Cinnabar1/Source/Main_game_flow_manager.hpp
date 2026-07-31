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
    void _eventDisplay() override;

    std::uint32_t _iterOrdinal = 0;
};

} // namespace cinnabar

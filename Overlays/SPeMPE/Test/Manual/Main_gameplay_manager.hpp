// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Config.hpp"
#include "Engine.hpp"

#include "Main_gameplay_manager_interface.hpp"

class MainGameplayManagerBase
    : public spe::NonstateObject {
public:
    MainGameplayManagerBase(QAO_InstGuard aInstGuard);

protected:
    void _didAttach(QAO_Runtime&) override;

private:    
    void _eventPreUpdate() override;
};

namespace singleplayer {

class MainGameplayManager
    : public MainGameplayManagerInterface
    , public MainGameplayManagerBase {
public:
    MainGameplayManager(QAO_InstGuard aInstGuard);

protected:
    void _didAttach(QAO_Runtime&) override;
};

} // namespace singleplayer

namespace multiplayer {

class MainGameplayManager
    : public MainGameplayManagerInterface
    , public MainGameplayManagerBase {
public:
    MainGameplayManager(QAO_InstGuard aInstGuard);

protected:
    void _didAttach(QAO_Runtime&) override;
};

} // namespace multiplayer

// clang-format on

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
    MainGameplayManagerBase(QAO_RuntimeRef aRuntimeRef);

private:
    void _eventPreUpdate() override;
};

namespace singleplayer {

class MainGameplayManager
    : public MainGameplayManagerInterface
    , public MainGameplayManagerBase {
public:
    MainGameplayManager(QAO_RuntimeRef aRuntimeRef);
};

} // namespace singleplayer

namespace multiplayer {

class MainGameplayManager
    : public MainGameplayManagerInterface
    , public MainGameplayManagerBase {
public:
    MainGameplayManager(QAO_RuntimeRef aRuntimeRef);
};

} // namespace multiplayer

// clang-format on

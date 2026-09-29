// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Config.hpp"
#include "Engine.hpp"

#include "Main_gameplay_service.hpp"

class MainGameplayServiceBase
    : public spe::NonstateObject {
public:
    MainGameplayServiceBase(QAO_InstGuard aInstGuard);

protected:
    void _didAttach(QAO_Runtime&) override;

private:    
    void _eventPreUpdate() override;
};

namespace singleplayer {

class DefaultMainGameplayService
    : public MainGameplayService
    , public MainGameplayServiceBase {
public:
    DefaultMainGameplayService(QAO_InstGuard aInstGuard);

protected:
    void _didAttach(QAO_Runtime&) override;
};

} // namespace singleplayer

namespace multiplayer {

class DefaultMainGameplayService
    : public MainGameplayService
    , public MainGameplayServiceBase {
public:
    DefaultMainGameplayService(QAO_InstGuard aInstGuard);

protected:
    void _didAttach(QAO_Runtime&) override;
};

} // namespace multiplayer

// clang-format on

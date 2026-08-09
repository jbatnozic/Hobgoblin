// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Alvin/Main_collision_dispatcher.hpp>

#include <Overworld_manager_interface.hpp>

namespace cinnabar {

class OverworldManager
    : public spe::StateObject
    , public OverworldManagerInterface {
public:
    OverworldManager(QAO_InstGuard aInstGuard);

    hg::alvin::Space& getAlvinSpace() override;

    const hg::alvin::Space& getAlvinSpace() const override;

private:
    // void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1() override;

    hg::alvin::Space _space;
    hg::alvin::MainCollisionDispatcher _mcd;
};

QAO_REGISTER_CLASS(OverworldManager, cinnabar_OverworldManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
}

} // namespace cinnabar

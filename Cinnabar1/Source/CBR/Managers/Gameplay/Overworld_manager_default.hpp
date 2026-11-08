// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/Managers/Gameplay/Overworld_manager.hpp>
#include <Engine.hpp>

#include <Hobgoblin/Alvin/Main_collision_dispatcher.hpp>

namespace cinnabar {

/*!
 * \brief Default implementation of `AuxiliaryGameplayManager`.
 *
 * Dependencies:
 * - <none>
 *
 * Event listing:
 * - _eventUpdate1() => Simulate Chipmunk (Alvin) physics space.
 *
 * Execution priority hint:
 * - n/a
 */
class DefaultOverworldManager
    : public spe::StateObject
    , public OverworldManager {
public:
    DefaultOverworldManager(QAO_InstGuard aInstGuard);

    hg::alvin::Space& getAlvinSpace() override;

    const hg::alvin::Space& getAlvinSpace() const override;

private:
    // void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1() override;

    hg::alvin::Space _space;
    hg::alvin::MainCollisionDispatcher _mcd;
};

QAO_REGISTER_CLASS(DefaultOverworldManager, cinnabar_DefaultOverworldManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
}

} // namespace cinnabar

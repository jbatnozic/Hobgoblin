// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/Managers/Gameplay/Auxiliary_gameplay_manager.hpp>
#include <Engine.hpp>

namespace cinnabar {

/*!
 * \brief Default implementation of `AuxiliaryGameplayManager`.
 *
 * Dependencies:
 * - <none>
 *
 * Event listing:
 * - <none>
 *
 * Execution priority hint:
 * - n/a
 */
class DefaultAuxiliaryGameplayManager
    : public spe::NonstateObject
    , public AuxiliaryGameplayManager {
public:
    DefaultAuxiliaryGameplayManager(QAO_InstGuard aInstGuard);

    QAO_Handle<ShipController> getAssociatedShipControllerOfLocalPlayer() const override;

private:
    void _didAttach(QAO_Runtime& aRuntime) override;
    void _willDetach(QAO_Runtime& aRuntime) override;
};

QAO_REGISTER_CLASS(DefaultAuxiliaryGameplayManager, cinnabar_DefaultAuxiliaryGameplayManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar

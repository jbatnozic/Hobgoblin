// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Auxiliary_gameplay_manager_default.hpp"

namespace cinnabar {

DefaultAuxiliaryGameplayManager::DefaultAuxiliaryGameplayManager(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard,
                          QAO_ExeCon::GAMEPLAY,
                          PRIORITY_AUXGAMEPLAYMGR,
                          QAO_STATIC_NAME("cinnabar::DefaultAuxiliaryGameplayManager")} {}

QAO_Handle<ShipController> DefaultAuxiliaryGameplayManager::getAssociatedShipControllerOfLocalPlayer()
    const //
{
    const auto* rt = getRuntime();
    return rt->find<ShipController>("cinnabar::ShipController"); // TODO: temporary implementation
}

void DefaultAuxiliaryGameplayManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);
}

void DefaultAuxiliaryGameplayManager::_willDetach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_willDetach(aRuntime);
}

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>
#include <CBR/Ship/Ship_controller.hpp>

namespace cinnabar {

class AuxiliaryGameplayManager : public spe::ContextComponent {
public:
    virtual QAO_Handle<ShipController> getAssociatedShipControllerOfLocalPlayer() const = 0;

private:
    SPEMPE_CTXCOMP_TAG("cinnabar::AuxiliaryGameplayManager");
};

using MAuxGameplay = AuxiliaryGameplayManager;

} // namespace cinnabar

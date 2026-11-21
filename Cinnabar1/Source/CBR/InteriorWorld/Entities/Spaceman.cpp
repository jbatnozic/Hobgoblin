// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Spaceman.hpp"

#include <CBR/Ship/Ship_controller.hpp>

namespace cinnabar {

Spaceman::Spaceman(QAO_InstGuard aInstGuard)
    : IWSyncObjSuper{aInstGuard, QAO_ExeCon::GAMEPLAY, 0, QAO_STATIC_NAME("cinnabar::Spaceman")} //
{
    if (0) _doSyncCreate();
}

void Spaceman::_didAttach(QAO_Runtime& aRuntime) {
    IWSyncObjSuper::_didAttach(aRuntime);

    if (isMasterObject()) {
        _getCurrentState().initMirror();
    }
}

void Spaceman::_willDetach(QAO_Runtime& aRuntime) {
    IWSyncObjSuper::_willDetach(aRuntime);
}

void Spaceman::_eventPostUpdate(spe::IfMaster) {
    _getCurrentState().commit();
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(Spaceman, (UPDATE, DESTROY));

void Spaceman::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    // const auto& iw                = getInteriorWorld();
    // const auto& onboardPlayerInfo = iw.getOnboardPlayerInfo();
    // aSyncCtrl.filter([&](hg::PZInteger aClientIndex) -> spe::SyncFilterStatus {
    //     return onboardPlayerInfo.getSyncFilterStatusForClient(aClientIndex);
    // });
    // SPEMPE_SYNC_CREATE_DEFAULT_IMPL(Spaceman, aSyncCtrl);
}

void Spaceman::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(Spaceman, aSyncCtrl);
}

void Spaceman::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(Spaceman, aSyncCtrl);
}

} // namespace cinnabar

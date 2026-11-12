// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/InteriorWorld/Object_bases.hpp>
#include <Engine.hpp>

namespace cinnabar {

// clang-format off
SPEMPE_DEFINE_AUTODIFF_STATE(Spaceman_VisibleState,
    SPEMPE_MEMBER(double, x, 0.0),
    SPEMPE_MEMBER(double, y, 0.0)
) {};
// clang-format on

class Spaceman : public InteriorWorldSynchronizedObject<Spaceman_VisibleState> {
public:
    Spaceman(QAO_InstGuard aInstGuard);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;
    void _willDetach(QAO_Runtime& aRuntime) override;

    void _eventPostUpdate(spe::IfMaster) override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

QAO_REGISTER_CLASS(Spaceman, cinnabar_Spaceman) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::SynchronizedObjectBase>();
    klass.setMessageHandler<C, SyncCreateAndUpdateToClient, &C::msgSyncCreateAndUpdateToClient>();
}

} // namespace cinnabar

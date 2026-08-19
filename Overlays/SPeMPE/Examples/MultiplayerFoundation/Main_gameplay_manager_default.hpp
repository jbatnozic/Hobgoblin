// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include "Engine.h"
#include "Main_gameplay_manager.hpp"

class DefaultMainGameplayManager
    : public MainGameplayManager
    , public spe::NonstateObject
    , private spe::NetworkingEventListener {
public:
    explicit DefaultMainGameplayManager(QAO_InstGuard aInstGuard, int aExecutionPriority);

private:
    hg::PZInteger stateBufferingLength = 0;

    hg::PZInteger printBandwidthUsageCountdown = 120;

    void _didAttach(QAO_Runtime&) override;
    void _willDetach(QAO_Runtime&) override;

    void _eventUpdate1() override;
    void _eventPostUpdate() override;
    void _eventDrawGUI() override;

    void onNetworkingEvent(const RN_Event& aEvent) override;
};

QAO_REGISTER_CLASS(DefaultMainGameplayManager, Example_MainGameplayManager) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setSuperclass<spe::NonstateObject>();
}

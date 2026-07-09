// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include <string>

#include <gtest/gtest.h>

namespace test {

QAO_DEFINE_MESSAGE(SetPower, const double*);
QAO_DEFINE_MESSAGE(MsgWithoutPayload, QAO_NO_PAYLOAD);
QAO_DEFINE_MESSAGE(UnusedMessage, int*);

//! Reflection & Messaging tester
class RMTesterBase : public hg::QAO_Base {
public:
    RMTesterBase(hg::QAO_InstGuard aInstGuard)
        : QAO_Base{aInstGuard, hg::QAO_ExeCon::GAMEPLAY, 0, "RMTesterBase"} {}

    void setPower(SetPower::PayloadPtr aPower, bool aConst) {
        lastReceivedMessage             = "SetPower";
        constParamOfLastReceivedMessage = aConst;

        if (!aConst) {
            power = *aPower;
        }
    }

    void handleMsgWithoutPayload(MsgWithoutPayload::PayloadPtr, bool aConst) {
        lastReceivedMessage             = "MsgWithoutPayload";
        constParamOfLastReceivedMessage = aConst;
    }

    double      power                           = 0.0;
    std::string lastReceivedMessage             = "";
    bool        constParamOfLastReceivedMessage = false;

private:
};

QAO_REGISTER_CLASS(RMTesterBase, QAO_AutomaticTest_RMTesterBase) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setSuperclass<hg::QAO_Base>();
    clazz.setMessageHandler<C, SetPower, &C::setPower>();
    clazz.setMessageHandler<C, MsgWithoutPayload, &C::handleMsgWithoutPayload>();
}

class RMTesterDerived : public RMTesterBase {
public:
    using RMTesterBase::RMTesterBase;

    void setDoubledPower(SetPower::PayloadPtr aPower, bool aConst) {
        lastReceivedMessage             = "SetPower";
        constParamOfLastReceivedMessage = aConst;

        if (!aConst) {
            power = *aPower * 2.0;
        }
    }

private:
};

QAO_REGISTER_CLASS(RMTesterDerived, QAO_AutomaticTest_RMTesterDerived) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setSuperclass<RMTesterBase>();
    clazz.setMessageHandler<C, SetPower, &C::setDoubledPower>();
}

} // namespace test
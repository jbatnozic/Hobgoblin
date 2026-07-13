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

QAO_DEFINE_CLASS_MESSAGE(NegateInt, int*);

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

    static void negateInt(const hg::QAO_ClassMetadata&, NegateInt::PayloadPtr aInt) {
        (*aInt) = -(*aInt);
    }

    double      power                           = 0.0;
    std::string lastReceivedMessage             = "";
    bool        constParamOfLastReceivedMessage = false;

private:
};

QAO_REGISTER_CLASS(RMTesterBase, QAO_AutomaticTest_RMTesterBase) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<hg::QAO_Base>()
        .setMessageHandler<C, SetPower, &C::setPower>()
        .setMessageHandler<C, MsgWithoutPayload, &C::handleMsgWithoutPayload>()
        .setClassMessageHandler<NegateInt, &C::negateInt>();
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
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<RMTesterBase>();
    klass.setMessageHandler<C, SetPower, &C::setDoubledPower>();
}

} // namespace test

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>

namespace test {

QAO_DEFINE_MESSAGE(SetPower, const double);
QAO_DEFINE_MESSAGE(UnusedMessage, int);

//! Reflection & Messaging tester
class RMTesterBase : public hg::QAO_Base {
public:
    RMTesterBase(hg::QAO_InstGuard aInstGuard)
        : QAO_Base{aInstGuard, hg::QAO_ExeCon::GAMEPLAY, 0, "RMTesterBase"} {}

    void setPower(const double* aPower) {
        power = *aPower;
    }

    double power = 0.0;

private:
};

QAO_REGISTER_CLASS(RMTesterBase, QAO_AutomaticTest_RMTesterBase) {
    QAO_LOCAL_ALIAS(C, clazz);
    clazz.setMessageHandler<C, SetPower, &C::setPower>();
}

class RMTesterDerived : public RMTesterBase {
public:
    using RMTesterBase::RMTesterBase;

private:
};

QAO_REGISTER_CLASS(RMTesterDerived, QAO_AutomaticTest_RMTesterDerived) {
    // QAO_LOCAL_ALIAS(C, clazz);
}

} // namespace test
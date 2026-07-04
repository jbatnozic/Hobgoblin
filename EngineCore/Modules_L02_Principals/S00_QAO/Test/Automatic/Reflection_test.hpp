// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>

namespace test {

//! Reflection & Messaging tester
class RMTesterBase : public hg::QAO_Base {
public:
    using hg::QAO_Base::QAO_Base;

private:
};

class RMTesterDerived : public RMTesterBase {
public:
    using RMTesterBase::RMTesterBase;

private:
};

QAO_REGISTER_CLASS(RMTesterBase, QAO_AutomaticTest_RMTesterBase) {
    // QAO_LOCAL_ALIAS(C, clazz);
}

} // namespace test
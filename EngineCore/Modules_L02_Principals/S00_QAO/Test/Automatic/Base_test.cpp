// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>

using namespace hg::qao;

namespace {
// QAO_Base itself is not instantiable
class Derived : public QAO_Base {
    using QAO_Base::QAO_Base;
};
} // namespace

TEST(QAO_BaseTest, SetAndGetExeconThreshold) {
    auto d = QAO_Create<Derived>(nullptr, 0, "");

    d->setExeconThreshold(QAO_ExeCon::ESSENTIAL);
    EXPECT_EQ(d->getExeconThreshold(), QAO_ExeCon::ESSENTIAL);

    d->setExeconThreshold(QAO_ExeCon::SYNCHRONIZATION);
    EXPECT_EQ(d->getExeconThreshold(), QAO_ExeCon::SYNCHRONIZATION);

    d->setExeconThreshold(QAO_ExeCon::EXTRAS);
    EXPECT_EQ(d->getExeconThreshold(), QAO_ExeCon::EXTRAS);
}

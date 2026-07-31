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
    auto d = QAO_Create<Derived>(nullptr, QAO_ExeCon::META_EXECUTE_ALL, 0, "");

    d->setExeconThreshold(QAO_ExeCon::ESSENTIAL);
    EXPECT_EQ(d->getExeconThreshold(), QAO_ExeCon::ESSENTIAL);

    d->setExeconThreshold(QAO_ExeCon::SYNCHRONIZATION);
    EXPECT_EQ(d->getExeconThreshold(), QAO_ExeCon::SYNCHRONIZATION);

    d->setExeconThreshold(QAO_ExeCon::EXTRAS);
    EXPECT_EQ(d->getExeconThreshold(), QAO_ExeCon::EXTRAS);
}

TEST(QAO_BaseTest, InstanceHasRegularName) {
    auto d = QAO_Create<Derived>(nullptr, QAO_ExeCon::META_EXECUTE_ALL, 0, "Bob");

    EXPECT_EQ(d->getName().length(), 3);
    EXPECT_EQ(d->getName(), "Bob");
}

TEST(QAO_BaseTest, InstanceHasStaticName) {
    auto d = QAO_Create<Derived>(nullptr, QAO_ExeCon::META_EXECUTE_ALL, 0, QAO_STATIC_NAME("MrTorgue"));

    EXPECT_EQ(d->getName().length(), 8);
    EXPECT_EQ(d->getName(), "MrTorgue");
}

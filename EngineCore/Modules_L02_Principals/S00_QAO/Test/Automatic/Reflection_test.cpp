// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include "Reflection_test.hpp"

#include <gtest/gtest.h>

using namespace hg::qao;

namespace test {

class QAO_ReflectionTest : public ::testing::Test {
public:
    static void SetUpTestSuite() {
        hg::QAO_InitializeMetadata();
    }

private:
};

TEST_F(QAO_ReflectionTest, RegisteredClassCount) {
    EXPECT_EQ(QAO_ClassMetadata::getClassCount(), 3); // Two testers and QAO_Base
}

TEST_F(QAO_ReflectionTest, CheckQAO_BaseMetadata) {
    const auto* metadata        = QAO_ClassMetadata::get(typeid(QAO_Base));
    const auto* metadata_byName = QAO_ClassMetadata::get("UHOBGOBLIN_QAO_Base");

    ASSERT_NE(metadata, nullptr);
    EXPECT_EQ(metadata, metadata_byName);

    EXPECT_EQ(metadata->getUniqueName(), "UHOBGOBLIN_QAO_Base");
    EXPECT_EQ(metadata->getTypeInfo(), typeid(QAO_Base));
    EXPECT_EQ(metadata->getSuperclass(), nullptr);
    EXPECT_EQ(metadata->getChildClasses().size(), 1);
}

TEST_F(QAO_ReflectionTest, CheckBaseClassMetadata) {
    const auto* metadata        = QAO_ClassMetadata::get(typeid(RMTesterBase));
    const auto* metadata_byName = QAO_ClassMetadata::get("QAO_AutomaticTest_RMTesterBase");

    ASSERT_NE(metadata, nullptr);
    EXPECT_EQ(metadata, metadata_byName);

    EXPECT_EQ(metadata->getUniqueName(), "QAO_AutomaticTest_RMTesterBase");
    EXPECT_EQ(metadata->getTypeInfo(), typeid(RMTesterBase));
    EXPECT_EQ(metadata->getSuperclass(), QAO_ClassMetadata::get(typeid(QAO_Base)));
    EXPECT_EQ(metadata->getChildClasses().size(), 1);
}

TEST_F(QAO_ReflectionTest, CheckDerivedClassMetadata) {
    const auto* metadata        = QAO_ClassMetadata::get(typeid(RMTesterDerived));
    const auto* metadata_byName = QAO_ClassMetadata::get("QAO_AutomaticTest_RMTesterDerived");

    ASSERT_NE(metadata, nullptr);
    EXPECT_EQ(metadata, metadata_byName);

    EXPECT_EQ(metadata->getUniqueName(), "QAO_AutomaticTest_RMTesterDerived");
    EXPECT_EQ(metadata->getTypeInfo(), typeid(RMTesterDerived));
    EXPECT_EQ(metadata->getSuperclass(), QAO_ClassMetadata::get(typeid(RMTesterBase)));
    EXPECT_EQ(metadata->getChildClasses().size(), 0);
}

TEST_F(QAO_ReflectionTest, SendMessagesToBaseInstance) {
    auto inst = QAO_Create<RMTesterBase>(nullptr);

    {
        const double power   = 5.0;
        const bool   didSend = QAO_SendMessage<SetPower>((QAO_Base&)(*inst), &power);

        EXPECT_TRUE(didSend);
        EXPECT_EQ(inst->power, power);
        EXPECT_EQ(inst->lastReceivedMessage, "SetPower");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, false);
    }
    {
        const bool didSend = QAO_SendMessage<UnusedMessage>((QAO_Base&)(*inst), nullptr);

        EXPECT_FALSE(didSend);
        EXPECT_EQ(inst->lastReceivedMessage, "SetPower");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, false);
    }
    {
        const bool didSend = QAO_SendMessage<MsgWithoutPayload>((const QAO_Base&)(*inst), nullptr);

        EXPECT_TRUE(didSend);
        EXPECT_EQ(inst->lastReceivedMessage, "MsgWithoutPayload");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, true);
    }
}

TEST_F(QAO_ReflectionTest, SendMessagesToDerivedInstance) {
    auto inst = QAO_Create<RMTesterDerived>(nullptr);

    {
        const double power   = 5.0;
        const bool   didSend = QAO_SendMessage<SetPower>((QAO_Base&)(*inst), &power);

        EXPECT_TRUE(didSend);
        EXPECT_NEAR(inst->power, power * 2.0, 0.0001);
        EXPECT_EQ(inst->lastReceivedMessage, "SetPower");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, false);
    }
    {
        const bool didSend = QAO_SendMessage<UnusedMessage>((QAO_Base&)(*inst), nullptr);

        EXPECT_FALSE(didSend);
        EXPECT_EQ(inst->lastReceivedMessage, "SetPower");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, false);
    }
    {
        const bool didSend = QAO_SendMessage<MsgWithoutPayload>((const QAO_Base&)(*inst), nullptr);

        EXPECT_TRUE(didSend);
        EXPECT_EQ(inst->lastReceivedMessage, "MsgWithoutPayload");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, true);
    }
}

TEST_F(QAO_ReflectionTest, SendClassMessageToBaseClass) {
    const auto* klass = QAO_ClassMetadata::get("QAO_AutomaticTest_RMTesterBase");
    ASSERT_NE(klass, nullptr);

    int i = 5;

    const bool didSend = QAO_SendMessage<NegateInt>(*klass, &i);

    EXPECT_TRUE(didSend);
    EXPECT_EQ(i, -5);
}

// The derived class doesn't answer to any class messages
TEST_F(QAO_ReflectionTest, SendClassMessageToDerivedClass) {
    const auto* klass = QAO_ClassMetadata::get("QAO_AutomaticTest_RMTesterDerived");
    ASSERT_NE(klass, nullptr);

    const bool didSend = QAO_SendMessage<NegateInt>(*klass, nullptr);

    EXPECT_FALSE(didSend);
}

} // namespace test

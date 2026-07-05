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
private:
};

TEST_F(QAO_ReflectionTest, RegisteredClassCount) {
    EXPECT_EQ(QAO_ClassMetadata::getClassCount(), 2);
}

TEST_F(QAO_ReflectionTest, SendMessage) {
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
        const bool   didSend = QAO_SendMessage<MsgWithoutPayload>((const QAO_Base&)(*inst), nullptr);

        EXPECT_TRUE(didSend);
        EXPECT_EQ(inst->lastReceivedMessage, "MsgWithoutPayload");
        EXPECT_EQ(inst->constParamOfLastReceivedMessage, true);
    }
}

} // namespace test

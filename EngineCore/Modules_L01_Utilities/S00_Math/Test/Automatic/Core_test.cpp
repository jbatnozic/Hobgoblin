// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Math/Core.hpp>

#include <gtest/gtest.h>

namespace jbatnozic {
namespace hobgoblin {
namespace math {

TEST(MathCoreTest, IntegralCeilDivSignedTest) {
    EXPECT_EQ(IntegralCeilDiv(0, 2), 0);
    EXPECT_EQ(IntegralCeilDiv(1, 3), 1);
    EXPECT_EQ(IntegralCeilDiv(7, 3), 3);
    EXPECT_EQ(IntegralCeilDiv(19, 2), 10);
    EXPECT_EQ(IntegralCeilDiv(6, 6), 1);
    EXPECT_EQ(IntegralCeilDiv(-6, 6), -1);
    EXPECT_EQ(IntegralCeilDiv(-6, -6), 1);
    EXPECT_EQ(IntegralCeilDiv(-5, 6), 0);
    EXPECT_EQ(IntegralCeilDiv(-7, 6), -1);
}

TEST(MathCoreTest, IntegralCeilDivUnsignedTest) {
    EXPECT_EQ(IntegralCeilDiv(0u, 2u), 0u);
    EXPECT_EQ(IntegralCeilDiv(1u, 3u), 1u);
    EXPECT_EQ(IntegralCeilDiv(7u, 3u), 3u);
    EXPECT_EQ(IntegralCeilDiv(19u, 2u), 10u);
    EXPECT_EQ(IntegralCeilDiv(6u, 6u), 1u);
}

} // namespace math
} // namespace hobgoblin
} // namespace jbatnozic

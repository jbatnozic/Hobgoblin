// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Math/Vector2.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace jbatnozic {
namespace hobgoblin {
namespace math {

namespace {
Vector2d Identity(Vector2d aVec) {
    return aVec;
}
} // namespace

TEST(Vector2Test, Conversions) {
    Vector2d vecd = {1.0, 2.0};
    Vector2f vecf = vecd.cast<float>();

    EXPECT_EQ(Identity(vecf), vecd);
}

} // namespace math
} // namespace hobgoblin
} // namespace jbatnozic

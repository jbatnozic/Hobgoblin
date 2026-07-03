// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Preprocessor.hpp>

#include "Do_before_main.hpp"

#include <gtest/gtest.h>

TEST(DoBeforeMainTest, UsingDoBeforeMainMacroInHeaderStillInvokesBodyOnlyOnce) {
    int& counter = GetTestCounter();
    EXPECT_EQ(counter, 1);
}

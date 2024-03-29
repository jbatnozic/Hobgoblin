#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/GSL.hpp>

#include <gtest/gtest.h>

#include <memory>

// This is just to check that it compiles.
TEST(HobgoblinGSLTest, DISABLED_NotNullTest) {
    void* null = nullptr;
    hg::NotNull<void*> nn{null};
}

// This is just to check that it compiles.
TEST(HobgoblinGSLTest, DISABLED_NotNullWithUniquePtr) {
    hg::NotNull<std::unique_ptr<int>> nn{std::make_unique<int>(123)};
}

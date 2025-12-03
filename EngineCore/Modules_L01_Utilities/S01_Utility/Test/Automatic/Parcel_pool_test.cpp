// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Log_with_scoped_stopwatch.hpp>
#include <Hobgoblin/Utility/Parcel_pool.hpp>

#include <cstdlib>

#include <gtest/gtest.h>

namespace jbatnozic {
namespace hobgoblin {
namespace util {

TEST(ParcelPoolTest, CreateMethodReturnsNonNull) {
    auto pool = ParcelPool::create();
    EXPECT_NE(pool, nullptr);
}

TEST(ParcelPoolTest, ObtainParcelWithDefaultSizeZero) {
    auto pp     = ParcelPool::create();
    auto parcel = pp->obtainParcel(64);
    EXPECT_EQ(parcel.getCapacity(), 64);
    EXPECT_EQ(parcel.getSize(), 0);
}

TEST(ParcelPoolTest, ParcelIsReused) {
    auto        pp   = ParcelPool::create();
    const char* pchr = nullptr;
    {
        auto parcel = pp->obtainParcel(64);
        pchr        = parcel.getData();
    }
    {
        auto        parcel = pp->obtainParcel(64);
        const char* pchr2  = parcel.getData();
        EXPECT_EQ(pchr, pchr2);
    }
}

} // namespace util
} // namespace hobgoblin
} // namespace jbatnozic

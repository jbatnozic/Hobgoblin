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
    EXPECT_GE(parcel.getCapacity(), 64);
    EXPECT_EQ(parcel.getSize(), 0);
}

TEST(ParcelPoolTest, CheckIsGood) {
    auto pp     = ParcelPool::create();
    auto parcel = pp->obtainParcel(64);
    EXPECT_TRUE(parcel.isGood());
    parcel.relinquish();
    EXPECT_FALSE(parcel.isGood());
}

TEST(ParcelPoolTest, DISABLED_ParcelIsReused) {
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

TEST(ParcelPoolTest, DISABLED_ParcelVsMalloc) {
    constexpr int ITERATION_COUNT = 1'000'000 / 1000;

    log::SetMinimalLogSeverity(log::Severity::All);

    {
        HG_LOG_WITH_SCOPED_STOPWATCH_MS(INFO,
                                        "Hobgoblin.Utility.AutomaticTest",
                                        "ParcelVsMalloc: malloc/free took {}ms.",
                                        elapsed_time_ms);

        for (int i = 0; i < ITERATION_COUNT; ++i) {
            auto* p = malloc(1024);
            free(p);
        }
    }

    {
        auto pp = ParcelPool::create();

        HG_LOG_WITH_SCOPED_STOPWATCH_MS(INFO,
                                        "Hobgoblin.Utility.AutomaticTest",
                                        "ParcelVsMalloc: parcels took {}ms.",
                                        elapsed_time_ms);

        for (int i = 0; i < ITERATION_COUNT; ++i) {
            auto parcel = pp->obtainParcel(1024);
        }
    }
}

} // namespace util
} // namespace hobgoblin
} // namespace jbatnozic

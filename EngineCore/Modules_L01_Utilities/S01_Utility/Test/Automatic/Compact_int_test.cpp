// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Compact_int.hpp>
#include <Hobgoblin/Utility/Stream_buffer.hpp>

#include <gtest/gtest.h>

#include <cstdint>

namespace jbatnozic {
namespace hobgoblin {
namespace util {

// MARK: Unsigned 56

class CompactUInt56Test : public ::testing::Test {
protected:
    void _testWithValue(std::uint64_t aValue, PZInteger aExpectedPacketSize) {
        SCOPED_TRACE("Testing CompactUInt56 with value " + std::to_string(aValue) +
                     " and expected packet size of " + std::to_string(aExpectedPacketSize) + ".");

        BufferStream bufStream;

        CompactUInt56 original{aValue};
        bufStream << original;
        EXPECT_EQ(bufStream.getRemainingDataSize(), aExpectedPacketSize);

        CompactUInt56 extracted;
        bufStream.noThrow() >> extracted;
        EXPECT_FALSE(bufStream.hasReadError());
        EXPECT_EQ(bufStream.getRemainingDataSize(), 0);

        EXPECT_EQ(original, extracted);
        EXPECT_EQ(original.value(), aValue);
        EXPECT_EQ(extracted.value(), aValue);
    }
};

TEST_F(CompactUInt56Test, TestValidValues) {
    _testWithValue(0, 1);
    _testWithValue(1, 1);

    // clang-format off
    _testWithValue((0x01ULL <<  7) - 1, 1);
    _testWithValue((0x01ULL <<  7) - 0, 2);
    _testWithValue((0x01ULL << 14) - 1, 2);
    _testWithValue((0x01ULL << 14) - 0, 3);
    _testWithValue((0x01ULL << 21) - 1, 3);
    _testWithValue((0x01ULL << 21) - 0, 4);
    _testWithValue((0x01ULL << 28) - 1, 4);
    _testWithValue((0x01ULL << 28) - 0, 5);
    _testWithValue((0x01ULL << 35) - 1, 5);
    _testWithValue((0x01ULL << 35) - 0, 6);
    _testWithValue((0x01ULL << 42) - 1, 6);
    _testWithValue((0x01ULL << 42) - 0, 7);
    _testWithValue((0x01ULL << 49) - 1, 7);
    _testWithValue((0x01ULL << 49) - 0, 8);
    _testWithValue((0x01ULL << 56) - 1, 8);
    // clang-format on
}

// MARK: Unsigned 28

class CompactUInt28Test : public ::testing::Test {
protected:
    void _testWithValue(std::uint32_t aValue, PZInteger aExpectedPacketSize) {
        SCOPED_TRACE("Testing CompactUInt28 with value " + std::to_string(aValue) +
                     " and expected packet size of " + std::to_string(aExpectedPacketSize) + ".");

        BufferStream bufStream;

        CompactUInt28 original{aValue};
        bufStream << original;
        EXPECT_EQ(bufStream.getRemainingDataSize(), aExpectedPacketSize);

        CompactUInt28 extracted;
        bufStream.noThrow() >> extracted;
        EXPECT_FALSE(bufStream.hasReadError());
        EXPECT_EQ(bufStream.getRemainingDataSize(), 0);

        EXPECT_EQ(original, extracted);
        EXPECT_EQ(original.value(), aValue);
        EXPECT_EQ(extracted.value(), aValue);
    }
};

TEST_F(CompactUInt28Test, TestValidValues) {
    _testWithValue(0, 1);
    _testWithValue(1, 1);

    // clang-format off
    _testWithValue((0x01ULL <<  7) - 1, 1);
    _testWithValue((0x01ULL <<  7) - 0, 2);
    _testWithValue((0x01ULL << 14) - 1, 2);
    _testWithValue((0x01ULL << 14) - 0, 3);
    _testWithValue((0x01ULL << 21) - 1, 3);
    _testWithValue((0x01ULL << 21) - 0, 4);
    _testWithValue((0x01ULL << 28) - 1, 4);
    // clang-format on
}

// MARK: Signed 56

class CompactInt56Test : public ::testing::Test {
protected:
    void _testWithValue(std::int64_t aValue, PZInteger aExpectedPacketSize) {
        SCOPED_TRACE("Testing CompactInt56 with value " + std::to_string(aValue) +
                     " and expected packet size of " + std::to_string(aExpectedPacketSize) + ".");

        BufferStream bufStream;

        CompactInt56 original{aValue};
        bufStream << original;
        EXPECT_EQ(bufStream.getRemainingDataSize(), aExpectedPacketSize);

        CompactInt56 extracted;
        bufStream.noThrow() >> extracted;
        EXPECT_FALSE(bufStream.hasReadError());
        EXPECT_EQ(bufStream.getRemainingDataSize(), 0);

        EXPECT_EQ(original, extracted);
        EXPECT_EQ(original.value(), aValue);
        EXPECT_EQ(extracted.value(), aValue);
    }
};

TEST_F(CompactInt56Test, TestValidValues) {
    _testWithValue(0, 1);
    _testWithValue(1, 1);
    _testWithValue(-1, 1);

    // clang-format off
    // Positive values
    _testWithValue((0x01LL << ( 7 - 1)) - 1, 1);
    _testWithValue((0x01LL << ( 7 - 1)) - 0, 2);
    _testWithValue((0x01LL << (14 - 1)) - 1, 2);
    _testWithValue((0x01LL << (14 - 1)) - 0, 3);
    _testWithValue((0x01LL << (21 - 1)) - 1, 3);
    _testWithValue((0x01LL << (21 - 1)) - 0, 4);
    _testWithValue((0x01LL << (28 - 1)) - 1, 4);
    _testWithValue((0x01LL << (28 - 1)) - 0, 5);
    _testWithValue((0x01LL << (35 - 1)) - 1, 5);
    _testWithValue((0x01LL << (35 - 1)) - 0, 6);
    _testWithValue((0x01LL << (42 - 1)) - 1, 6);
    _testWithValue((0x01LL << (42 - 1)) - 0, 7);
    _testWithValue((0x01LL << (49 - 1)) - 1, 7);
    _testWithValue((0x01LL << (49 - 1)) - 0, 8);
    _testWithValue((0x01LL << (56 - 1)) - 1, 8);

    // Negative values
    _testWithValue(-(0x01LL << ( 7 - 1)) - 0, 1);
    _testWithValue(-(0x01LL << ( 7 - 1)) - 1, 2);
    _testWithValue(-(0x01LL << (14 - 1)) - 0, 2);
    _testWithValue(-(0x01LL << (14 - 1)) - 1, 3);
    _testWithValue(-(0x01LL << (21 - 1)) - 0, 3);
    _testWithValue(-(0x01LL << (21 - 1)) - 1, 4);
    _testWithValue(-(0x01LL << (28 - 1)) - 0, 4);
    _testWithValue(-(0x01LL << (28 - 1)) - 1, 5);
    _testWithValue(-(0x01LL << (35 - 1)) - 0, 5);
    _testWithValue(-(0x01LL << (35 - 1)) - 1, 6);
    _testWithValue(-(0x01LL << (42 - 1)) - 0, 6);
    _testWithValue(-(0x01LL << (42 - 1)) - 1, 7);
    _testWithValue(-(0x01LL << (49 - 1)) - 0, 7);
    _testWithValue(-(0x01LL << (49 - 1)) - 1, 8);
    _testWithValue(-(0x01LL << (56 - 1)) - 0, 8);
    // clang-format on
}

// MARK: Signed 28

class CompactInt28Test : public ::testing::Test {
protected:
    void _testWithValue(std::int32_t aValue, PZInteger aExpectedPacketSize) {
        SCOPED_TRACE("Testing CompactInt28 with value " + std::to_string(aValue) +
                     " and expected packet size of " + std::to_string(aExpectedPacketSize) + ".");

        BufferStream bufStream;

        CompactInt28 original{aValue};
        bufStream << original;
        EXPECT_EQ(bufStream.getRemainingDataSize(), aExpectedPacketSize);

        CompactInt28 extracted;
        bufStream.noThrow() >> extracted;
        EXPECT_FALSE(bufStream.hasReadError());
        EXPECT_EQ(bufStream.getRemainingDataSize(), 0);

        EXPECT_EQ(original, extracted);
        EXPECT_EQ(original.value(), aValue);
        EXPECT_EQ(extracted.value(), aValue);
    }
};

TEST_F(CompactInt28Test, TestValidValues) {
    _testWithValue(0, 1);
    _testWithValue(1, 1);
    _testWithValue(-1, 1);

    // clang-format off
    // Positive values
    _testWithValue((0x01 << ( 7 - 1)) - 1, 1);
    _testWithValue((0x01 << ( 7 - 1)) - 0, 2);
    _testWithValue((0x01 << (14 - 1)) - 1, 2);
    _testWithValue((0x01 << (14 - 1)) - 0, 3);
    _testWithValue((0x01 << (21 - 1)) - 1, 3);
    _testWithValue((0x01 << (21 - 1)) - 0, 4);
    _testWithValue((0x01 << (28 - 1)) - 1, 4);

    // Negative values
    _testWithValue(-(0x01 << ( 7 - 1)) - 0, 1);
    _testWithValue(-(0x01 << ( 7 - 1)) - 1, 2);
    _testWithValue(-(0x01 << (14 - 1)) - 0, 2);
    _testWithValue(-(0x01 << (14 - 1)) - 1, 3);
    _testWithValue(-(0x01 << (21 - 1)) - 0, 3);
    _testWithValue(-(0x01 << (21 - 1)) - 1, 4);
    _testWithValue(-(0x01 << (28 - 1)) - 0, 4);
    // clang-format on
}

} // namespace util
} // namespace hobgoblin
} // namespace jbatnozic

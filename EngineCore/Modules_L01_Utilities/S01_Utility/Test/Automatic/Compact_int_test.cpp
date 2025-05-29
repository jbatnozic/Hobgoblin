// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Compact_int.hpp>
#include <Hobgoblin/Utility/Stream_buffer.hpp>

#include <gtest/gtest.h>

#include <cstdint>

namespace jbatnozic {
namespace hobgoblin {
namespace util {

class CompactUInt56Test : public ::testing::Test {
protected:
    void _testWithValue(std::uint64_t aValue, PZInteger aExpectedPacketSize) {
        SCOPED_TRACE("Testing CompactUInt56Test with value " + std::to_string(aValue) +
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

class CompactUInt28Test : public ::testing::Test {
protected:
    void _testWithValue(std::uint32_t aValue, PZInteger aExpectedPacketSize) {
        SCOPED_TRACE("Testing CompactUInt28Test with value " + std::to_string(aValue) +
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

} // namespace util
} // namespace hobgoblin
} // namespace jbatnozic

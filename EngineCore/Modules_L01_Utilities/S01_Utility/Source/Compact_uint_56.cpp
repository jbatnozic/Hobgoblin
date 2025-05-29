// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Utility/Compact_uint_56.hpp>

#include <bit>
#include <cstring>

#if defined(_MSC_VER)
#include <intrin.h> // Compiler intrincics for BitWidth()
#pragma intrinsic(_BitScanReverse64)
#endif

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

namespace {
//! BitWidth return the number of bits needed to represent an unsigned int (64 bit) value.
//! \warning the return value is UNDEFINED if `aValue` is 0!
PZInteger BitWidth(std::uint64_t aValue) {
#if defined(_MSC_VER)
    unsigned long index;
    _BitScanReverse64(&index, aValue);
    return static_cast<PZInteger>(index + 1);
#else
    const auto width = 64 - __builtin_clzll(aValue);
    return static_cast<PZInteger>(width);
#endif
}

PZInteger CountTrailingZeros(std::uint64_t aValue) {
    return std::countr_zero(aValue);
}
} // namespace

OutputStream& operator<<(OutputStreamExtender& aStreamExt, CompactUInt56 aData) {
    std::uint64_t val = aData._value;

    if (val == 0) {
        aStreamExt->append((std::uint8_t)0x01);
    } else {
        aData._validateRange();

        const auto packWidth = (BitWidth(val) + 6) / 7;

        val <<= packWidth;
        val |= (1 << (packWidth - 1));

        static_assert(HG_ENDIANESS == HG_LITTLE_ENDIAN); // TODO explain why
        aStreamExt->write(&val, packWidth);
    }

    return *aStreamExt;
}

InputStream& operator>>(InputStreamExtender& aStreamExt, CompactUInt56& aData) {
    std::uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0 ,0};

    aStreamExt->noThrow() >> buffer[0];
    if (buffer[0] <= 1) {
        aData = 0;
    } else {
        const auto units = CountTrailingZeros(buffer[0]);
        (void)aStreamExt->read(&buffer[1], units);
        aData = 0;
        static_assert(HG_ENDIANESS == HG_LITTLE_ENDIAN); // TODO explain why
        std::memcpy(&*aData, &buffer, 8);
        *aData >>= units + 1;
    }

    return *aStreamExt;
}

void CompactUInt56::_validateRange() {
    static constexpr std::uint64_t LIMIT = 0x00'FF'FF'FF'FF'FF'FF'FF;

    if (_value <= LIMIT) {
        return;
    }

    HG_THROW_TRACED(TracedLogicError,
                    0,
                    "The stored value ({}) is outside of the 56-bit range.",
                    _value);
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

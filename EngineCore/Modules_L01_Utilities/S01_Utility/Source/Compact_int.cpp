// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Utility/Compact_int.hpp>

#include <bit>
#include <cstring>
#include <type_traits>

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

template <class T>
T IntWithTopNBitsSet(PZInteger aN) {
    static constexpr PZInteger BIT_COUNT = sizeof(T) * 8;

    if (aN >= BIT_COUNT) {
        return ~T(0);
    }

    return ~T(0) << (BIT_COUNT - aN);
}
} // namespace

template <class taValueType>
OutputStream& operator<<(OutputStreamExtender& aStreamExt, CompactIntImpl<taValueType> aData) {
    static constexpr bool IS_SIGNED = std::is_signed_v<taValueType>;

    taValueType val = aData.value();

    aData.validateValueRange();

    if (val == 0) {
        aStreamExt->append((std::uint8_t)0x01);
    } else {
        aData.validateValueRange();

        PZInteger packWidth;
        if (val > 0) {
            packWidth = (BitWidth(val) + 6 + (IS_SIGNED ? 1 : 0)) / 7;
        } else {
            packWidth = (BitWidth(~static_cast<std::uint64_t>(val)) + 6 + (IS_SIGNED ? 1 : 0)) / 7;
        }

        val <<= packWidth;
        val |= (1 << (packWidth - 1));

        // This function, as currently implemented, depends on the least significat byte being in
        // the lowest address, because that byte has a marker for how many bytes follow it (if any).
        static_assert(HG_ENDIANESS == HG_LITTLE_ENDIAN);
        aStreamExt->write(&val, packWidth);
    }

    return *aStreamExt;
}

template <class taValueType>
InputStream& operator>>(InputStreamExtender& aStreamExt, CompactIntImpl<taValueType>& aData) {
    std::uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    aStreamExt->noThrow() >> buffer[0];
    if (buffer[0] <= 1) {
        aData = 0;
        return *aStreamExt;
    }

    if constexpr (std::is_unsigned_v<taValueType>) { // UNSIGNED
        const auto units = CountTrailingZeros(buffer[0]);
        (void)aStreamExt->read(&buffer[1], units);
        aData = 0;
        // The insertion operator (<<) stores the number in the stream in the little-endian format.
        static_assert(HG_ENDIANESS == HG_LITTLE_ENDIAN);
        std::memcpy(&*aData, &buffer, sizeof(taValueType));
        *aData >>= units + 1;
    } else { // SIGNED
        const auto extraOctets = CountTrailingZeros(buffer[0]);
        (void)aStreamExt->read(&buffer[1], extraOctets);
        aData = 0;
        // The insertion operator (<<) stores the number in the stream in the little-endian format.
        static_assert(HG_ENDIANESS == HG_LITTLE_ENDIAN);
        std::memcpy(&*aData, &buffer, sizeof(taValueType));
        *aData >>= extraOctets + 1;
        if ((buffer[extraOctets] & 0x80) != 0) {
            const auto leading1count = (sizeof(taValueType) * 8) - ((extraOctets + 1) * 7);
            *aData |= IntWithTopNBitsSet<taValueType>(leading1count);
        }
    }

    return *aStreamExt;
}

template <class taValueType>
void CompactIntImpl<taValueType>::validateValueRange() {
    if constexpr (std::is_unsigned_v<taValueType>) { // UNSIGNED
        static constexpr std::uint32_t LIMIT_32 = 0x0F'FF'FF'FF;
        static constexpr std::uint64_t LIMIT_64 = 0x00'FF'FF'FF'FF'FF'FF'FF;

        if constexpr (sizeof(taValueType) == 4) {
            if (_value <= LIMIT_32) {
                return;
            }

            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "The stored value ({}) is outside of the 28-bit range.",
                            _value);
        } else {
            if (_value <= LIMIT_64) {
                return;
            }

            HG_THROW_TRACED(TracedLogicError,
                            0,
                            "The stored value ({}) is outside of the 56-bit range.",
                            _value);
        }
    } else { // SIGNED
        // TODO
    }
}

///////////////////////////////////////////////////////////////////////////
// MARK: EXPLICIT INSTANTIATIONS                                         //
///////////////////////////////////////////////////////////////////////////

template class CompactIntImpl<std::uint64_t>;
template class CompactIntImpl<std::uint32_t>;
template class CompactIntImpl<std::int64_t>;
template class CompactIntImpl<std::int32_t>;

template OutputStream& operator<<(OutputStreamExtender& aStreamExt, CompactIntImpl<std::uint64_t> aData);
template OutputStream& operator<<(OutputStreamExtender& aStreamExt, CompactIntImpl<std::uint32_t> aData);
template OutputStream& operator<<(OutputStreamExtender& aStreamExt, CompactIntImpl<std::int64_t> aData);
template OutputStream& operator<<(OutputStreamExtender& aStreamExt, CompactIntImpl<std::int32_t> aData);

template InputStream& operator>>(InputStreamExtender& aStreamExt, CompactIntImpl<std::uint64_t>& aData);
template InputStream& operator>>(InputStreamExtender& aStreamExt, CompactIntImpl<std::uint32_t>& aData);
template InputStream& operator>>(InputStreamExtender& aStreamExt, CompactIntImpl<std::int64_t>& aData);
template InputStream& operator>>(InputStreamExtender& aStreamExt, CompactIntImpl<std::int32_t>& aData);

} // namespace util
HOBGOBLIN_NAMESPACE_END

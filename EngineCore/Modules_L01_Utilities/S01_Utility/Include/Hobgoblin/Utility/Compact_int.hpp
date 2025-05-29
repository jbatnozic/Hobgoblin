// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UTIL_COMPACT_UINT_56_HPP
#define UHOBGOBLIN_UTIL_COMPACT_UINT_56_HPP

#include <Hobgoblin/Utility/Stream_input.hpp>
#include <Hobgoblin/Utility/Stream_output.hpp>

#include <cstdint>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

//! An instance of a CompactInt (either signed or unsigned) represents a basic integer of a certain
//! width. What sets it apart from a primitive integer type is the somewhat reduced range of values
//! traded for the ability to be packed into an output stream (`hg::util::OutputStream`) or packet
//! (`hg::util::Packet`) using fewer total bytes the closer the actual value is to zero.
//!
//! As such, this is a type whose intended use is network bandwidth optimization.
//!
//! \note do not use this class template directly; use one of the concrete aliases provided below:
//!       - CompactUInt56
//!       - CompactUInt28
//!       - CompactInt56
//!       - CompactInt28
template <class taValueType>
class CompactIntImpl {
public:
    CompactIntImpl(taValueType aValue = 0)
        : _value{aValue} {}

    CompactIntImpl(const CompactIntImpl& aOther) = default;

    CompactIntImpl& operator=(const CompactIntImpl& aOther) = default;

    operator taValueType() const {
        return _value;
    }

    CompactIntImpl& operator=(taValueType aValue) {
        _value = aValue;
        return SELF;
    }

    // Value accessors

    taValueType& operator*() {
        return _value;
    }

    taValueType operator*() const {
        return _value;
    }

    taValueType value() const {
        return _value;
    }

    // Comparison operators

    bool operator==(const CompactIntImpl& aOther) {
        return _value == aOther._value;
    }

    bool operator!=(const CompactIntImpl& aOther) {
        return _value != aOther._value;
    }

    bool operator<(const CompactIntImpl& aOther) {
        return _value < aOther._value;
    }

    bool operator<=(const CompactIntImpl& aOther) {
        return _value <= aOther._value;
    }

    bool operator>(const CompactIntImpl& aOther) {
        return _value > aOther._value;
    }

    bool operator>=(const CompactIntImpl& aOther) {
        return _value >= aOther._value;
    }

    // Misc.

    //! If the stored value is outside of the 28-bit or the 56-bit range (depending on `taValueType`),
    //! throws `TracedLogicError`. If the value is in range, it does nothing.
    void validateValueRange();

private:
    taValueType _value;
};

template <class taValueType>
OutputStream& operator<<(OutputStreamExtender& aStreamExtender, CompactIntImpl<taValueType> aData);

template <class taValueType>
InputStream& operator>>(InputStreamExtender& aStreamExtender, CompactIntImpl<taValueType>& aData);

//! Unsigned CompactInt implementation that can hold up to 56 bits of information.
//! With this class, numbers that can be represented using up to 7 bits will take up 1 byte
//! when packed to a stream, numbers that can be represented up to 14 bits will take up 2 bytes
//! when packed into a stream, and so on (max = 56-bit number -> 8 bytes in a stream).
//!      0..127     -> 1B
//!    128..16383   -> 2B
//!  16384..2097151 -> 3B
//!  etc.
using CompactUInt56 = CompactIntImpl<std::uint64_t>;

//! Unsigned CompactInt implementation that can hold up to 28 bits of information.
//! With this class, numbers that can be represented using up to 7 bits will take up 1 byte
//! when packed to a stream, numbers that can be represented up to 14 bits will take up 2 bytes
//! when packed into a stream, and so on (max = 28-bit number -> 4 bytes in a stream).
//!      0..127     -> 1B
//!    128..16383   -> 2B
//!  16384..2097151 -> 3B
//!  etc.
using CompactUInt28 = CompactIntImpl<std::uint32_t>;

//! Signed CompactInt implementation that can hold up to 56 bits of information.
//! With this class, numbers that can be represented using up to 7 bits will take up 1 byte
//! when packed to a stream, numbers that can be represented up to 14 bits will take up 2 bytes
//! when packed into a stream, and so on (max = 56-bit number -> 8 bytes in a stream).
//!       -64..63      -> 1B
//!     -8192..8191    -> 2B
//!  -1048576..1048575 -> 3B
//!  etc.
using CompactInt56 = CompactIntImpl<std::int64_t>;

//! Signed CompactInt implementation that can hold up to 28 bits of information.
//! With this class, numbers that can be represented using up to 7 bits will take up 1 byte
//! when packed to a stream, numbers that can be represented up to 14 bits will take up 2 bytes
//! when packed into a stream, and so on (max = 28-bit number -> 4 bytes in a stream).
//!       -64..63      -> 1B
//!     -8192..8191    -> 2B
//!  -1048576..1048575 -> 3B
//!  etc.
using CompactInt28 = CompactIntImpl<std::int32_t>;

} // namespace util
HOBGOBLIN_NAMESPACE_END

#endif // !UHOBGOBLIN_UTIL_COMPACT_UINT_56_HPP

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
using CompactUInt56 = CompactIntImpl<std::uint64_t>;

//! Unsigned CompactInt implementation that can hold up to 28 bits of information.
using CompactUInt28 = CompactIntImpl<std::uint32_t>;

//! Signed CompactInt implementation that can hold up to 56 bits of information.
using CompactInt56 = CompactIntImpl<std::int64_t>;

} // namespace util
HOBGOBLIN_NAMESPACE_END

#endif // !UHOBGOBLIN_UTIL_COMPACT_UINT_56_HPP

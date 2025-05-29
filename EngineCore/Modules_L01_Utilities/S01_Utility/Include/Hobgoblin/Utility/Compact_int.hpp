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
    void validateValueRange();

private:
    taValueType _value;
};

template <class taValueType>
OutputStream& operator<<(OutputStreamExtender& aStreamExtender, CompactIntImpl<taValueType> aData);

template <class taValueType>
InputStream& operator>>(InputStreamExtender& aStreamExtender, CompactIntImpl<taValueType>& aData);

using CompactUInt56 = CompactIntImpl<std::uint64_t>;
using CompactUInt28 = CompactIntImpl<std::uint32_t>;

} // namespace util
HOBGOBLIN_NAMESPACE_END

#endif // !UHOBGOBLIN_UTIL_COMPACT_UINT_56_HPP

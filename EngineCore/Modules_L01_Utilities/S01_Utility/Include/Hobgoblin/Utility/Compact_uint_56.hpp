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

class CompactUInt56 {
public:
    CompactUInt56(std::uint64_t aValue = 0) :  _value{aValue} {}

    CompactUInt56(const CompactUInt56& aOther) = default;

    CompactUInt56& operator=(const CompactUInt56& aOther) = default;

    operator std::int64_t() const {
        return _value;
    }

    std::uint64_t& operator*() {
        return _value;
    }

    std::uint64_t operator*() const {
        return _value;
    }

    CompactUInt56& operator=(std::uint64_t aValue) {
        _value = aValue;
        return SELF;
    }

    std::int64_t value() const {
        return _value;
    }

    bool operator==(const CompactUInt56& aOther) {
        return _value == aOther._value;
    }

    bool operator!=(const CompactUInt56& aOther) {
        return _value != aOther._value;
    }

    friend OutputStream& operator<<(OutputStreamExtender& aStreamExtender, CompactUInt56 aData);
    friend InputStream&  operator>>(InputStreamExtender& aStreamExtender, CompactUInt56& aData);

private:
    std::uint64_t _value;

    void _validateRange();
};

} // namespace util
HOBGOBLIN_NAMESPACE_END

#endif // !UHOBGOBLIN_UTIL_COMPACT_UINT_56_HPP

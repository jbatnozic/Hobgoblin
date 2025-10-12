// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Math/Hash.hpp>

#include <bit>
#include <cstdint>
#include <limits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace math {
namespace detail {

namespace {
template <class T>
T XorShift(const T& n, int i) {
    return n ^ (n >> i);
}

#ifdef NEED_32_BIT_SUPPORT
// a hash function with another name as to not confuse with std::hash
std::uint32_t Distribute(const std::uint32_t& n) {
    const std::uint32_t P = 0x55555555ul; // pattern of alternating 0 and 1
    const std::uint32_t C = 3423571495ul; // random uneven integer constant;
    return C * XorShift(P * XorShift(n, 16), 16);
}
#endif

// a hash function with another name as to not confuse with std::hash
std::uint64_t Distribute(const std::uint64_t& n) {
    const std::uint64_t P = 0x5555555555555555ull;   // pattern of alternating 0 and 1
    const std::uint64_t C = 17316035218449499591ull; // random uneven integer constant;
    return C * XorShift(P * XorShift(n, 32), 32);
}
} // namespace

// Source: https://stackoverflow.com/a/50978188
void HashCombineImpl(std::size_t& aOldSeed, std::size_t aNewSeed) {
    aOldSeed = std::rotl(aOldSeed, std::numeric_limits<std::size_t>::digits / 3) ^ Distribute(aNewSeed);
}

} // namespace detail
} // namespace math
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

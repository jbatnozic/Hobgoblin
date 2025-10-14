// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_MATH_HASH_HPP
#define UHOBGOBLIN_MATH_HASH_HPP

#include <cstddef>
#include <memory> // for std::hash

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace math {

namespace detail {
void HashCombineImpl(std::size_t& aOldSeed, std::size_t aNewSeed);
} // namespace detail

//! \brief Combines the hash values of one or more objects into a single hash.
//!
//! This utility function is used to incrementally combine multiple hash values
//! into a single hash value. It is useful when implementing custom hash functions
//! (e.g., for `std::unordered_map` keys) where multiple members of a type need to be hashed.
//!
//! Internally, this function uses `std::hash` to hash each object, and then combines
//! those hash values into the given seed using a deterministic method.
//!
//! \tparam taObjects Variadic template parameter pack for the types of the objects to hash.
//!
//! \param[in, out] aSeed The seed value to combine with the hash of the given objects.
//!                       Typically initialized to 0. After the function returns, this will
//!                       hold the combined hash.
//! \param[in] aObjects One or more objects whose hash values will be combined into `aSeed`.
//!
//! \note The combination is order-sensitive; i.e., `HashCombine(seed, a, b)` will produce
//!       a different result than `HashCombine(seed, b, a)` if `a != b`.
template <class... taObjects>
void HashCombine(std::size_t& aSeed, const taObjects&... aObjects) {
    (detail::HashCombineImpl(aSeed, std::hash<taObjects>{}(aObjects)), ...);
}

} // namespace math
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_MATH_HASH_HPP

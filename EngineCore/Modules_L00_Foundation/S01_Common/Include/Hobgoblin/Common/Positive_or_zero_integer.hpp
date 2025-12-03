// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_COMMON_POSITIVE_OR_ZERO_INTEGER_HPP
#define UHOBGOBLIN_COMMON_POSITIVE_OR_ZERO_INTEGER_HPP

#include <cstddef>
#include <cstdint>
#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

//! Positive-or-Zero Integer (size not specified but most likely 32 bit)
using PZInteger = int;

//! std::size_t to PZInteger
constexpr PZInteger stopz(std::size_t value) {
    return static_cast<PZInteger>(value);
}

//! PZInteger to std::size_t
constexpr std::size_t pztos(PZInteger value) {
    return static_cast<std::size_t>(value);
}

//! T to PZInteger
template <class T>
constexpr PZInteger ToPz(T&& value) {
    return static_cast<PZInteger>(std::forward<T>(value));
}

//! T to std::size_t
template <class T>
constexpr std::size_t ToSz(T&& value) {
    return static_cast<std::size_t>(std::forward<T>(value));
}

//! Positive-or-Zero fixed-size 64-bit Integer
using PZInt64 = std::int64_t;

//! std::size_t to PZInt64
constexpr PZInt64 stopz64(std::size_t value) {
    return static_cast<PZInt64>(value);
}

//! PZInt64 to std::size_t
constexpr std::size_t pz64tos(PZInt64 value) {
    return static_cast<std::size_t>(value);
}

//! T to PZInt64
template <class T>
constexpr PZInt64 ToPz64(T&& value) {
    return static_cast<PZInt64>(std::forward<T>(value));
}

HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_COMMON_POSITIVE_OR_ZERO_INTEGER_HPP

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_UTIL_MAKE_VISITOR_HPP
#define UHOBGOBLIN_UTIL_MAKE_VISITOR_HPP

#include <type_traits>
#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

template <class ...Args>
struct Visitor : Args... {
    Visitor(Args&&... args)
        : Args{std::forward<Args>(args)}...
    {
    }

    using Args::operator()...;
};

template <class ...Args>
auto MakeVisitor(Args&&... fs) {
    return Visitor<Args...>(std::forward<Args>(fs)...);
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UTIL_MAKE_VISITOR_HPP

// clang-format on

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UTIL_ON_SCOPE_EXIT_HPP
#define UHOBGOBLIN_UTIL_ON_SCOPE_EXIT_HPP

#include <Hobgoblin/Preprocessor.hpp>

#include <functional> // IWYU pragma: keep

#define UHOBGOBLIN_ON_SCOPE_EXIT_DECL(_linenumber_)        \
    struct UHOBGOBLIN_OnScopeExitImpl_##_linenumber_ {     \
        std::function<void()> _callable;                   \
        ~UHOBGOBLIN_OnScopeExitImpl_##_linenumber_() {     \
            if (_callable) {                               \
                _callable();                               \
            }                                              \
        }                                                  \
    } UHOBGOBLIN_OnScopeExitImpl_instance_##_linenumber_

#define UHOBGOBLIN_ON_SCOPE_EXIT_IMPL(_linenumber_, _capture_, ...) \
    UHOBGOBLIN_ON_SCOPE_EXIT_DECL(_linenumber_) {                   \
        _capture_() {                                               \
            __VA_ARGS__;                                            \
        }                                                           \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures no variables from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP0(...)                                   \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {          \
        []() {                                                       \
            __VA_ARGS__;                                             \
        }                                                            \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 1 variable (_c1_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP1(_c1_, ...)                             \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {          \
        [_c1_]() {                                                   \
            __VA_ARGS__;                                             \
        }                                                            \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 2 variables (_c1_, _c2_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP2(_c1_, _c2_, ...)                       \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {          \
        [_c1_, _c2_]() {                                             \
            __VA_ARGS__;                                             \
        }                                                            \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 3 variables (_c1_ - _c3_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP3(_c1_, _c2_, _c3_, ...)                 \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {          \
        [_c1_, _c2_, _c3_]() {                                       \
            __VA_ARGS__;                                             \
        }                                                            \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 4 variables (_c1_ - _c4_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP4(_c1_, _c2_, _c3_, _c4_, ...)           \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {          \
        [_c1_, _c2_, _c3_, _c4_]() {                                 \
            __VA_ARGS__;                                             \
        }                                                            \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 5 variables (_c1_ - _c5_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP5(_c1_, _c2_, _c3_, _c4_, _c5_, ...)     \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {          \
        [_c1_, _c2_, _c3_, _c4_, _c5_]() {                           \
            __VA_ARGS__;                                             \
        }                                                            \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 6 variables (_c1_ - _c6_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP6(_c1_, _c2_, _c3_, _c4_, _c5_, _c6_, ...) \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {            \
        [_c1_, _c2_, _c3_, _c4_, _c5_, _c6_]() {                       \
            __VA_ARGS__;                                               \
        }                                                              \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 7 variables (_c1_ - _c7_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP7(_c1_, _c2_, _c3_, _c4_, _c5_, _c6_, _c7_, ...) \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {                  \
        [_c1_, _c2_, _c3_, _c4_, _c5_, _c6_, _c7_]() {                       \
            __VA_ARGS__;                                                     \
        }                                                                    \
    }

//! \brief Execute a piece of code when the current scope is exited.
//!
//! What this macro basically does is create an instance of unspecified type and name
//! in the place of its invocation. This instance will have automatic storage duration,
//! and, in its destructor, it will invoke the code provided as ... (__VA_ARGS__) to
//! this macro invocation.
//!
//! The _CAP0 variant captures 8 variables (_c1_ - _c8_) from the surrounding context.
#define HG_ON_SCOPE_EXIT_CAP8(_c1_, _c2_, _c3_, _c4_, _c5_, _c6_, _c7_, _c8_, ...) \
    HG_PP_INVOKE(UHOBGOBLIN_ON_SCOPE_EXIT_DECL, __LINE__) {                        \
        [_c1_, _c2_, _c3_, _c4_, _c5_, _c6_, _c7_, _c8_]() {                       \
            __VA_ARGS__;                                                           \
        }                                                                          \
    }

#endif // !UHOBGOBLIN_UTIL_ON_SCOPE_EXIT_HPP

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_EXECON_HPP
#define UHOBGOBLIN_QAO_EXECON_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

enum class QAO_ExeCon : signed char {
    ESSENTIAL       = 1,
    INTERACTIVITY   = 2,
    SYNCHRONIZATION = 3,
    GAMEPLAY        = 4,
    EXTRAS          = 5,

    META_EXECUTE_NONE = 0,
    META_EXECUTE_ALL  = 5,

    //! Minimum EXECON level of the runtime for the gameplay to be considered unpaused and running.
    META_MIN_FOR_GAMEPLAY_FLOW = 4
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_EXECON_HPP

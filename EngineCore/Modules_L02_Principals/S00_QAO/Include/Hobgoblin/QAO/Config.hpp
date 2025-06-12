// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_CONFIG_HPP
#define UHOBGOBLIN_QAO_CONFIG_HPP

#include <Hobgoblin/Common.hpp>

#include <cstdint>
#include <limits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

//! Index of an object (derived from QAO_Base) attached to a QAO_Runtime.
//! No two objects attached to the same runtime can have the same index at the same time.
using QAO_Index = PZInteger;

//! An invalid value for QAO_Index.
static constexpr QAO_Index  QAO_NULL_INDEX  = std::numeric_limits<QAO_Index>::max();

//! Serial number of an object (derived from QAO_Base) attached to a QAO_Runtime.
//! No two objects over the whole lifetime of the runtime can have the same serial number.
//! \note detaching an object from the runtime and reattaching it later *can* change its
//!       serial number depending on how it was reattached.
using QAO_Serial = std::uint64_t;

//! An invalid value for QAO_Serial.
static constexpr QAO_Serial QAO_NULL_SERIAL = std::numeric_limits<QAO_Serial>::max();

//! Serial number assigned to the first object attached to a new runtime.
static constexpr QAO_Serial QAO_STARTING_SERIAL  = 0;

struct QAO_Event {
    enum Enum {
        NONE = -1,

        PRE_UPDATE,
        BEGIN_UPDATE,
        UPDATE_1,
        UPDATE_2,
        END_UPDATE,
        POST_UPDATE,

        PRE_DRAW,
        DRAW_1,
        DRAW_2,
        DRAW_GUI,
        POST_DRAW,

        DISPLAY,

        EVENT_COUNT,
    };
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_CONFIG_HPP

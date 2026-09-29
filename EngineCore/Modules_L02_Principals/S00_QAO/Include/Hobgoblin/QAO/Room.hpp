// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_ROOM_HPP
#define UHOBGOBLIN_QAO_ROOM_HPP

#include <Hobgoblin/Common/Positive_or_zero_integer.hpp>

#include <cstdint>
#include <string>
#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

using QAO_RoomId = std::uint8_t;

constexpr QAO_RoomId QAO_INVALID_ROOM_ID = 0;

constexpr PZInteger QAO_MAX_ROOM_COUNT = 250;

struct QAO_Room {
    std::string name;
    QAO_RoomId  id;

    QAO_Room(std::string aName, QAO_RoomId aId)
        : name{std::move(aName)}
        , id(aId) {}
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_ROOM_HPP

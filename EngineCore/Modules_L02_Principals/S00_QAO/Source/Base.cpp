// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>
#include <Hobgoblin/Utility/Passkey.hpp>

#include <Hobgoblin/QAO/Functions.hpp>
#include <Hobgoblin/QAO/Handle.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

QAO_Base::QAO_Base(QAO_IKey, const std::type_info& typeInfo, int executionPriority, std::string name)
    : _instanceName{std::move(name)}
    , _typeInfo{typeInfo}
    , _execution_priority{executionPriority} {}

QAO_Base::~QAO_Base() = default;

QAO_Runtime* QAO_Base::getRuntime() const noexcept {
    return _context.runtime;
}

int QAO_Base::getExecutionPriority() const noexcept {
    return _execution_priority;
}

std::string QAO_Base::getName() const {
    return _instanceName;
}

QAO_GenericId QAO_Base::getId() const noexcept {
    return _context.id;
}

const std::type_info& QAO_Base::getTypeInfo() const {
    return _typeInfo;
}

bool QAO_Base::message(int tag, util::AnyPtr context) {
    return false;
}

void QAO_Base::setExecutionPriority(int new_priority) {
    if (_execution_priority == new_priority) {
        return;
    }
    if (_context.runtime != nullptr) {
        _context.runtime->updateExecutionPriorityForObject(SELF, new_priority);
    } else {
        _execution_priority = new_priority;
    }
}

void QAO_Base::setName(std::string newName) {
    _instanceName = std::move(newName);
}

// Private

void QAO_Base::_callEvent(QAO_Event::Enum ev) {
    // clang-format off
    using EventHandlerPointer = void(QAO_Base::*)();
    const EventHandlerPointer handlers[QAO_Event::EVENT_COUNT] = {
        &QAO_Base::_eventPreUpdate,
        &QAO_Base::_eventBeginUpdate,
        &QAO_Base::_eventUpdate1,
        &QAO_Base::_eventUpdate2,
        &QAO_Base::_eventEndUpdate,
        &QAO_Base::_eventPostUpdate,

        &QAO_Base::_eventPreDraw,
        &QAO_Base::_eventDraw1,
        &QAO_Base::_eventDraw2,
        &QAO_Base::_eventDrawGUI,
        &QAO_Base::_eventPostDraw,

        &QAO_Base::_eventDisplay
    };
    assert(ev >= 0 && ev < QAO_Event::EVENT_COUNT);
    (this->*handlers[ev])();
    // clang-format on
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// clang-format on

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>
#include <Hobgoblin/Utility/Passkey.hpp>

#include <Hobgoblin/QAO/Handle.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

static constexpr auto LOG_ID = "Hobgoblin.QAO";

QAO_Base::QAO_Base(QAO_IKey, const std::type_info& typeInfo, int executionPriority, std::string name)
    : _instanceName{std::move(name)}
    , _typeInfo{typeInfo}
    , _executionPriority{executionPriority} {}

QAO_Base::~QAO_Base() {
    if (HG_UNLIKELY_CONDITION((_flags & TORN_DOWN_PROPERLY) == 0)) {
        HG_UNLIKELY_BRANCH;

        HG_LOG_ERROR(LOG_ID,
                     "Object to destroy ('{}' of type '{}') wasn't torn down properly. Do all derived "
                     "classes call the "
                     "_tearDown() method of their superclasses?",
                     getName(),
                     getTypeInfo().name());

        assert(false && "Object to destroy wasn't torn down properly. Do all derived "
                        "classes call the "
                        "_tearDown() method of their superclasses?");
    }
}

void QAO_Base::_setUp() {
    _flags |= SET_UP_PROPERLY;
}

void QAO_Base::_tearDown() {
    _flags |= TORN_DOWN_PROPERLY;
}

void QAO_Base::_didAttach(QAO_Runtime&) {
    _flags |= ATTACHED_PROPERLY;
    _flags &= ~DETACHED_PROPERLY;
}

void QAO_Base::_willDetach(QAO_Runtime&) {
    _flags |= DETACHED_PROPERLY;
    _flags &= ~ATTACHED_PROPERLY;
}

QAO_Runtime* QAO_Base::getRuntime() const noexcept {
    return _context.runtime;
}

int QAO_Base::getExecutionPriority() const noexcept {
    return _executionPriority;
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
    if (_executionPriority == new_priority) {
        return;
    }
    if (_context.runtime != nullptr) {
        _context.runtime->updateExecutionPriorityForObject(SELF, new_priority);
    } else {
        _executionPriority = new_priority;
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

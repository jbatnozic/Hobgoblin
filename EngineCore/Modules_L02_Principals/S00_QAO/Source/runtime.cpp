// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>

#include <cassert>
#include <cstring>
#include <limits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

constexpr std::int64_t MIN_STEP_ORDINAL = std::numeric_limits<std::int64_t>::min(); // TODO to config.hpp

QAO_Runtime::QAO_Runtime()
    : QAO_Runtime{nullptr} {}

QAO_Runtime::QAO_Runtime(util::AnyPtr userData)
    : _step_counter{MIN_STEP_ORDINAL + 1}
    , _current_event{QAO_Event::NONE}
    , _step_orderer_iterator{_orderer.end()}
    , _user_data{userData} {}

QAO_Runtime::~QAO_Runtime() = default;

QAO_RuntimeRef QAO_Runtime::nonOwning() {
    QAO_RuntimeRef rv{this};
    rv._isOwning = false;
    return rv;
}

void QAO_Runtime::attachObject(AvoidNull<QAO_GenericHandle> aHandle) {
    HG_VALIDATE_PRECONDITION(aHandle->getRuntime() == nullptr);

    QAO_Base* const objRaw = aHandle.underlying().ptr();
    const auto      id     = _registry.insert(std::move(aHandle));

    auto ordererInsertionResult = _orderer.insert(objRaw);
    HG_HARD_ASSERT(ordererInsertionResult.second);

    objRaw->_context = {.stepOrdinal     = MIN_STEP_ORDINAL,
                        .id              = id,
                        .ordererIterator = ordererInsertionResult.first,
                        .runtime         = this};

    objRaw->_didAttach();
}

void QAO_Runtime::attachObject(AvoidNull<QAO_GenericHandle> aHandle, QAO_GenericId aSpecificId) {
    HG_VALIDATE_PRECONDITION(aHandle->getRuntime() == nullptr);

    QAO_Base* const objRaw = aHandle.underlying().ptr();
    _registry.insertWithId(std::move(aHandle), aSpecificId);

    auto ordererInsertionResult = _orderer.insert(objRaw);
    HG_HARD_ASSERT(ordererInsertionResult.second);

    objRaw->_context = {.stepOrdinal     = MIN_STEP_ORDINAL,
                        .id              = aSpecificId,
                        .ordererIterator = ordererInsertionResult.first,
                        .runtime         = this};

    objRaw->_didAttach();
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(QAO_GenericId aId) {
    auto handle = _registry.findObjectWithId(aId);
    HG_VALIDATE_PRECONDITION(!handle.isNull() &&
                             "Object by given ID must exist attached to the Runtime.");

    handle->_willDetach();

    const auto index = aId.getIndex();

    auto rv = _registry.remove(index);

    // If current _step_orderer_iterator points to released object, advance it first
    if (_step_orderer_iterator != _orderer.end() && *_step_orderer_iterator == handle.ptr()) {
        _step_orderer_iterator = std::next(_step_orderer_iterator);
    }
    _orderer.erase(handle.ptr());

    handle->_context = QAO_Base::Context{};

    return rv;
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(QAO_Base& aObject) {
    HG_VALIDATE_PRECONDITION(aObject.getRuntime() == this && "Object must be attached to this Runtime.");
    return detachObject(aObject.getId());
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(NeverNull<QAO_GenericHandle> aObject) {
    return detachObject(*aObject);
}

void QAO_Runtime::destroyAllOwnedObjects() {
    std::vector<QAO_GenericId> objectsToErase;
    for (auto& object : SELF) {
        const auto id = object->getId();
        if (ownsObject(id)) {
            objectsToErase.push_back(id);
        }
    }
    for (auto& id : objectsToErase) {
        detachObject(id);
    }
}

QAO_Base* QAO_Runtime::find(const std::string& name) const {
    for (auto iter = cbegin(); iter != cend(); iter = std::next(iter)) {
        if ((*iter)->getName() == name) {
            return *iter;
        }
    }
    return nullptr;
}

QAO_Base* QAO_Runtime::find(QAO_GenericId id) const {
    return _registry.findObjectWithId(id).ptr();
}

void QAO_Runtime::updateExecutionPriorityForObject(QAO_Base* object, int newPriority) {
    assert(object);
    assert(find(object->getId()) == object);

    _orderer.erase(object);
    object->_execution_priority = newPriority;

    const auto ord_pair              = _orderer.insert(object); // first = iterator, second = added_new
    object->_context.ordererIterator = ord_pair.first;
}

// Execution

void QAO_Runtime::startStep() {
    _current_event         = QAO_Event::PRE_UPDATE;
    _step_orderer_iterator = _orderer.begin();
}

void QAO_Runtime::advanceStep(bool& done, std::int32_t eventFlags) {
    done                      = false;
    QAO_OrdererIterator& curr = _step_orderer_iterator;

    //-----------------------------------------//
    for (std::int32_t i = _current_event; i < QAO_Event::EVENT_COUNT; i += 1) {
        if ((eventFlags & (1 << i)) == 0) {
            continue;
        }

        auto ev        = static_cast<QAO_Event::Enum>(i);
        _current_event = ev;

        while (curr != _orderer.end()) {
            QAO_Base* const instance = *curr;

            char currBeforeEvent[sizeof(curr)];
            std::memcpy(currBeforeEvent, &curr, sizeof(curr));

            if (instance->_context.stepOrdinal < _step_counter) {
                instance->_context.stepOrdinal = _step_counter;
                instance->_callEvent(ev);
                // After calling _callEvent, the instance variable must no longer be used until
                // reassigned, because an instance is allowed to delete itself inside of an event
                // implementation
            }
            // If the step orderer iterator wasn't advanced by the _callEvent invocation (by the callee
            // deleting itself), it must be advanced here. Raw memory compare is necessary because
            // the iterator being compared then becomes invalid when deleting the list node.
            const bool stepIteratorStayedTheSame =
                (std::memcmp(currBeforeEvent, &curr, sizeof(curr)) == 0);
            if (stepIteratorStayedTheSame && curr != _orderer.end()) {
                curr = std::next(curr);
            }
        }

        curr = _orderer.begin();
        _step_counter += 1;
    }
    //-----------------------------------------//

    _current_event = QAO_Event::NONE;
    done           = true;
}

QAO_Event::Enum QAO_Runtime::getCurrentEvent() const {
    return _current_event;
}

// Other

PZInteger QAO_Runtime::getObjectCount() const noexcept {
    return _registry.instanceCount();
}

bool QAO_Runtime::ownsObject(QAO_GenericId aId) const {
    return _registry.isObjectWithIndexOwned(aId.getIndex());
}

bool QAO_Runtime::ownsObject(QAO_Base& aObject) const {
    HG_VALIDATE_PRECONDITION(aObject.getRuntime() == this && "Object must be attached to this Runtime.");
    return ownsObject(aObject.getId());
}

bool QAO_Runtime::ownsObject(NeverNull<QAO_GenericHandle> aObject) const {
    return ownsObject(*aObject);
}

// User data

void QAO_Runtime::setUserData(std::nullptr_t) {
    _user_data.reset(nullptr);
}

// Orderer/instance iterations

QAO_OrdererIterator QAO_Runtime::begin() {
    return _orderer.begin();
}

QAO_OrdererIterator QAO_Runtime::end() {
    return _orderer.end();
}

QAO_OrdererReverseIterator QAO_Runtime::rbegin() {
    return _orderer.rbegin();
}

QAO_OrdererReverseIterator QAO_Runtime::rend() {
    return _orderer.rend();
}

QAO_OrdererConstIterator QAO_Runtime::cbegin() const {
    return _orderer.cbegin();
}

QAO_OrdererConstIterator QAO_Runtime::cend() const {
    return _orderer.cend();
}

QAO_OrdererConstReverseIterator QAO_Runtime::crbegin() const {
    return _orderer.crbegin();
}

QAO_OrdererConstReverseIterator QAO_Runtime::crend() const {
    return _orderer.crend();
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

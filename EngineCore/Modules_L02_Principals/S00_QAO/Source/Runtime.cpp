// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>

#include <cassert>
#include <cstring>
#include <exception>
#include <limits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

namespace {
constexpr auto         LOG_ID           = "Hobgoblin.QAO";
constexpr std::int64_t MIN_STEP_ORDINAL = std::numeric_limits<std::int64_t>::min(); // TODO to config.hpp
} // namespace

QAO_Runtime::QAO_Runtime()
    : QAO_Runtime{nullptr} {}

QAO_Runtime::QAO_Runtime(util::AnyPtr userData)
    : _step_counter{MIN_STEP_ORDINAL + 1}
    , _currentEvent{QAO_Event::NONE}
    , _step_orderer_iterator{_orderer.end()}
    , _userData{userData} {}

QAO_Runtime::~QAO_Runtime() {
    destroyAllOwnedObjects(NO_PROPAGATE_EXCEPTIONS);
}

QAO_RuntimeRef QAO_Runtime::nonOwning() {
    QAO_RuntimeRef rv{this};
    rv._isOwning = false;
    return rv;
}

void QAO_Runtime::attachObject(AvoidNull<QAO_GenericHandle> aHandle) {
    if (HG_UNLIKELY_CONDITION((aHandle->_flags & QAO_Base::SET_UP_PROPERLY) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ('{}' of type '{}') wasn't set up properly. Do all derived "
                        "classes call the "
                        "_setUp() method of their superclasses?",
                        aHandle->getName(),
                        aHandle->getTypeInfo().name());
    }

    HG_VALIDATE_PRECONDITION(aHandle->getRuntime() == nullptr);

    QAO_Base* const objRaw = aHandle.underlying().ptr();
    const auto      id     = _registry.insert(std::move(aHandle));

    auto ordererInsertionResult =
        _orderer.insert(qao_detail::QAO_HandleFactory::createHandle(objRaw, false));
    HG_HARD_ASSERT(ordererInsertionResult.second);

    objRaw->_context = {.stepOrdinal     = MIN_STEP_ORDINAL,
                        .id              = id,
                        .ordererIterator = ordererInsertionResult.first,
                        .runtime         = this};

    objRaw->_didAttach(SELF);

    if (HG_UNLIKELY_CONDITION((objRaw->_flags & QAO_Base::ATTACHED_PROPERLY) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ('{}' of type '{}') wasn't attached properly. Do all derived "
                        "classes call the "
                        "_didAttach() method of their superclasses?",
                        objRaw->getName(),
                        objRaw->getTypeInfo().name());
    }
}

void QAO_Runtime::attachObject(AvoidNull<QAO_GenericHandle> aHandle, QAO_GenericId aSpecificId) {
    if (HG_UNLIKELY_CONDITION((aHandle->_flags & QAO_Base::SET_UP_PROPERLY) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ('{}' of type '{}') wasn't set up properly. Do all derived "
                        "classes call the "
                        "_setUp() method of their superclasses?",
                        aHandle->getName(),
                        aHandle->getTypeInfo().name());
    }

    HG_VALIDATE_PRECONDITION(aHandle->getRuntime() == nullptr);

    QAO_Base* const objRaw = aHandle.underlying().ptr();
    _registry.insertWithId(std::move(aHandle), aSpecificId);

    auto ordererInsertionResult =
        _orderer.insert(qao_detail::QAO_HandleFactory::createHandle(objRaw, false));
    HG_HARD_ASSERT(ordererInsertionResult.second);

    objRaw->_context = {.stepOrdinal     = MIN_STEP_ORDINAL,
                        .id              = aSpecificId,
                        .ordererIterator = ordererInsertionResult.first,
                        .runtime         = this};

    objRaw->_didAttach(SELF);

    if (HG_UNLIKELY_CONDITION((aHandle->_flags & QAO_Base::ATTACHED_PROPERLY) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ('{}' of type '{}') wasn't attached properly. Do all derived "
                        "classes call the "
                        "_didAttach() method of their superclasses?",
                        aHandle->getName(),
                        aHandle->getTypeInfo().name());
    }
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(QAO_GenericId aId) {
    auto handle = _registry.findObjectWithId(aId);
    HG_VALIDATE_PRECONDITION(!handle.isNull() &&
                             "Object by given ID must exist attached to the Runtime.");

    handle->_willDetach(SELF);
    if (HG_UNLIKELY_CONDITION((handle->_flags & QAO_Base::DETACHED_PROPERLY) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to detach ('{}' of type '{}') wasn't detached properly. Do all derived "
                        "classes call the "
                        "_willDetach() method of their superclasses?",
                        handle->getName(),
                        handle->getTypeInfo().name());
    }
    handle->_context = QAO_Base::Context{};

    const auto index = aId.getIndex();

    auto rv = _registry.remove(index);

    // If current _step_orderer_iterator points to released object, advance it first
    if (_step_orderer_iterator != _orderer.end() && _step_orderer_iterator->ptr() == handle.ptr()) {
        _step_orderer_iterator = std::next(_step_orderer_iterator);
    }
    _orderer.erase(qao_detail::QAO_HandleFactory::createHandle(handle.ptr(), false));

    return rv;
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(QAO_Base& aObject) {
    HG_VALIDATE_PRECONDITION(aObject.getRuntime() == this && "Object must be attached to this Runtime.");
    return detachObject(aObject.getId());
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(NeverNull<QAO_GenericHandle> aObject) {
    return detachObject(*aObject);
}

void QAO_Runtime::destroyAllOwnedObjects(bool aPropagateExceptions) {
    std::vector<QAO_GenericId> objectsToErase;
    for (auto& object : SELF) {
        const auto id = object->getId();
        if (ownsObject(id)) {
            objectsToErase.push_back(id);
        }
    }
    for (auto& id : objectsToErase) {
        std::string objectInfo = "?";
        try {
            auto handle = MoveToUnderlying(detachObject(id));
            objectInfo  = fmt::format(FMT_STRING("'{}' of type '{}'"),
                                     handle->getName(),
                                     handle->getTypeInfo().name());
            handle.reset();
        } catch (const TracedException& ex) {
            HG_LOG_ERROR(LOG_ID,
                         "destroyAllOwnedObjects - Encountered an error while detaching and/or "
                         "destroying object ({}). Details: {}",
                         objectInfo,
                         ex.getFormattedDescription());
            if (aPropagateExceptions) {
                throw;
            }
        } catch (const std::exception& ex) {
            HG_LOG_ERROR(LOG_ID,
                         "destroyAllOwnedObjects - Encountered an error while detaching and/or "
                         "destroying object ({}). Details: {}",
                         objectInfo,
                         ex.what());
            if (aPropagateExceptions) {
                throw;
            }
        } catch (...) {
            HG_LOG_ERROR(LOG_ID,
                         "destroyAllOwnedObjects - Encountered an error while detaching and/or "
                         "destroying object ({}). Details: n/a",
                         objectInfo);
            if (aPropagateExceptions) {
                throw;
            }
        }
    }
}

void QAO_Runtime::updateExecutionPriorityForObject(QAO_Base& object, int newPriority) {
    assert(find(object.getId()).ptr() == &object);

    _orderer.erase(qao_detail::QAO_HandleFactory::createHandle(&object, false));
    object._executionPriority = newPriority;

    const auto ord_pair = _orderer.insert(
        qao_detail::QAO_HandleFactory::createHandle(&object,
                                                    false)); // first = iterator, second = added_new
    object._context.ordererIterator = ord_pair.first;
}

// Execution

void QAO_Runtime::startStep() {
    _currentEvent          = QAO_Event::PRE_UPDATE;
    _step_orderer_iterator = _orderer.begin();
}

void QAO_Runtime::advanceStep(bool& done, std::int32_t eventFlags) {
    done                      = false;
    QAO_OrdererIterator& curr = _step_orderer_iterator;

    //-----------------------------------------//
    for (std::int32_t i = _currentEvent; i < QAO_Event::EVENT_COUNT; i += 1) {
        if ((eventFlags & (1 << i)) == 0) {
            continue;
        }

        auto ev       = static_cast<QAO_Event::Enum>(i);
        _currentEvent = ev;

        while (curr != _orderer.end()) {
            auto* const instance = curr->ptr();

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

    _currentEvent = QAO_Event::NONE;
    done          = true;
}

QAO_Event::Enum QAO_Runtime::getCurrentEvent() const {
    return _currentEvent;
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
    _userData.reset(nullptr);
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

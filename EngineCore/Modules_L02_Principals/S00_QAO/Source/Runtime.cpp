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
constexpr PZInteger    MAX_ATTEMPTS     = 10;
} // namespace

QAO_Runtime::QAO_Runtime()
    : QAO_Runtime{nullptr, nullptr} {}

QAO_Runtime::QAO_Runtime(util::AnyPtr aUserData)
    : QAO_Runtime{aUserData, nullptr} {}

QAO_Runtime::QAO_Runtime(const QAO_ExeCon* aExeconAddress)
    : QAO_Runtime{nullptr, aExeconAddress} {}

QAO_Runtime::QAO_Runtime(util::AnyPtr aUserData, const QAO_ExeCon* aExeconAddress)
    : _step_counter{MIN_STEP_ORDINAL + 1}
    , _currentEvent{QAO_Event::NONE}
    , _step_orderer_iterator{_orderer.end()}
    , _userData{aUserData}
    , _execon{aExeconAddress} //
{
    pushRoom("default");
}

QAO_Runtime::~QAO_Runtime() {
    popAllRooms(NO_PROPAGATE_EXCEPTIONS);

    PZInteger attempts = 0;
    while (true) {
        ++attempts;

        PZInteger                  ownedCount = 0;
        std::vector<QAO_GenericId> objectsToDetach;
        for (auto& object : SELF) {
            const auto id = object->getId();
            if (ownsObject(id)) {
                ownedCount += 1;
            }
            objectsToDetach.push_back(id);
        }
        if (objectsToDetach.empty()) {
            break;
        }
        if (attempts > MAX_ATTEMPTS) {
            HG_LOG_ERROR(LOG_ID,
                         "QAO_Runtime still has {} objects attached after {} attempts to forcibly "
                         "detach them! Bailing...",
                         objectsToDetach.size(),
                         MAX_ATTEMPTS);
            break;
        }

        HG_LOG_WARN(LOG_ID,
                    "QAO_Runtime is being destroyed with {} owned and {} non-owned objects still "
                    "attached; will forcibly detach all of them (attempt {}/{}).",
                    ownedCount,
                    stopz(objectsToDetach.size()) - ownedCount,
                    attempts,
                    MAX_ATTEMPTS);

        for (const auto& id : objectsToDetach) {
            const auto current = find(id);
            if (current.isNull()) {
                continue; // Could have been destroyed by another object's detaching or destruction
            }
            try {
                MoveToUnderlying(detachObject(id)).reset();
            } catch (const TracedException& ex) {
                HG_LOG_ERROR(LOG_ID,
                             "Encountered an error while forcibly detaching object. Details: {}",
                             ex.getFormattedDescription());
            } catch (const std::exception& ex) {
                HG_LOG_ERROR(LOG_ID,
                             "Encountered an error while forcibly detaching object. Details: {}",
                             ex.what());
            } catch (...) {
                HG_LOG_ERROR(LOG_ID,
                             "Encountered an error while forcibly detaching object. Details: n/a");
            }
        }
    }
}

QAO_RuntimeRef QAO_Runtime::nonOwning() {
    QAO_RuntimeRef rv{this};
    rv._isOwning = false;
    return rv;
}

void QAO_Runtime::attachObject(AvoidNull<QAO_GenericHandle> aHandle) {
    if (HG_UNLIKELY_CONDITION((aHandle->_flags & QAO_Base::SET_UP_PROPERLY_BIT) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ({}) wasn't set up properly. Do all derived "
                        "classes call the "
                        "_setUp() method of their superclasses?",
                        aHandle->getDebugDescription());
    }

    HG_VALIDATE_PRECONDITION(aHandle->getRuntime() == nullptr);
    // Usually we can't insert objects when there is no active room, but for objects that
    // won't be owned by the runtime this doesn't matter.
    HG_VALIDATE_PRECONDITION(getRoomCount() > 0 || !aHandle.underlying().isOwning());

    const bool      ownedByRuntime = aHandle.underlying().isOwning();
    QAO_Base* const objRaw         = aHandle.underlying().ptr();
    const auto      id             = _registry.insert(std::move(aHandle));

    auto ordererInsertionResult =
        _orderer.insert(qao_detail::QAO_HandleFactory::createHandle(objRaw, false));
    HG_HARD_ASSERT(ordererInsertionResult.second);

    const auto roomCount = getRoomCount();
    objRaw->_context     = {.stepOrdinal     = MIN_STEP_ORDINAL,
                            .id              = id,
                            .ordererIterator = ordererInsertionResult.first,
                            .runtime         = this,
                            .roomId          = (roomCount == 0 || !ownedByRuntime)
                                                   ? QAO_INVALID_ROOM_ID
                                                   : static_cast<QAO_RoomId>(roomCount)};

    objRaw->_didAttach(SELF);

    if (HG_UNLIKELY_CONDITION((objRaw->_flags & QAO_Base::ATTACHED_PROPERLY_BIT) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ({}) wasn't attached properly. Do all derived "
                        "classes call the "
                        "_didAttach() method of their superclasses?",
                        objRaw->getDebugDescription());
    }
}

void QAO_Runtime::attachObject(AvoidNull<QAO_GenericHandle> aHandle, QAO_GenericId aSpecificId) {
    if (HG_UNLIKELY_CONDITION((aHandle->_flags & QAO_Base::SET_UP_PROPERLY_BIT) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ({}) wasn't set up properly. Do all derived "
                        "classes call the "
                        "_setUp() method of their superclasses?",
                        aHandle->getDebugDescription());
    }

    HG_VALIDATE_PRECONDITION(aHandle->getRuntime() == nullptr);
    // Usually we can't insert objects when there is no active room, but for objects that
    // won't be owned by the runtime this doesn't matter.
    HG_VALIDATE_PRECONDITION(getRoomCount() > 0 || !aHandle.underlying().isOwning());

    const bool      ownedByRuntime = aHandle.underlying().isOwning();
    QAO_Base* const objRaw         = aHandle.underlying().ptr();
    _registry.insertWithId(std::move(aHandle), aSpecificId);

    auto ordererInsertionResult =
        _orderer.insert(qao_detail::QAO_HandleFactory::createHandle(objRaw, false));
    HG_HARD_ASSERT(ordererInsertionResult.second);

    const auto roomCount = getRoomCount();
    objRaw->_context     = {.stepOrdinal     = MIN_STEP_ORDINAL,
                            .id              = aSpecificId,
                            .ordererIterator = ordererInsertionResult.first,
                            .runtime         = this,
                            .roomId          = (roomCount == 0 || !ownedByRuntime)
                                                   ? QAO_INVALID_ROOM_ID
                                                   : static_cast<QAO_RoomId>(roomCount)};

    objRaw->_didAttach(SELF);

    if (HG_UNLIKELY_CONDITION((objRaw->_flags & QAO_Base::ATTACHED_PROPERLY_BIT) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to attach ({}) wasn't attached properly. Do all derived "
                        "classes call the "
                        "_didAttach() method of their superclasses?",
                        aHandle->getDebugDescription());
    }
}

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(QAO_GenericId aId) {
    auto handle = _registry.findObjectWithId(aId);
    HG_VALIDATE_PRECONDITION(!handle.isNull() &&
                             "Object by given ID must exist attached to the Runtime.");

    handle->_willDetach(SELF);
    if (HG_UNLIKELY_CONDITION((handle->_flags & QAO_Base::DETACHED_PROPERLY_BIT) == 0)) {
        HG_UNLIKELY_BRANCH;
        HG_THROW_TRACED(AssertionFailedError,
                        0,
                        "Object to detach ({}) wasn't detached properly. Do all derived "
                        "classes call the "
                        "_willDetach() method of their superclasses?",
                        handle->getDebugDescription());
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
    HG_NOT_IMPLEMENTED();
}

const QAO_Room& QAO_Runtime::pushRoom(std::string aRoomName) {
    const auto roomCount = getRoomCount();
    HG_VALIDATE_PRECONDITION(roomCount < QAO_MAX_ROOM_COUNT);

    _roomStack.emplace_back(std::move(aRoomName), static_cast<QAO_RoomId>(roomCount + 1));
    return _roomStack.back();
}

void QAO_Runtime::popRoom(bool aPropagateExceptions) {
    // TODO: guard against recursive popRoom() calls

    HG_VALIDATE_PRECONDITION(getRoomCount() > 0);

    {
        const auto* topRoom = getTopRoom();
        HG_LOG_ERROR(LOG_ID, "Popping room '{}'...", topRoom->name);
    }

    PZInteger attempts = 0;
    while (true) {
        ++attempts;

        const auto* topRoom = getTopRoom();

        std::vector<QAO_GenericId> objectsToErase;
        for (auto& object : SELF) {
            const auto id = object->getId();
            if (object->getRoomId() == topRoom->id && ownsObject(id)) {
                objectsToErase.push_back(id);
            }
        }
        if (objectsToErase.empty()) {
            if (attempts > 1) {
                HG_LOG_ERROR(LOG_ID, "Room '{}' is now empty.", topRoom->name);
            }
            break;
        }

        if (attempts <= MAX_ATTEMPTS) {
            HG_LOG_ERROR(LOG_ID,
                         "Clearing out room '{}' (attempt {}/{}).",
                         topRoom->name,
                         attempts,
                         MAX_ATTEMPTS);
        } else {
            HG_LOG_ERROR(LOG_ID,
                         "Room '{}' still not cleared after {} attempts! Bailing...",
                         topRoom->name,
                         MAX_ATTEMPTS);
            break;
        }

        for (auto& id : objectsToErase) {
            if (find(id).isNull()) {
                continue; // Could have been destroyed by another object's detaching or destruction
            }

            std::string objectInfo = "?";
            try {
                auto handle = MoveToUnderlying(detachObject(id));
                objectInfo  = handle->getDebugDescription();
                handle.reset();
            } catch (const TracedException& ex) {
                HG_LOG_ERROR(LOG_ID,
                             "Encountered an error while detaching and/or "
                             "destroying object ({}). Details: {}",
                             objectInfo,
                             ex.getFormattedDescription());
                if (aPropagateExceptions) {
                    throw;
                }
            } catch (const std::exception& ex) {
                HG_LOG_ERROR(LOG_ID,
                             "Encountered an error while detaching and/or "
                             "destroying object ({}). Details: {}",
                             objectInfo,
                             ex.what());
                if (aPropagateExceptions) {
                    throw;
                }
            } catch (...) {
                HG_LOG_ERROR(LOG_ID,
                             "Encountered an error while detaching and/or "
                             "destroying object ({}). Details: n/a",
                             objectInfo);
                if (aPropagateExceptions) {
                    throw;
                }
            }
        }
    }

    _roomStack.pop_back();
}

void QAO_Runtime::popAllRooms(bool aPropagateExceptions) {
    while (!_roomStack.empty()) {
        popRoom(aPropagateExceptions);
    }
}

const QAO_Room* QAO_Runtime::getTopRoom() const {
    return _roomStack.empty() ? nullptr : &(_roomStack.back());
}

PZInteger QAO_Runtime::getRoomCount() const {
    return stopz(_roomStack.size());
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

            if ((!_execon || (*_execon >= instance->getExeconThreshold())) &&
                instance->_context.stepOrdinal < _step_counter) //
            {
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

// Execon

void QAO_Runtime::setExeconAddress(const QAO_ExeCon* aExeconAddress) {
    _execon = aExeconAddress;
}

const QAO_ExeCon* QAO_Runtime::getExeconAddress() const {
    return _execon;
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

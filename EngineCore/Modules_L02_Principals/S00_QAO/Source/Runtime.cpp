// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>
#include <Hobgoblin/Utility/Swallow.hpp>

#include <cassert>
#include <exception>
#include <limits>
#include <string>

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
            util::Swallow([&, this]() {
                MoveToUnderlying(detachObject(id)).reset();
            });
        }
    }
}

QAO_RuntimeRef QAO_Runtime::nonOwning() {
    QAO_RuntimeRef rv{this};
    rv._isOwning = false;
    return rv;
}

// MARK: Attaching objects

template <class taCallable>
void QAO_Runtime::_attachObjectImpl(QAO_GenericHandle&& aHandle,
                                    taCallable&&        aInsertIntoRegistryAndReturnId) {
    HG_VALIDATE_ARGUMENT(!aHandle.isNull());

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
    HG_VALIDATE_PRECONDITION(getRoomCount() > 0 || !aHandle.isOwning());

    const bool      ownedByRuntime = aHandle.isOwning();
    QAO_Base* const objRaw         = aHandle.ptr();

    auto [iter, didInsert] = _orderer.insert(qao_detail::QAO_HandleFactory::createHandle(objRaw, false));
    // If insertion into the orderer fails, no container will have been changed (including
    // `aHandle`), so in that case it's safe to propagate the failure upwards as-is.
    HG_HARD_ASSERT(didInsert);

    QAO_GenericId id;
    try {
        // If insertion into the registry fails, the registry itself will remain unchanged, as
        // well as `aHandle`, but we will already have modified the orderer, so we need to
        // clean it up before propagating the failure upwards.
        id = aInsertIntoRegistryAndReturnId(std::move(aHandle));
    } catch (...) {
        _orderer.erase(iter);
        throw;
    }

    const auto roomCount = getRoomCount();
    objRaw->_context     = {.stepOrdinal     = MIN_STEP_ORDINAL,
                            .id              = id,
                            .ordererIterator = iter,
                            .runtime         = this,
                            .roomId          = (roomCount == 0 || !ownedByRuntime)
                                                   ? QAO_INVALID_ROOM_ID
                                                   : static_cast<QAO_RoomId>(roomCount)};

    try {
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
    } catch (...) {
        // Here is the tricky part: if the final step of attaching fails,
        // we must restore all of the following (in the specified order):
        // - the object's context
        // - `aHandle` and the registry
        // - the orderer
        // And then propagate the failure up to the caller to deal with it.

        // `_didAttach()` could have invalidated `iter` by changing instance priority
        const auto ordererIter = objRaw->_context.ordererIterator;

        if (_step_orderer_iterator == ordererIter) {
            ++_step_orderer_iterator;
        }

        objRaw->_context = {};
        aHandle          = _registry.remove(id.getIndex());
        _orderer.erase(ordererIter);

        throw;
    }
}

void QAO_Runtime::attachObject(QAO_GenericHandle&& aHandle) {
    _attachObjectImpl(std::move(aHandle), [this](QAO_GenericHandle&& aHandle_) -> QAO_GenericId {
        return _registry.insert(std::move(aHandle_));
    });
}

void QAO_Runtime::attachObject(QAO_GenericHandle&& aHandle, QAO_GenericId aSpecificId) {
    _attachObjectImpl(std::move(aHandle),
                      [this, aSpecificId](QAO_GenericHandle&& aHandle_) -> QAO_GenericId {
                          _registry.insertWithId(std::move(aHandle_), aSpecificId);
                          return aSpecificId;
                      });
}

// MARK: Detaching objects

AvoidNull<QAO_GenericHandle> QAO_Runtime::detachObject(QAO_GenericId aId) {
    auto handle = _registry.findObjectWithId(aId);
    HG_VALIDATE_PRECONDITION(!handle.isNull() &&
                             "Object by given ID must exist attached to the Runtime.");

    // In general, detaching an object should do minimal work outside of nulling out some fields
    // or unsubscribing the object from runtime-bound components (and similar). So, right from the
    // get-go, we don't really expect that throwing due to improper detaching is a real factor or
    // something that's likely to happen. But, more importantly than that, detaching objects
    // either happens during runtime destruction, or, an exception escaping from here is likely to
    // lead to runtime destruction (during stack unwinding) or at least to some rooms being popped.
    // Either way, the most reasonable thing to do here is to swallow any exceptions and remove the
    // object from the runtime anyway, in whatever state it may be.
    util::Swallow([this, &handle]() {
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
    });

    // If current _step_orderer_iterator points to the object being detached, advance it first
    if (_step_orderer_iterator != _orderer.end() && _step_orderer_iterator->ptr() == handle.ptr()) {
        ++_step_orderer_iterator;
    }

    // We know that `remove()` will not throw because of the precondition check at the start
    auto rv = _registry.remove(aId.getIndex());

    // We would have to have some truly extraordinary circumstances for this to fail...
    if (auto iter = _orderer.find(qao_detail::QAO_HandleFactory::createHandle(handle.ptr(), false));
        iter != _orderer.end()) //
    {
        _orderer.erase(iter);
    } else {
        HG_LOG_ERROR(LOG_ID,
                     "Object to detach ({}) was not found in the orderer at all.",
                     rv->getDebugDescription());
    }

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

void QAO_Runtime::destroyAllOwnedObjects(bool aPropagateExceptions) {
    HG_NOT_IMPLEMENTED();
}

// MARK: Room management

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

// MARK: Execution

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

        _currentEvent = static_cast<QAO_Event::Enum>(i);

        while (true) {
            // Try to find the first instance whose event needs to be executed next.
            // If no instance is found, we're done with this event.
            while (true) {
                if (curr == _orderer.end()) {
                    goto EVENT_FINISHED;
                }
                if ((*curr)->_context.stepOrdinal >= _step_counter) {
                    ++curr;
                } else {
                    break;
                }
            }

            QAO_Base* instance = curr->ptr();

            if (!_execon || (*_execon >= instance->getExeconThreshold())) {
                instance->_context.stepOrdinal = _step_counter;
                instance->_callEvent(_currentEvent);
                //! \warning After calling _callEvent, the instance variable must no longer be used 
                //!          until reassigned, because an instance is allowed to delete itself inside 
                //!          of an event implementation
                instance = nullptr;
            } else {
                ++curr;
            }
        }

    EVENT_FINISHED:
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

// MARK: Inspection

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

// MARK: User data management

void QAO_Runtime::setUserData(std::nullptr_t) {
    _userData.reset(nullptr);
}

// MARK: EXECON management

void QAO_Runtime::setExeconAddress(const QAO_ExeCon* aExeconAddress) {
    _execon = aExeconAddress;
}

const QAO_ExeCon* QAO_Runtime::getExeconAddress() const {
    return _execon;
}

// MARK: Iterating

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

// MARK: Private methods

void QAO_Runtime::_updateExecutionPriorityForObject(QAO_Base& aObject, int aNewPriority) {
    HG_ASSERT(aObject._context.runtime == this);

    if (_step_orderer_iterator != _orderer.end() && _step_orderer_iterator->ptr() == &aObject) {
        ++_step_orderer_iterator;
    }

    auto nh = _orderer.extract(aObject._context.ordererIterator);
    HG_ASSERT(&aObject == nh.value().ptr());

    aObject._executionPriority = aNewPriority;

    aObject._context.ordererIterator = _orderer.insert(std::move(nh)).position;
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

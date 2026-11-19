// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_RUNTIME_HPP
#define UHOBGOBLIN_QAO_RUNTIME_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Execon.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Orderer.hpp>
#include <Hobgoblin/QAO/Registry.hpp>
#include <Hobgoblin/QAO/Room.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>
#include <Hobgoblin/Utility/Any_ptr.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

constexpr std::int32_t QAO_ALL_EVENT_FLAGS = 0xFFFFFFFF;

class QAO_Base;

class QAO_Runtime
    : NO_COPY
    , NO_MOVE {
public:
    QAO_Runtime();
    QAO_Runtime(util::AnyPtr aUserData);
    QAO_Runtime(const QAO_ExeCon* aExeconAddress);
    QAO_Runtime(util::AnyPtr aUserData, const QAO_ExeCon* aExeconAddress);
    ~QAO_Runtime();

    //! Create a non-owning reference to this runtime.
    QAO_RuntimeRef nonOwning();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: ATTACHING OBJECTS                                               //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Attach an object to this Runtime.
    //!
    //! \param aHandle rvalue reference to the (non-null) handle pointing to the object to detach. If
    //!                the handle is owning, ownership of the object will be transferred over to the
    //!                runtime. Otherwise, whoever is owning the object will keep owning it. If this
    //!                method call is successful, the passed handle will be moved-from.
    //!
    //! \throws InvalidArgumentError if `aHandle` is null.
    //! \throws PreconditionNotMetError if the object to attach is already attached to (any) runtime, or
    //!                                 if you want to pass the ownership of the object onto the runtime,
    //!                                 but the current room count is 0.
    //! \throws ? (propagates the exception) if the object's `_didAttach()` call throws.
    //!
    //! This method provides the strong exception guarantee: If an exception is thrown from it, the
    //! Runtime will be left in a valid state, without the passed object attached or partially attached,
    //! and also `aHandle` (or rather the original referenced object) will not have been moved-from or
    //! modified in any way.
    void attachObject(QAO_GenericHandle&& aHandle);

    //! \brief Attach helper for typed handles.
    //! \see `attachObject(QAO_GenericHandle&&)`
    template <class T>
    void attachObject(QAO_Handle<T>&& aHandle) {
        QAO_GenericHandle generic{std::move(aHandle)};
        try {
            attachObject(std::move(generic));
        } catch (...) {
            aHandle = generic.downcastMove<T>();
            throw;
        }
    }

    //! Similar to `attachObject(QAO_GenericHandle&&)`, but inserts with caller-specified ID
    //! (for deserialization purposes) and throws if that ID's index or serial is already taken.
    void attachObject(QAO_GenericHandle&& aHandle, QAO_GenericId aSpecificId);

    //! \brief Attach helper for typed handles.
    //! \see `attachObject(QAO_GenericHandle&&, QAO_GenericId)`
    template <class T>
    void attachObject(QAO_Handle<T>&& aHandle, QAO_GenericId aSpecificId) {
        QAO_GenericHandle generic{std::move(aHandle)};
        try {
            attachObject(std::move(generic), aSpecificId);
        } catch (...) {
            aHandle = generic.downcastMove<T>();
            throw;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: DETACHING OBJECTS                                               //
    ///////////////////////////////////////////////////////////////////////////

    //! Detach an object from the runtime.
    //! - If the object was owned by the runtime, an owning handle will be returned.
    //! - Otherwise, a non-owning handle will be returned.
    //! \throws PreconditionNotMetError if the object is not attached to this runtime.
    AvoidNull<QAO_GenericHandle> detachObject(QAO_GenericId aId);
    AvoidNull<QAO_GenericHandle> detachObject(QAO_Base& aObject);
    AvoidNull<QAO_GenericHandle> detachObject(NeverNull<QAO_GenericHandle> aObject);

    static constexpr bool PROPAGATE_EXCEPTIONS    = true;
    static constexpr bool NO_PROPAGATE_EXCEPTIONS = false;

    //! \warning NOT YET IMPLEMENTED
    void destroyAllOwnedObjects(bool aPropagateExceptions = PROPAGATE_EXCEPTIONS);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: ROOM MANAGEMENT                                                 //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Push a new room onto the room stack.
    //!
    //! While this room is the top room of the room stack, all objects added to the runtime will
    //! be added to this room, and will be destroyed once the room is popped (note: this only applies
    //! to objects owned by the runtime).
    //!
    //! \param aRoomName name for the new room.
    //!
    //! \returns reference to the newly pushed room object.
    //!
    //! \throws if the room count would exceed `QAO_MAX_ROOM_COUNT`.
    //!
    //! \note even if you push no rooms, by creating the runtime, an initial room with the name
    //!       "default" will be pushed.
    const QAO_Room& pushRoom(std::string aRoomName);

    //! \brief Pop a room from the room stack.
    //!
    //! This action will destroy all objects added to this room (note: this only applies
    //! to objects owned by the runtime; see `pushRoom`).
    //!
    //! \param aPropagateExceptions whether to propagate exceptions produced by destroying owned objects
    //!                             (true) or swallow them (false).
    //!
    //! \throws if there are no more rooms left to pop.
    //!
    //! \warning if you pop all the rooms, you won't be able to add runtime-owned objects to the runtime
    //!          until you push a new room!
    void popRoom(bool aPropagateExceptions = PROPAGATE_EXCEPTIONS);

    //! \brief Pops all rooms from the room stack (LIFO ordering).
    void popAllRooms(bool aPropagateExceptions = PROPAGATE_EXCEPTIONS);

    //! \brief Get the top room of the room stack.
    //!
    //! \returns pointer to the top room of the room stack, or `nullptr` if all rooms have been popped.
    const QAO_Room* getTopRoom() const;

    //! \brief Get the size of the room stack.
    PZInteger getRoomCount() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: EXECUTION                                                       //
    ///////////////////////////////////////////////////////////////////////////

    void            startStep();
    void            advanceStep(bool& done, std::int32_t eventFlags = QAO_ALL_EVENT_FLAGS);
    QAO_Event::Enum getCurrentEvent() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: INSPECTION                                                      //
    ///////////////////////////////////////////////////////////////////////////

    template <class T = QAO_Base>
    QAO_Handle<T> find(const std::string& name) const;

    template <class T = QAO_Base>
    QAO_Handle<T> find(QAO_GenericId id) const;

    template <class T>
    T* find(QAO_Id<T> id) const;

    PZInteger getObjectCount() const noexcept;
    bool      ownsObject(QAO_GenericId aId) const;
    bool      ownsObject(QAO_Base& aObject) const;
    bool      ownsObject(NeverNull<QAO_GenericHandle> aObject) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: USERDATA MANAGEMENT                                             //
    ///////////////////////////////////////////////////////////////////////////

    void setUserData(std::nullptr_t);

    template <class T>
    void setUserData(T* value);

    template <class T>
    T* getUserData() const;

    template <class T>
    T* getUserDataOrThrow() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: EXECON MANAGEMENT                                               //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief set the address from which the runtime will read the current execon level
    //! If set to null, the runtime will always assume `QAO_ExeCon::META_EXECUTE_ALL`.
    void setExeconAddress(const QAO_ExeCon* aExeconAddress);

    //! \brief return the set execon address.
    const QAO_ExeCon* getExeconAddress() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: ORDERER ITERATIONS                                              //
    ///////////////////////////////////////////////////////////////////////////

    QAO_OrdererIterator begin();
    QAO_OrdererIterator end();

    QAO_OrdererReverseIterator rbegin();
    QAO_OrdererReverseIterator rend();

    QAO_OrdererConstIterator cbegin() const;
    QAO_OrdererConstIterator cend() const;

    QAO_OrdererConstReverseIterator crbegin() const;
    QAO_OrdererConstReverseIterator crend() const;

private:
    friend class QAO_Base;

    qao_detail::QAO_Registry _registry;
    qao_detail::QAO_Orderer  _orderer;
    std::vector<QAO_Room>    _roomStack;
    std::int64_t             _step_counter;
    QAO_Event::Enum          _currentEvent;
    QAO_OrdererIterator      _step_orderer_iterator;
    util::AnyPtr             _userData;
    const QAO_ExeCon*        _execon;

    template <class taCallable>
    void _attachObjectImpl(QAO_GenericHandle&& aHandle, taCallable&& aInsertIntoRegistryAndReturnId);

    void _updateExecutionPriorityForObject(QAO_Base& aObject, int aNewPriority);
};

template <class T>
QAO_Handle<T> QAO_Runtime::find(const std::string& name) const {
    for (auto iter = cbegin(); iter != cend(); iter = std::next(iter)) {
        if ((*iter)->getName() != name) {
            continue;
        }
        if constexpr (!std::is_same_v<T, QAO_Base>) {
            if (dynamic_cast<T*>(iter->ptr()) == nullptr) {
                continue;
            }
        }
        return iter->downcastCopy<T>();
    }
    return {};
}

template <class T>
QAO_Handle<T> QAO_Runtime::find(QAO_GenericId id) const {
    auto handle = _registry.findObjectWithId(id);
    if constexpr (!std::is_same_v<T, QAO_Base>) {
        if (dynamic_cast<T*>(handle.ptr()) == nullptr) {
            return {};
        }
    }
    return handle.downcastCopy<T>();
}

template <class T>
T* QAO_Runtime::find(QAO_Id<T> id) const {
    return static_cast<T*>(find(QAO_GenericId{id}));
}

template <class T>
void QAO_Runtime::setUserData(T* value) {
    _userData.reset(value);
}

template <class T>
T* QAO_Runtime::getUserData() const {
    return _userData.get<T>();
}

template <class T>
T* QAO_Runtime::getUserDataOrThrow() const {
    return _userData.getOrThrow<T>();
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_RUNTIME_HPP

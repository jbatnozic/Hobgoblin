// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_RUNTIME_HPP
#define UHOBGOBLIN_QAO_RUNTIME_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Orderer.hpp>
#include <Hobgoblin/QAO/Registry.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>
#include <Hobgoblin/Utility/Any_ptr.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include <cstdint>
#include <string>

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
    QAO_Runtime(util::AnyPtr userData);
    ~QAO_Runtime();

    //! Create a non-owning reference to this runtime.
    QAO_RuntimeRef nonOwning();

    // Object manipulation
    void attachObject(AvoidNull<QAO_GenericHandle> aHandle);

    // TODO: adding objects with specific IDs seems to be unused (serialization?)
    void attachObject(AvoidNull<QAO_GenericHandle> aHandle, QAO_GenericId aSpecificId);

    //! Detach an object from the runtime.
    //! - If the object was owned by the runtime, an owning handle will be returned.
    //! - Otherwise, a non-owning handle will be returned.
    //! \throws PreconditionNotMetError if the object is not attached to this runtime.
    AvoidNull<QAO_GenericHandle> detachObject(QAO_GenericId aId);
    AvoidNull<QAO_GenericHandle> detachObject(QAO_Base& aObject);
    AvoidNull<QAO_GenericHandle> detachObject(NeverNull<QAO_GenericHandle> aObject);

    void destroyAllOwnedObjects();

    template <class T = QAO_Base>
    QAO_Handle<T> find(const std::string& name) const;

    template <class T = QAO_Base>
    QAO_Handle<T> find(QAO_GenericId id) const;

    template <class T>
    T* find(QAO_Id<T> id) const;

    void updateExecutionPriorityForObject(QAO_Base& object, int new_priority);

    // Execution
    void            startStep();
    void            advanceStep(bool& done, std::int32_t eventFlags = QAO_ALL_EVENT_FLAGS);
    QAO_Event::Enum getCurrentEvent() const;

    // Other
    PZInteger getObjectCount() const noexcept;
    bool      ownsObject(QAO_GenericId aId) const;
    bool      ownsObject(QAO_Base& aObject) const;
    bool      ownsObject(NeverNull<QAO_GenericHandle> aObject) const;

    // User data
    void setUserData(std::nullptr_t);

    template <class T>
    void setUserData(T* value);

    template <class T>
    T* getUserData() const;

    template <class T>
    T* getUserDataOrThrow() const;

    // Orderer/instance iterations:
    QAO_OrdererIterator begin();
    QAO_OrdererIterator end();

    QAO_OrdererReverseIterator rbegin();
    QAO_OrdererReverseIterator rend();

    QAO_OrdererConstIterator cbegin() const;
    QAO_OrdererConstIterator cend() const;

    QAO_OrdererConstReverseIterator crbegin() const;
    QAO_OrdererConstReverseIterator crend() const;

private:
    qao_detail::QAO_Registry _registry;
    qao_detail::QAO_Orderer  _orderer;
    std::int64_t             _step_counter;
    QAO_Event::Enum          _currentEvent;
    QAO_OrdererIterator      _step_orderer_iterator;
    util::AnyPtr             _userData;
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

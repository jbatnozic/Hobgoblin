// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_HANDLE_HPP
#define UHOBGOBLIN_QAO_HANDLE_HPP

#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Base;

namespace qao_detail {
class QAO_HandleFactory;

void DetachFromRuntime(QAO_Base& aObject);
} // namespace qao_detail

//! TODO(add description)
template <class taObject>
class QAO_Handle {
public:
    static_assert(std::is_base_of_v<QAO_Base, taObject>,
                  "QAO_Handle can only point to QAO_Base or its derived classes.");

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CONSTRUCTION                                                    //
    ///////////////////////////////////////////////////////////////////////////

    QAO_Handle() = default;

    QAO_Handle(std::nullptr_t)
        : QAO_Handle{} {}

    //! Destructor.
    //! If the handle references and owns an object at the time of destruction, it will also
    //! detach that object from its runtime (if any) and destroy it, similar to a `std::unique_ptr`.
    ~QAO_Handle() {
        reset();
    }

    //! Copy constructor
    //! \warning a copied `QAO_Handle` will ALWAYS be non-owning even if the original object (`aOther`)
    //!          was owning!
    QAO_Handle(const QAO_Handle& aOther)
        : _object{aOther._object}
        , _isOwning{false} {}

    template <class T, T_ENABLE_IF(std::is_base_of_v<taObject, T> && !std::is_same_v<taObject, T>)>
    QAO_Handle(const QAO_Handle<T>& aOther)
        : _object{aOther._object}
        , _isOwning{false} {}

    template <class T, T_ENABLE_IF(!std::is_base_of_v<taObject, T>)>
    QAO_Handle(const QAO_Handle<T>& aOther) = delete;

    //! Copy assignment
    QAO_Handle& operator=(const QAO_Handle& aOther) {
        if (&aOther != this) {
            reset();
            _object = aOther._object;
        }
        return SELF;
    }

    template <class T, T_ENABLE_IF(std::is_base_of_v<taObject, T> && !std::is_same_v<taObject, T>)>
    QAO_Handle& operator=(const QAO_Handle<T>& aOther) {
        if (&aOther != this) {
            reset();
            _object = aOther._object;
        }
        return SELF;
    }

    template <class T, T_ENABLE_IF(!std::is_base_of_v<taObject, T>)>
    QAO_Handle& operator=(const QAO_Handle<T>& aOther) = delete;

    //! Move constructor
    QAO_Handle(QAO_Handle&& aOther) noexcept
        : _object{aOther._object}
        , _isOwning{aOther._isOwning} //
    {
        aOther._object   = nullptr;
        aOther._isOwning = false;
    }

    template <class T, T_ENABLE_IF(std::is_base_of_v<taObject, T> && !std::is_same_v<taObject, T>)>
    QAO_Handle(QAO_Handle<T>&& aOther) noexcept
        : _object{aOther._object}
        , _isOwning{aOther._isOwning} //
    {
        aOther._object   = nullptr;
        aOther._isOwning = false;
    }

    template <class T, T_ENABLE_IF(!std::is_base_of_v<taObject, T>)>
    QAO_Handle(QAO_Handle<T>&& aOther) = delete;

    //! Move assignment
    QAO_Handle& operator=(QAO_Handle&& aOther) {
        if (&aOther != this) {
            reset();
            _object          = aOther._object;
            _isOwning        = aOther._isOwning;
            aOther._object   = nullptr;
            aOther._isOwning = false;
        }
        return SELF;
    }

    template <class T, T_ENABLE_IF(std::is_base_of_v<taObject, T> && !std::is_same_v<taObject, T>)>
    QAO_Handle& operator=(QAO_Handle<T>&& aOther) {
        if (&aOther != this) {
            reset();
            _object          = aOther._object;
            _isOwning        = aOther._isOwning;
            aOther._object   = nullptr;
            aOther._isOwning = false;
        }
        return SELF;
    }

    template <class T, T_ENABLE_IF(!std::is_base_of_v<taObject, T>)>
    QAO_Handle& operator=(QAO_Handle<T>&& aOther) = delete;

    //! Copy-construct and return a handle to `T`, where `T` is a subtype of `taObject`.
    template <class T, T_ENABLE_IF(std::is_base_of_v<taObject, T>)>
    QAO_Handle<T> downcastCopy() const {
        QAO_Handle<T> result;
        result._object = static_cast<T*>(_object);
        return result;
    }

    //! Move-construct and return a handle to `T`, where `T` is a subtype of `taObject`.
    template <class T, T_ENABLE_IF(std::is_base_of_v<taObject, T>)>
    QAO_Handle<T> downcastMove() {
        QAO_Handle<T> result;
        result._object   = static_cast<T*>(_object);
        result._isOwning = _isOwning;
        _object          = nullptr;
        _isOwning        = false;
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: NULL CHECKING                                                   //
    ///////////////////////////////////////////////////////////////////////////

    bool isNull() const {
        return (_object == nullptr);
    }

    operator bool() const {
        return !isNull();
    }

    bool operator==(std::nullptr_t) const {
        return isNull();
    }

    bool operator!=(std::nullptr_t) const {
        return !isNull();
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: DEREFERENCING                                                   //
    ///////////////////////////////////////////////////////////////////////////

    taObject* ptr() const {
        return _object;
    }

    taObject* operator->() const {
        return _object;
    }

    taObject& operator*() const {
        return *_object;
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: MISC                                                            //
    ///////////////////////////////////////////////////////////////////////////

    bool isOwning() const {
        return _isOwning;
    }

    void reset() {
        if (_isOwning) {
            _deleteObject();
        } else {
            _object = nullptr;
        }
    }

private:
    template <class T>
    friend class QAO_Handle;
    friend class qao_detail::QAO_HandleFactory;

    void _deleteObject() {
        if (_object != nullptr) {
            qao_detail::DetachFromRuntime(*_object);
            _object->_tearDown();
            delete _object;
            _object = nullptr;
        }
        _isOwning = false;
    }

    taObject* _object   = nullptr;
    bool      _isOwning = false;
};

using QAO_GenericHandle = QAO_Handle<QAO_Base>;

namespace qao_detail {
class QAO_HandleFactory {
public:
    template <class taObject>
    static QAO_Handle<taObject> createHandle(taObject* aObject, bool aIsOwning) {
        QAO_Handle<taObject> handle;
        handle._object   = aObject;
        handle._isOwning = aIsOwning;
        return handle;
    }
};
} // namespace qao_detail

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_HANDLE_HPP

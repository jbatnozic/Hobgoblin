// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_QAO_FUNC_HPP
#define UHOBGOBLIN_QAO_FUNC_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/id.hpp>
#include <Hobgoblin/QAO/base.hpp>
#include <Hobgoblin/QAO/runtime.hpp>

#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

template <class T, class ... Args>
T* QAO_PCreate(Args&&... args) {
    T* const object = new T(std::forward<Args>(args)...);

    try {
        if (object->getRuntime() == nullptr || !object->getRuntime()->ownsObject(object)) {
            HG_THROW_TRACED(TracedLogicError, 0, "Will not return unowned object as raw pointer.");
        }
    }
    catch (...) {
        delete object;
        throw;
    }

    return object;
}

template <class T, class ... Args>
QAO_Id<T> QAO_ICreate(Args&&... args) {
    T* const object = new T(std::forward<Args>(args)...);

    try {
        if (object->getRuntime() == nullptr || !object->getRuntime()->ownsObject(object)) {
            HG_THROW_TRACED(TracedLogicError, 0, "Will not return unowned object as QAO_Id.");
        }
    }
    catch (...) {
        delete object;
        throw;
    }

    return QAO_Id<T>{*object};
}

template <class T, class ... Args>
std::unique_ptr<T> QAO_UPCreate(Args&&... args) {
    T* const object = new T(std::forward<Args>(args)...);

    try {
        if (object->getRuntime() != nullptr && object->getRuntime()->ownsObject(object)) {
            HG_THROW_TRACED(TracedLogicError, 0, "Will not return already owned object as std::unique_ptr.");
        }
    }
    catch (...) {
        delete object;
        throw;
    }

    return std::unique_ptr<T>{object};
}

inline void QAO_PDestroy(QAO_Base* object) {
    delete object;
}

inline void QAO_IDestroy(QAO_GenericId id, QAO_Runtime& runtime) {
    QAO_Base* const object = runtime.find(id);
    if (object == nullptr || !runtime.ownsObject(object)) {
        HG_THROW_TRACED(TracedLogicError, 0, "Cannot delete object which is not owned by this runtime.");
    }
    delete object;
}

inline void QAO_UPDestroy(std::unique_ptr<QAO_Base> object) {
    object.reset();
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_FUNC_HPP

// clang-format on

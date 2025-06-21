// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_FUNCTIONS_CREATE_HPP
#define UHOBGOBLIN_QAO_FUNCTIONS_CREATE_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Instantiation_key.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>

#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

//! Use this function to create an instance of `T` on the heap, using `operator new`.
//!
//! \param aRuntimeRef reference to a runtime to which to attach the object.
//!                    If NULL, the object will not be attached to any runtime.
//! \param aArgs parameters that will be forwarded to the constructor of `T` after
//!              the instantiation guard (`QAO_IKey`).
//!
//! \returns handle to the newly created object. If the passed runtime ref was non-NULL and
//!          owning, the runtime will own the object; thus the returned handle will be
//!          non-owning. Oherwise the returned handle will own the object.
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs) {
    QAO_IKey ikey;
    T*       object = new T(ikey, std::forward<taArgs>(aArgs)...);

    bool didSetUp = false;
    try {
        object->_setUp();
        didSetUp = true;

        auto* rt = aRuntimeRef.ptr();
        if (rt) {
            const bool rtOwnsObject = aRuntimeRef.isOwning();
            rt->attachObject(qao_detail::QAO_HandleFactory::createHandle(object, rtOwnsObject));
            return qao_detail::QAO_HandleFactory::createHandle(object, !rtOwnsObject);
        }
        return qao_detail::QAO_HandleFactory::createHandle(object, true);
    } catch (...) {
        if (didSetUp) {
            object->_tearDown();
        }
        delete object;
        throw;
    }

    HG_UNREACHABLE();
    return {};
}

//! TODO: QAO_ICreate

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_FUNCTIONS_CREATE_HPP

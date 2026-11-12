// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_FUNCTIONS_CREATE_HPP
#define UHOBGOBLIN_QAO_FUNCTIONS_CREATE_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Utility/Swallow.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Instantiation_guard.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>

#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

///////////////////////////////////////////////////////////////////////////
// MARK: Public declarations                                             //
///////////////////////////////////////////////////////////////////////////

//! Use this function to create an instance of `T` on the heap, using `operator new`.
//!
//! \param aRuntimeRef reference to a runtime to which to attach the object.
//!                    If NULL, the object will not be attached to any runtime.
//! \param aArgs parameters that will be forwarded to the constructor of `T` after
//!              the instantiation guard (`QAO_InstGuard`).
//!
//! \returns handle to the newly created object. If the passed runtime ref was non-NULL and
//!          owning, the runtime will own the object; thus the returned handle will be
//!          non-owning. Oherwise the returned handle will own the object.
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);

//! TODO(add description)
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);

//! TODO: QAO_ICreate

///////////////////////////////////////////////////////////////////////////
// MARK: Implementations (internal)                                      //
///////////////////////////////////////////////////////////////////////////

#define UHOBGOBLIN_QAO_CREATE_BODY(_ta_args_, _runtime_ref_, _args_)                             \
    QAO_InstGuard instGuard;                                                                     \
    T*            object = new T(instGuard, std::forward<_ta_args_>(_args_)...);                 \
                                                                                                 \
    bool didSetUp = false;                                                                       \
    try {                                                                                        \
        static_cast<QAO_Base*>(object)->_setUp();                                                \
        didSetUp = true;                                                                         \
                                                                                                 \
        auto* rt = (_runtime_ref_).ptr();                                                        \
        if (rt) {                                                                                \
            const bool rtOwnsObject = (_runtime_ref_).isOwning();                                \
            rt->attachObject(qao_detail::QAO_HandleFactory::createHandle(object, rtOwnsObject)); \
            return qao_detail::QAO_HandleFactory::createHandle(object, !rtOwnsObject);           \
        }                                                                                        \
        return qao_detail::QAO_HandleFactory::createHandle(object, true);                        \
    } catch (...) {                                                                              \
        if (didSetUp) {                                                                          \
            util::Swallow([object]() {                                                           \
                static_cast<QAO_Base*>(object)->_tearDown();                                     \
            });                                                                                  \
        }                                                                                        \
        delete object;                                                                           \
        throw;                                                                                   \
    }                                                                                            \
                                                                                                 \
    HG_UNREACHABLE();                                                                            \
    return {}

template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs) {
    UHOBGOBLIN_QAO_CREATE_BODY(taArgs, aRuntimeRef, aArgs);
}

template <class T, class... taArgs>
QAO_Handle<T> QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs) {
    UHOBGOBLIN_QAO_CREATE_BODY(taArgs, aRuntimeRef, aArgs);
}

#undef UHOBGOBLIN_QAO_CREATE_BODY

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_FUNCTIONS_CREATE_HPP

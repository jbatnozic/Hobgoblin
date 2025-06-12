// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_FUNC_HPP
#define UHOBGOBLIN_QAO_FUNC_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Instantiation_key.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>

#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

//! TODO: add description
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

template <class T>
void QAO_Destroy(QAO_Handle<T> aHandle) {
    if (!aHandle) {
        return;
    }

    auto*      object       = aHandle.ptr();
    auto*      rt           = object->getRuntime();
    const bool rtOwnsObject = rt != nullptr && rt->ownsObject(*object);
    if (!rtOwnsObject && !aHandle.isOwning()) {
        HG_THROW_TRACED(InvalidArgumentError,
                        0,
                        "Passed handle does not own its object nor does it point to an object owned by "
                        "its runtime.");
    }

    rt->detachObject(*object);
    aHandle.reset();
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_FUNC_HPP

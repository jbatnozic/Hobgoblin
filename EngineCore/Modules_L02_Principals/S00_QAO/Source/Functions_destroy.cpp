// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/QAO/Functions_destroy.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>

#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

void QAO_Destroy(QAO_GenericHandle&& aHandle) {
    if (!aHandle) {
        return;
    }

    auto* object = aHandle.ptr();
    auto* rt     = object->getRuntime();

    auto rtHandle = rt ? MoveToUnderlying(rt->detachObject(*object)) : QAO_GenericHandle{};

    if (aHandle.isOwning()) {
        aHandle._object   = nullptr;
        aHandle._isOwning = false;
    } else if (rtHandle.isOwning()) {
        rtHandle._object   = nullptr;
        rtHandle._isOwning = false;
    } else {
        HG_THROW_TRACED(InvalidArgumentError,
                        0,
                        "Passed handle does not own its object nor does it point to an object owned by "
                        "its runtime.");
    }

    object->_tearDown();
    delete object;
}

void QAO_Destroy(QAO_Base* aObject) {
    if (!aObject) {
        return;
    }

    auto* rt       = aObject->getRuntime();
    auto  rtHandle = rt ? MoveToUnderlying(rt->detachObject(*aObject)) : QAO_GenericHandle{};

    if (rtHandle.isNull() || !rtHandle.isOwning()) {
        HG_THROW_TRACED(InvalidArgumentError,
                        0,
                        "Passed pointer does not point to an object owned by its runtime.");
    }

    // Letting the handle go out of scope will destroy the object
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

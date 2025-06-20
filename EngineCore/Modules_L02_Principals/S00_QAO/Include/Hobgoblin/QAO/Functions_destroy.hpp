// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_FUNCTIONS_DESTROY_HPP
#define UHOBGOBLIN_QAO_FUNCTIONS_DESTROY_HPP

#include <Hobgoblin/QAO/Handle.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Base;

//! Destroys the object pointed to by the passed handle, provided that the object
//! is owned by either the handle or the object's runtime (if attached at all).
//! 
//! \note does nothing if passed a NULL handle.
//!
//! \throws InvalidArgumentError if the object is not owned by the passed handle
//!         nor its runtime.
//!
//! \warning if the relevant object wasn't created using the `QAO_Create` function,
//!          this function call is Undefined Behaviour!
void QAO_Destroy(QAO_GenericHandle&& aHandle);

//! Destroys the object pointed to by the passed pointer, provided that the object
//! is attached to a runtime and owned by that runtime.
//! 
//! \note does nothing if passed a NULL pointer.
//!
//! \throws InvalidArgumentError if the object is not owned by the runtime it is
//!         attached to.
//!
//! \warning if the relevant object wasn't created using the `QAO_Create` function,
//!          this function call is Undefined Behaviour!
void QAO_Destroy(QAO_Base* aObject);

//! Destroys the object pointed to by the passed reference, provided that the object
//! is attached to a runtime and owned by that runtime.
//!
//! \throws InvalidArgumentError if the object is not owned by the runtime it is
//!         attached to.
//!
//! \warning if the relevant object wasn't created using the `QAO_Create` function,
//!          this function call is Undefined Behaviour!
inline void QAO_Destroy(QAO_Base& aObject) {
    QAO_Destroy(&aObject);
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_FUNCTIONS_DESTROY_HPP

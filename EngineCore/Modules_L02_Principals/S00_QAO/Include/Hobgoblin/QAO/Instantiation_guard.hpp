// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_INSTANTIATION_GUARD_HPP
#define UHOBGOBLIN_QAO_INSTANTIATION_GUARD_HPP

#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

// Forward-declare the create function.
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);

//! This object is a mandatory parameter of the constructor of `QAO_Base`.
//! Because it has a private constructor, it can only be constructed by certain
//! select parts of the QAO framework; thus it guards against incorrect instantiation
//! of objects of classes derived from `QAO_Base`.
//!
//! \note these derived classes (mentioned above) need to accept the instantiation
//!       guard as the first parameter of their constructor(s), and then pass it on
//!       to the constructor of `QAO_Base`.
struct QAO_InstGuard {
private:
    QAO_InstGuard() = default;

    template <class T, class... taArgs>
    friend QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_INSTANTIATION_GUARD_HPP

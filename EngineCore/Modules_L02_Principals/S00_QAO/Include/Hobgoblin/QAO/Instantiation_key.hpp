// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_INSTANTIATION_KEY_HPP
#define UHOBGOBLIN_QAO_INSTANTIATION_KEY_HPP

#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Runtime_ref.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

// Forward-declare the create function.
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);

//! Instantiation key. TBD: Instantiation guard?
struct QAO_IKey {
private:
    QAO_IKey() = default;

    template <class T, class... taArgs>
    friend QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_INSTANTIATION_KEY_HPP

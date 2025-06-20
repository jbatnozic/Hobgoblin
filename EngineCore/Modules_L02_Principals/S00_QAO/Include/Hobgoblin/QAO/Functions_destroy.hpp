// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_FUNCTIONS_DESTROY_HPP
#define UHOBGOBLIN_QAO_FUNCTIONS_DESTROY_HPP

#include <Hobgoblin/QAO/Handle.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Base;

void QAO_Destroy(QAO_GenericHandle&& aHandle);

void QAO_Destroy(QAO_Base* aObject);

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_FUNCTIONS_DESTROY_HPP

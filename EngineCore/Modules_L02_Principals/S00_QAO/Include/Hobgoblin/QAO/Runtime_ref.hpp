// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_RUNTIME_REF_HPP
#define UHOBGOBLIN_QAO_RUNTIME_REF_HPP

#include <cstddef>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Runtime;

// TODO: add description
class QAO_RuntimeRef {
public:
    QAO_RuntimeRef() noexcept = default;

    QAO_RuntimeRef(std::nullptr_t) noexcept
        : QAO_RuntimeRef{} {}

    QAO_RuntimeRef(QAO_Runtime& runtime) noexcept
        : _runtime{&runtime}
        , _isOwning{true} {}

    QAO_RuntimeRef(QAO_Runtime* runtime) noexcept
        : _runtime{runtime}
        , _isOwning{true} {}

    QAO_Runtime* ptr() const noexcept {
        return _runtime;
    }

    bool isOwning() const noexcept {
        return _isOwning;
    }

private:
    QAO_Runtime* _runtime  = nullptr;
    bool         _isOwning = false;

    friend class QAO_Runtime;
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_RUNTIME_REF_HPP

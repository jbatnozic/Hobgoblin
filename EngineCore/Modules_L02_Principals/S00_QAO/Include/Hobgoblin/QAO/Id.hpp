// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_ID_HPP
#define UHOBGOBLIN_QAO_ID_HPP

#include <Hobgoblin/Common.hpp>

#include <cstdint>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Base;
class QAO_Runtime;
namespace qao_detail {
class QAO_Registry;
} // namespace qao_detail

template <class T>
class QAO_Id;

class QAO_GenericId {
public:
    QAO_GenericId();
    QAO_GenericId(std::nullptr_t p);
    explicit QAO_GenericId(const QAO_Base* object);
    explicit QAO_GenericId(const QAO_Base& object);

    bool operator==(const QAO_GenericId& other) const;
    bool operator!=(const QAO_GenericId& other) const;

    // Copy:
    QAO_GenericId(const QAO_GenericId& other)            = default;
    QAO_GenericId& operator=(const QAO_GenericId& other) = default;

    // Move:
    QAO_GenericId(QAO_GenericId&& other)            = default;
    QAO_GenericId& operator=(QAO_GenericId&& other) = default;

    // Utility:
    PZInteger    getIndex() const noexcept;
    std::int64_t getSerial() const noexcept;
    bool         isNull() const noexcept;

    template <class T>
    QAO_Id<T> cast() const noexcept;

protected:
    friend class QAO_Runtime;
    friend class qao_detail::QAO_Registry;
    QAO_GenericId(std::int64_t serial, PZInteger index);

private:
    std::int64_t _serial;
    PZInteger    _index;
};

template <class T>
class QAO_Id : public QAO_GenericId {
public:
    using QAO_GenericId::QAO_GenericId;

    // Copy:
    QAO_Id(const QAO_Id<T>& other)            = default;
    QAO_Id& operator=(const QAO_Id<T>& other) = default;

    // Move:
    QAO_Id(QAO_Id&& other)            = default;
    QAO_Id& operator=(QAO_Id&& other) = default;

protected:
    friend class QAO_GenericId;
    QAO_Id(std::int64_t serial, PZInteger index)
        : QAO_GenericId{serial, index} {}
};

template <class T>
QAO_Id<T> QAO_GenericId::cast() const noexcept {
    return QAO_Id<T>{_serial, _index};
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_ID_HPP

// clang-format on

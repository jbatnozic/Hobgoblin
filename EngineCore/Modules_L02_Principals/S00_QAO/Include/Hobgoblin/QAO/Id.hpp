// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_ID_HPP
#define UHOBGOBLIN_QAO_ID_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Config.hpp>

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

    bool operator==(const QAO_GenericId& other) const;
    bool operator!=(const QAO_GenericId& other) const;

    // Copy:
    QAO_GenericId(const QAO_GenericId& other)            = default;
    QAO_GenericId& operator=(const QAO_GenericId& other) = default;

    // Move:
    QAO_GenericId(QAO_GenericId&& other)            = default;
    QAO_GenericId& operator=(QAO_GenericId&& other) = default;

    // Utility:
    QAO_Index  getIndex() const noexcept;
    QAO_Serial getSerial() const noexcept;
    bool       isNull() const noexcept;

    template <class T>
    QAO_Id<T> cast() const noexcept;

protected:
    friend class QAO_Runtime;
    friend class qao_detail::QAO_Registry;
    QAO_GenericId(QAO_Serial serial, QAO_Index index);

private:
    QAO_Serial _serial;
    QAO_Index  _index;
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
    QAO_Id(QAO_Serial serial, QAO_Index index)
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

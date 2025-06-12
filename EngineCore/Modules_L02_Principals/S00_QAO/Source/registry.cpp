// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Registry.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {
namespace qao_detail {

QAO_Registry::QAO_Registry(PZInteger capacity)
    : _indexer{capacity}
    , _elements{ToSz(capacity)}
    , _serialCounter{QAO_STARTING_SERIAL} {}

QAO_GenericId QAO_Registry::insert(QAO_GenericHandle aHandle) {
    HG_VALIDATE_ARGUMENT(!!aHandle);

    const auto index  = static_cast<QAO_Index>(_indexer.acquire());
    const auto serial = _nextSerial();

    const auto didInsert = _serialToIndex.insert(std::make_pair(serial, index)).second;
    if (!didInsert) {
        HG_THROW_TRACED(TracedRuntimeError, 0, "Fatal serial number clash with value {}.", serial);
    }

    _adjustSize();

    const auto id = QAO_GenericId{serial, index};

    _elements[ToSz(index)] = Elem(id, std::move(aHandle));

    return id;
}

void QAO_Registry::insertWithId(QAO_GenericHandle aHandle, QAO_GenericId aId) {
    if (!_indexer.tryAcquireSpecific(aId.getIndex())) {
        HG_THROW_TRACED(TracedLogicError,
                        0,
                        "Cannot register object; Index {} already in use.",
                        aId.getIndex());
    }

    const auto didInsert = _serialToIndex.insert(std::make_pair(aId.getSerial(), aId.getIndex())).second;
    if (!didInsert) {
        HG_THROW_TRACED(TracedRuntimeError,
                        0,
                        "Fatal serial number clash with value {}.",
                        aId.getSerial());
    }

    _adjustSize();

    _elements[ToSz(aId.getIndex())] = {aId, std::move(aHandle)};
}

QAO_GenericHandle QAO_Registry::remove(QAO_Index aIndex) {
    Elem& elem = _elements.at(ToSz(aIndex));
    if (elem.handle.isNull()) {
        return {};
    }

    QAO_GenericHandle rv = std::move(elem.handle);

    _serialToIndex.erase(elem.id.getSerial());

    elem = {};
    _indexer.free(aIndex);

    return rv;
}

QAO_GenericHandle QAO_Registry::findObjectWithIndex(QAO_Index aIndex) const {
    const auto szIdx = ToSz(aIndex);
    if (szIdx >= _elements.size()) {
        return {};
    }
    return _elements[szIdx].handle;
}

QAO_GenericHandle QAO_Registry::findObjectWithSerial(QAO_Serial aSerial) const {
    const auto iter = _serialToIndex.find(aSerial);
    if (iter == _serialToIndex.end()) {
        return {};
    }

    return _elements[ToSz(iter->second)].handle;
}

QAO_GenericHandle QAO_Registry::findObjectWithId(QAO_GenericId aId) const {
    const auto szIdx = ToSz(aId.getIndex());
    if (szIdx >= _elements.size()) {
        return {};
    }
    return _elements[szIdx].handle;
}

bool QAO_Registry::isObjectWithIndexOwned(QAO_Index aIndex) const {
    HG_VALIDATE_ARGUMENT(ToSz(aIndex) < _elements.size());
    const auto& elem = _elements[ToSz(aIndex)];
    HG_VALIDATE_PRECONDITION(!elem.handle.isNull());
    return elem.handle.isOwning();
}

// QAO_Serial QAO_Registry::serialAt(QAO_Index index) const {
//     return _elements[index].serial;
// }

QAO_Index QAO_Registry::instanceCount() const {
    return _indexer.countFilled();
}

// bool QAO_Registry::isSlotEmpty(QAO_Index index) const {
//     return _indexer.isSlotEmpty(static_cast<std::size_t>(index));
// }

// MARK: Private

void QAO_Registry::_adjustSize() {
    const auto indexerSize = _indexer.getSize();

    if (indexerSize > _elements.size()) {
        _elements.resize(indexerSize);
    }
}

QAO_Serial QAO_Registry::_nextSerial() {
    QAO_Serial rv = _serialCounter;
    _serialCounter += 1;
    return rv;
}

} // namespace qao_detail
} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

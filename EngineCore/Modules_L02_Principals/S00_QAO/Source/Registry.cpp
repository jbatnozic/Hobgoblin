// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Registry.hpp>

#include <algorithm>
#include <cstddef>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {
namespace qao_detail {

QAO_Registry::QAO_Registry(PZInteger capacity)
    : _indexer{capacity}
    , _elements{ToSz(capacity)}
    , _serialCounter{QAO_STARTING_SERIAL} {}

/*
    In order to provide the Strong Exception Guarantee (i.e., in case of exception being thrown,
    the container is left in a valid, unmodified state), all non-const methods of `QAO_Registry`
    are split into 3 sections:
      - Check   : Check state and input parameters; can throw; no state change.
      - Reserve : Grow containers as needed.
      - Commit  :
    In all of the above, we're ignoring the existence of `std::bad_alloc`, because if that happens,
    the app is fucked anyway.
*/

QAO_GenericId QAO_Registry::insert(QAO_GenericHandle&& aHandle) {
    // SECTION: Check

    HG_VALIDATE_ARGUMENT(!!aHandle);

    const auto serial = _serialCounter;
    if (_serialToIndex.count(serial) != 0) {
        HG_THROW_TRACED(TracedRuntimeError, 0, "Fatal serial number clash with value {}.", serial);
    }

    // SECTION: Reserve

    if (_indexer.countEmpty() == 0) {
        _indexer.resize(static_cast<PZInteger>(_indexer.getSize()) + 1);
    }
    _adjustSize(_indexer.getSize());
    HG_HARD_ASSERT(_indexer.countEmpty() > 0 && _elements.size() >= _indexer.getSize());

    // SECTION: Commit

    const auto iter = _serialToIndex.emplace(serial, QAO_Index{}).first; // Index filled in below

    const auto index = static_cast<QAO_Index>(_indexer.acquire());
    iter->second     = index;
    _serialCounter++;

    const auto id = QAO_GenericId{serial, index};

    _elements[ToSz(index)] = {id, std::move(aHandle)};

    return id;
}

void QAO_Registry::insertWithId(QAO_GenericHandle&& aHandle, QAO_GenericId aId) {
    // SECTION: Check

    HG_VALIDATE_ARGUMENT(!!aHandle);

    const auto index  = aId.getIndex();
    const auto serial = aId.getSerial();

    if (ToSz(index) < _indexer.getSize() && !_indexer.isSlotEmpty(index)) {
        HG_THROW_TRACED(TracedLogicError, 0, "Cannot register object; Index {} already in use.", index);
    }
    if (_serialToIndex.count(serial) != 0) {
        HG_THROW_TRACED(TracedRuntimeError, 0, "Fatal serial number clash with value {}.", serial);
    }

    // SECTION: Reserve

    _indexer.resize(std::max(index + 1, static_cast<PZInteger>(_indexer.getSize())));
    _adjustSize(_indexer.getSize());
    HG_HARD_ASSERT(_indexer.isSlotEmpty(index) && ToSz(index) < _elements.size());

    // SECTION: Commit

    _serialToIndex.emplace(serial, index);

    (void)_indexer.tryAcquireSpecific(index); // We already know the slot is in range and empty

    _elements[ToSz(index)] = {aId, std::move(aHandle)};
}

QAO_GenericHandle QAO_Registry::remove(QAO_Index aIndex) {
    // SECTION: Check (and, in this case, an early return is possible)

    Elem& elem = _elements.at(ToSz(aIndex));
    if (elem.handle.isNull()) {
        return {};
    }
    HG_HARD_ASSERT(!_indexer.isSlotEmpty(aIndex));

    // SECTION: Reserve

    // (empty in this case)

    // SECTION: Commit

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

QAO_Index QAO_Registry::instanceCount() const {
    return _indexer.countFilled();
}

// MARK: Private

void QAO_Registry::_adjustSize(std::size_t aMinSize) {
    if (aMinSize > _elements.size()) {
        _elements.resize(aMinSize);
    }
}

} // namespace qao_detail
} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

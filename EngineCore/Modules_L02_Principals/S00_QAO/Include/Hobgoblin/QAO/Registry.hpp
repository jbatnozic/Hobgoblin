// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_REGISTRY_HPP
#define UHOBGOBLIN_QAO_REGISTRY_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/Utility/Slab_indexer.hpp>

#include <cstddef>
#include <unordered_map>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Base;

namespace qao_detail {

class QAO_Registry {
public:
    QAO_Registry(PZInteger aCapacity = 1);

    //! Insert an object into the registry and assign it a new ID.
    //! \returns the ID assigned to the object.
    //! \throws InvalidArgumentError if `aHandle` is null.
    //! \throws TracedRuntimeError on serial number clash.
    //! \note provides the strong exception guarantee: if this method throws, the registry
    //!       is left unchanged and `aHandle` is not moved from.
    QAO_GenericId insert(QAO_GenericHandle&& aHandle);

    //! Insert an object into the registry with a specific, predetermined ID.
    //! \throws InvalidArgumentError if `aHandle` is null.
    //! \throws TracedLogicError if the index of `aId` is already in use.
    //! \throws TracedRuntimeError if the serial of `aId` is already in use.
    //! \note provides the strong exception guarantee: if this method throws, the registry
    //!       is left unchanged and `aHandle` is not moved from.
    void insertWithId(QAO_GenericHandle&& aHandle, QAO_GenericId aId);

    //! Remove the object with the given index from the registry.
    //! \returns the handle that was stored in the registry (owning or non-owning),
    //!          or a null handle if there was no object with this index.
    //! \throws std::out_of_range if `aIndex` is outside of the registry's current size.
    //! \note provides the strong exception guarantee.
    QAO_GenericHandle remove(QAO_Index aIndex);

    //! Look for an object with the specified index.
    //! \returns non-owning handle to the object if it is found, or a null handle if it is not found.
    //! \note this function has O(1) complexity.
    QAO_GenericHandle findObjectWithIndex(QAO_Index aIndex) const;

    //! Look for an object with the specified serial number.
    //! \returns non-owning handle to the object if it is found, or a null handle if it is not found.
    //! \note this function has O(log(N)) complexity.
    QAO_GenericHandle findObjectWithSerial(QAO_Serial aSerial) const;

    //! Look for an object with the specified ID.
    //! \returns non-owning handle to the object if it is found, or a null handle if it is not found.
    //! \note this function has O(1) complexity.
    QAO_GenericHandle findObjectWithId(QAO_GenericId aId) const;

    //! Check if an object by the given index is owned by the registry.
    //! Throws if the registry does not contain an object by this index at all.
    bool isObjectWithIndexOwned(QAO_Index aIndex) const;

    //! Count currently inserted instances (non-owned + owned).
    PZInteger instanceCount() const;

private:
    struct Elem {
        QAO_GenericId     id;
        QAO_GenericHandle handle;
    };

    util::SlabIndexer _indexer;
    std::vector<Elem> _elements;
    QAO_Serial        _serialCounter;

    std::unordered_map<QAO_Serial, QAO_Index> _serialToIndex;

    void _adjustSize(std::size_t aMinSize);
};

} // namespace qao_detail
} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_REGISTRY_HPP

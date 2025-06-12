// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_REGISTRY_HPP
#define UHOBGOBLIN_QAO_REGISTRY_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/Utility/Slab_indexer.hpp>

#include <cstdint>
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

    QAO_GenericId     insert(QAO_GenericHandle aHandle);
    void              insertWithId(QAO_GenericHandle aHandle, QAO_GenericId aId);
    QAO_GenericHandle remove(PZInteger aIndex);

    //! Look for an object with the specified index.
    //! \returns non-owning handle to the object if it is found, or a null handle if it is not found.
    //! \note this function has O(1) complexity.
    QAO_GenericHandle findObjectWithIndex(PZInteger aIndex) const;

    //! Look for an object with the specified serial number.
    //! \returns non-owning handle to the object if it is found, or a null handle if it is not found.
    //! \note this function has O(log(N)) complexity.
    QAO_GenericHandle findObjectWithSerial(std::int64_t aSerial) const;

    //! Look for an object with the specified ID.
    //! \returns non-owning handle to the object if it is found, or a null handle if it is not found.
    //! \note this function has O(1) complexity.
    QAO_GenericHandle findObjectWithId(QAO_GenericId aId) const;

    //!
    bool isObjectWithIndexOwned(PZInteger aIndex) const;
    // std::int64_t      serialAt(PZInteger index) const;
    PZInteger instanceCount() const;
    // bool              isSlotEmpty(PZInteger index) const;

private:
    struct Elem {
        QAO_GenericId     id;
        QAO_GenericHandle handle;
    };

    util::SlabIndexer _indexer;
    std::vector<Elem> _elements;
    std::int64_t      _serialCounter;

    std::unordered_map<std::int64_t, PZInteger> _serialToIndex;

    void         _adjustSize();
    std::int64_t _nextSerial();
};

} // namespace qao_detail
} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_REGISTRY_HPP

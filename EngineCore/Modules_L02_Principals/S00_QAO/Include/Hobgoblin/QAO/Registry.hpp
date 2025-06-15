// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_REGISTRY_HPP
#define UHOBGOBLIN_QAO_REGISTRY_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/Utility/Slab_indexer.hpp>

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

    bool isObjectWithIndexOwned(QAO_Index aIndex) const;
    
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

    void       _adjustSize();
    QAO_Serial _nextSerial();
};

} // namespace qao_detail
} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_REGISTRY_HPP

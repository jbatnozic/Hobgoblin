// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Reflection.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

const QAO_TypeIndexToClassMetadataMap& QAO_GetAllTypeIndexToClassMetadataMappings() {
    static QAO_TypeIndexToClassMetadataMap map;
    return map;
}

#define GetTypeIdxMap() \
    const_cast<QAO_TypeIndexToClassMetadataMap&>(QAO_GetAllTypeIndexToClassMetadataMappings())

const QAO_UniqueNameToClassMetadataMap& QAO_GetAllUniqueNameToClassMetadataMappings() {
    static QAO_UniqueNameToClassMetadataMap map;
    return map;
}

#define GetUniqNameMap() \
    const_cast<QAO_UniqueNameToClassMetadataMap&>(QAO_GetAllUniqueNameToClassMetadataMappings())

namespace qao_detail {

QAO_ClassMetadata& QAO_RegisterClass(const std::type_info& aTypeInfo, std::string_view aUniqueName) {
    auto& typeIdxMap  = GetTypeIdxMap();
    auto& uniqNameMap = GetUniqNameMap();

    if (uniqNameMap.contains(aUniqueName)) {
        assert(false && "QAO: Failed to register class - name is not unique!");
        HG_THROW_TRACED(PreconditionNotMetError, 0, "TODO"); // TODO
    }

    auto [iter, didInsert] = typeIdxMap.try_emplace(std::type_index{aTypeInfo});

    if (!didInsert) {
        assert(false && "QAO: Failed to register class - type index is not unique!");
        HG_THROW_TRACED(PreconditionNotMetError, 0, "TODO"); // TODO
    }

    uniqNameMap.insert(std::make_pair(aUniqueName, std::ref(iter->second)));

    return iter->second;
}

} // namespace qao_detail

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Reflection.hpp>

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

// MARK: Maps

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

// MARK: Class Metadata

const QAO_ClassMetadata* QAO_ClassMetadata::get(std::string_view aUniqueName) {
    const auto& uniqNameMap = GetUniqNameMap();
    const auto  iter        = uniqNameMap.find(aUniqueName);
    if (iter == uniqNameMap.end()) {
        return nullptr;
    }
    return &(iter->second);
}

const QAO_ClassMetadata* QAO_ClassMetadata::get(std::type_index aTypeIndex) {
    const auto& typeIdxMap = GetTypeIdxMap();
    const auto  iter       = typeIdxMap.find(aTypeIndex);
    if (iter == typeIdxMap.end()) {
        return nullptr;
    }
    return &(iter->second);
}

const QAO_ClassMetadata* QAO_ClassMetadata::get(const std::type_info& aTypeInfo) {
    return get(std::type_index(aTypeInfo));
}

PZInteger QAO_ClassMetadata::getClassCount() {
    return stopz(GetTypeIdxMap().size());
}

// MARK: Registering classes

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

// MARK: Sending messages

namespace qao_detail {

bool QAO_SendMessage(QAO_MessageHandlerMap& aMessageHandlerMap,
                     std::type_index        aMessageTypeIndex,
                     QAO_Base&              aReceiverInstance,
                     void*                  aMessagePayloadPtr) {
    QAO_UntypedMessage untypedMessagePtr = nullptr;

    aMessageHandlerMap.lazy_emplace_l(
        aMessageTypeIndex,
        // --- CASE 1: Key already exists ---
        // Called under a submap lock.
        [&untypedMessagePtr](QAO_MessageHandlerMap::value_type& aPair) {
            untypedMessagePtr = aPair.second; // Copy the existing value
        },
        // --- CASE 2: Key is missing ---
        // Called under a submap lock.
        [&untypedMessagePtr, &aMessageTypeIndex, &aReceiverInstance](
            const QAO_MessageHandlerMap::constructor& aCtor) {
            // Generate the value lazily
            const auto* metadata = QAO_ClassMetadata::get(typeid(aReceiverInstance));
            if (!metadata) {
                aCtor(aMessageTypeIndex, nullptr);
                return;
            }

            const auto iter = metadata->_messageHandlers.find(aMessageTypeIndex);
            if (iter == metadata->_messageHandlers.end()) {
                aCtor(aMessageTypeIndex, nullptr);
                return;
            }

            untypedMessagePtr = iter->second;

            aCtor(aMessageTypeIndex, untypedMessagePtr); // Atomically construct it in-place
        });

    if (!untypedMessagePtr) {
        return false;
    }

    untypedMessagePtr(aReceiverInstance, aMessagePayloadPtr);
    return true;
}

} // namespace qao_detail

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

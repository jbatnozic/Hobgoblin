// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Reflection.hpp>

#include <cassert>
#include <deque>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

QAO_REGISTER_CLASS(QAO_Base, UHOBGOBLIN_QAO_Base) {
    /* This is the only class which doesn't need to have its superclass set */
}

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

std::string_view QAO_ClassMetadata::getUniqueName() const {
    return _selfUniqueName;
}

const std::type_info& QAO_ClassMetadata::getTypeInfo() const {
    return _selfTypeInfo;
}

std::type_index QAO_ClassMetadata::getTypeIndex() const {
    return std::type_index{_selfTypeInfo};
}

const QAO_ClassMetadata* QAO_ClassMetadata::getSuperclass() const {
    return _superclassMetadata;
}

std::span<const QAO_ClassMetadata* const> QAO_ClassMetadata::getChildClasses() const {
    return _childClasses;
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

    auto [iter, didInsert] = typeIdxMap.try_emplace(std::type_index{aTypeInfo}, aTypeInfo, aUniqueName);

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
                     void*                  aMessagePayloadPtr,
                     bool                   aConst) {
    QAO_UntypedMessage untypedMessagePtr = nullptr;

    const auto instTypeIdx = std::type_index{typeid(aReceiverInstance)};
    aMessageHandlerMap.lazy_emplace_l(
        instTypeIdx,
        // --- CASE 1: Key already exists ---
        // Called under a submap lock.
        [&untypedMessagePtr](QAO_MessageHandlerMap::value_type& aPair) {
            untypedMessagePtr = aPair.second; // Copy the existing value
        },
        // --- CASE 2: Key is missing ---
        // Called under a submap lock.
        [&untypedMessagePtr, &aMessageTypeIndex, &aReceiverInstance, instTypeIdx](
            const QAO_MessageHandlerMap::constructor& aCtor) //
        {
            // Generate the value lazily
            const auto* metadata = QAO_ClassMetadata::get(typeid(aReceiverInstance));
            if (!metadata) {
                aCtor(instTypeIdx, nullptr); // Atomically construct it in-place
                return;
            }

            const auto iter = metadata->_messageHandlers.find(aMessageTypeIndex);
            if (iter == metadata->_messageHandlers.end()) {
                aCtor(instTypeIdx, nullptr); // Atomically construct it in-place
                return;
            }

            untypedMessagePtr = iter->second;

            aCtor(instTypeIdx, untypedMessagePtr); // Atomically construct it in-place
        });

    if (!untypedMessagePtr) {
        return false;
    }

    untypedMessagePtr(aReceiverInstance, aMessagePayloadPtr, aConst);
    return true;
}

} // namespace qao_detail

// MARK: Init
namespace {
bool g_qaoMetadataInitialized = false;
} // namespace

void QAO_InitializeMetadata() {
    {
        bool b = true;
        std::swap(b, g_qaoMetadataInitialized);
        if (b) {
            return;
        }
    }

    auto& typeIdxMap  = GetTypeIdxMap();
    auto& uniqNameMap = GetUniqNameMap();

    const auto QAO_BASE_TYPE_IDX = std::type_index{typeid(QAO_Base)};

    // Resolve superclasses and child classes
    {
        for (auto& [typeIndex, metadata] : typeIdxMap) {
            if (typeIndex == QAO_BASE_TYPE_IDX) {
                continue; // QAO_Base doesn't have a superclass in the QAO hierarchy
            }

            const auto* superclassTypeInfo = metadata._superclassTypeInfo;
            if (!superclassTypeInfo) {
                HG_THROW_TRACED(TracedLogicError,
                                0,
                                "No superclass was set for class '{}'",
                                metadata._selfUniqueName);
            }

            auto iter = typeIdxMap.find(std::type_index{*metadata._superclassTypeInfo});
            if (iter == typeIdxMap.end()) {
                HG_THROW_TRACED(TracedLogicError,
                                0,
                                "No superclass of class '{}' points to a non-registered type '{}'",
                                metadata._selfUniqueName,
                                superclassTypeInfo->name());
            }

            metadata._superclassMetadata = &(iter->second);
            (iter->second)._childClasses.push_back(&metadata);
        }
    }

    // By this point, the classes will form a structure pretty much like a tree inside the map,
    // with the metadata for QAO_Base being the root. We can now traverse the tree breadth-first
    // in order to resolve message handler inheritance.

    {
        PZInteger visitedClassesCount = 1; // We don't really visit `QAO_Base` but it counts

        std::deque<QAO_ClassMetadata*> queue;
        for (auto* child : typeIdxMap.find(QAO_BASE_TYPE_IDX)->second._childClasses) {
            queue.push_back(child);
        }

        while (!queue.empty()) {
            auto* current = queue.front();
            queue.pop_front();
            ++visitedClassesCount;

            // Inherit message handlers which aren't overriden
            auto& parent = *current->_superclassMetadata;
            for (auto& [typeIndex, untypedMessage] : parent._messageHandlers) {
                (void)current->_messageHandlers.try_emplace(typeIndex, untypedMessage);
            }

            for (auto& child : current->_childClasses) {
                queue.push_back(child);
            }
        }

        HG_HARD_ASSERT(visitedClassesCount == stopz(typeIdxMap.size()));
    }
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

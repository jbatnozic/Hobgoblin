// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_REFLECTION_HPP
#define UHOBGOBLIN_QAO_REFLECTION_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Preprocessor.hpp>
#include <Hobgoblin/QAO/Base.hpp>

#include <gtl/phmap.hpp>

#include <mutex>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

///////////////////////////////////////////////////////////////////////////
// MARK: MESSAGING                                                       //
///////////////////////////////////////////////////////////////////////////

namespace qao_detail {
using QAO_UntypedMessage = void (*)(QAO_Base&, void*);
} // namespace qao_detail

template <class taMessage>
bool QAO_SendMessage(QAO_Base& aReceiverInstance, typename taMessage::Payload* aMessagePayloadPtr);

#define QAO_DEFINE_MESSAGE(_message_name_, _payload_type_)                                            \
    class _message_name_ {                                                                            \
    public:                                                                                           \
        using Payload = _payload_type_;                                                               \
                                                                                                      \
    private:                                                                                          \
        using QAO_Base              = ::jbatnozic::hobgoblin::qao::QAO_Base;                          \
        using QAO_ClassMetadata     = ::jbatnozic::hobgoblin::qao::QAO_ClassMetadata;                 \
        using QAO_MessageHandlerMap = ::jbatnozic::hobgoblin::qao::qao_detail::QAO_MessageHandlerMap; \
        template <class taMessage>                                                                    \
        friend bool ::jbatnozic::hobgoblin::qao::QAO_SendMessage(                                     \
            QAO_Base&                    aReceiverInstance,                                           \
            typename taMessage::Payload* aMessagePayloadPtr);                                         \
        inline static QAO_MessageHandlerMap& _getMessageHandlerMap() {                                \
            static QAO_MessageHandlerMap map{                                                         \
                static_cast<std::size_t>(QAO_ClassMetadata::getClassCount())};                        \
            return map;                                                                               \
        }                                                                                             \
    }

template <class taMessage>
bool QAO_SendMessage(QAO_Base& aInstance, Nullable<typename taMessage::Payload*> aPayload);

///////////////////////////////////////////////////////////////////////////
// MARK: CLASS METADATA                                                  //
///////////////////////////////////////////////////////////////////////////

class QAO_ClassMetadata {
public:
    ///////////////////////////////////////////////////////////////////////////
    // CLASS METHODS                                                         //
    ///////////////////////////////////////////////////////////////////////////

    static const QAO_ClassMetadata* get(std::string_view aUniqueName);
    static const QAO_ClassMetadata* get(std::type_index aTypeIndex);
    static const QAO_ClassMetadata* get(const std::type_info& aTypeInfo);
    static PZInteger                getClassCount();

    ///////////////////////////////////////////////////////////////////////////
    // INSTANCE METHODS                                                      //
    ///////////////////////////////////////////////////////////////////////////

    template <class taSuperclass>
    QAO_ClassMetadata& setSuperclass() {
        return SELF;
    }

    template <class taReceiver, class taMessage, auto taMethod>
    QAO_ClassMetadata& setMessageHandler() {
        _messageHandlers[std::type_index{typeid(taMessage)}] = [](QAO_Base& aInstance, void* aPayload) {
            (static_cast<taReceiver&>(aInstance).*taMethod)((typename taMessage::Payload*)aPayload);
        };
        return SELF;
    }

    // private:
    std::unordered_map<std::type_index, qao_detail::QAO_UntypedMessage> _messageHandlers;
};

using QAO_TypeIndexToClassMetadataMap  = std::unordered_map<std::type_index, QAO_ClassMetadata>;
using QAO_UniqueNameToClassMetadataMap = std::unordered_map<std::string_view, const QAO_ClassMetadata&>;

const QAO_TypeIndexToClassMetadataMap&  QAO_GetAllTypeIndexToClassMetadataMappings();
const QAO_UniqueNameToClassMetadataMap& QAO_GetAllUniqueNameToClassMetadataMappings();

//! TODO(add description)
#define QAO_REGISTER_CLASS(_actual_type_, _unique_name_)                    \
    /* Defer through `HG_PP_INVOKE` so `__LINE__` gets expanded properly */ \
    HG_PP_INVOKE(UHOBGOBLIN_QAO_REGISTER_CLASS_IMPL, _actual_type_, _unique_name_, __LINE__)

//! TODO(add description)
#define QAO_LOCAL_ALIAS(_class_type_, _class_metadata_object_)                                 \
    using _class_type_            = std::remove_pointer_t<decltype(UHOBGOBLIN_QAO_dummy_ptr)>; \
    auto& _class_metadata_object_ = UHOBGOBLIN_QAO_classmd_ref

///////////////////////////////////////////////////////////////////////////
// MARK: IMPLEMENTATION DETAILS                                          //
///////////////////////////////////////////////////////////////////////////

#define UHOBGOBLIN_QAO_REGISTER_CLASS_IMPL(_type_, _name_, _ln_)                                     \
    void UHOBGOBLIN_QAO_RegisterClassUserFunc_##_name_##_##_ln_(                                     \
        ::jbatnozic::hobgoblin::qao::QAO_ClassMetadata&,                                             \
        _type_*);                                                                                    \
    struct UHOBGOBLIN_QAO_RegisterClassImpl_##_name_##_##_ln_ {                                      \
        explicit inline UHOBGOBLIN_QAO_RegisterClassImpl_##_name_##_##_ln_() {                       \
            static constexpr const char* UNIQUE_NAME = #_name_;                                      \
            auto&                        klass =                                                     \
                ::jbatnozic::hobgoblin::qao::qao_detail::QAO_RegisterClass<_type_>(UNIQUE_NAME);     \
            UHOBGOBLIN_QAO_RegisterClassUserFunc_##_name_##_##_ln_(klass, (_type_*)0);               \
        }                                                                                            \
    };                                                                                               \
    inline UHOBGOBLIN_QAO_RegisterClassImpl_##_name_##_##_ln_                                        \
                UHOBGOBLIN_QAO_RegisterClassInstance_##_name_##_##_ln_{};                            \
    inline void UHOBGOBLIN_QAO_RegisterClassUserFunc_##_name_##_##_ln_(                              \
        [[maybe_unused]] ::jbatnozic::hobgoblin::qao::QAO_ClassMetadata& UHOBGOBLIN_QAO_classmd_ref, \
        [[maybe_unused]] _type_*                                         UHOBGOBLIN_QAO_dummy_ptr)

namespace qao_detail {

QAO_ClassMetadata& QAO_RegisterClass(const std::type_info& aTypeInfo, std::string_view aUniqueName);

template <class T>
QAO_ClassMetadata& QAO_RegisterClass(std::string_view aUniqueName) {
    return QAO_RegisterClass(typeid(T), aUniqueName);
}

using QAO_MessageHandlerMap =
    gtl::parallel_flat_hash_map<std::type_index,
                                QAO_UntypedMessage,
                                gtl::priv::hash_default_hash<std::type_index>,
                                gtl::priv::hash_default_eq<std::type_index>,
                                std::allocator<std::pair<std::type_index, QAO_UntypedMessage>>,
                                4,
                                std::recursive_mutex>;

bool QAO_SendMessage(QAO_MessageHandlerMap& aMessageHandlerMap,
                     std::type_index        aMessageTypeIndex,
                     QAO_Base&              aReceiverInstance,
                     void*                  aMessagePayloadPtr);

} // namespace qao_detail

template <class taMessage>
bool QAO_SendMessage(QAO_Base& aReceiverInstance, typename taMessage::Payload* aMessagePayloadPtr) {
    return qao_detail::QAO_SendMessage(taMessage::_getMessageHandlerMap(),
                                       std::type_index{typeid(taMessage)},
                                       aReceiverInstance,
                                       (void*)aMessagePayloadPtr);
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_REFLECTION_HPP

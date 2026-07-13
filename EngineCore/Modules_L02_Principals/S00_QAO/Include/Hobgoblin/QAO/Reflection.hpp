// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_REFLECTION_HPP
#define UHOBGOBLIN_QAO_REFLECTION_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Preprocessor.hpp>
#include <Hobgoblin/QAO/Base.hpp>

#include <gtl/phmap.hpp>

#include <cassert>
#include <mutex>
#include <span>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

///////////////////////////////////////////////////////////////////////////
// MARK: FORWARD DECLARATIONS                                            //
///////////////////////////////////////////////////////////////////////////

class QAO_ClassMetadata;

namespace qao_detail {
using QAO_UntypedMessage = void (*)(QAO_Base&, void*, bool);

using QAO_UntypedClassMessage = void (*)(const QAO_ClassMetadata&, void*);

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
                     void*                  aMessagePayloadPtr,
                     bool                   aConst);
} // namespace qao_detail

///////////////////////////////////////////////////////////////////////////
// MARK: INIT                                                            //
///////////////////////////////////////////////////////////////////////////

//! \brief Initialize the metadata system.
//!
//! You should call this function as soon as possible after your program enters `main()`.
//! If you don't do so, advanced reflection functionality such as messaging and class child/parent
//! relations will not work correctly.
//!
//! This function is idempotent; calling it again after the first call does nothing.
//!
//! \throws TracedLogicError on failure. Failure occurs when a class can't resolve its superclass
//!         (likely no `setSuperclass` call during its registration).
void QAO_InitializeMetadata();

///////////////////////////////////////////////////////////////////////////
// MARK: MESSAGING                                                       //
///////////////////////////////////////////////////////////////////////////

//! \brief Payload pointer type to use with `QAO_DEFINE_MESSAGE` for messages that carry no payload.
//!
//! Since `QAO_DEFINE_MESSAGE` requires the payload to be a pointer type, use this macro as the
//! payload argument to declare a message that conveys no data (only the fact that it was sent).
//!
//! Example:
//! QAO_DEFINE_MESSAGE(Ping, QAO_NO_PAYLOAD);
#define QAO_NO_PAYLOAD ::std::monostate*

//! \brief Declares a message type that can be sent to QAO instances via `QAO_SendMessage`.
//!
//! Defines a new type named `_message_name_` that acts as a compile-time tag identifying the
//! message when registering handlers and dispatching. This type carries no state and exposes
//!  two member aliases derived from `_payload_ptr_`:
//! - `_message_name_::PayloadPtr` - the pointer type itself (equal to `_payload_ptr_`).
//! - `_message_name_::Payload`    - the pointed-to type (`_payload_ptr_` with the pointer removed).
//!
//! \param _message_name_ the name of the message class to define.
//! \param _payload_ptr_ the pointer type of the payload carried by the message. This MUST be a
//!                      pointer type; use `QAO_NO_PAYLOAD` for messages that carry no data.
//!
//! Example:
//! QAO_DEFINE_MESSAGE(SetPower, const double*); // A message carrying a pointer to a read-only `double`
//! QAO_DEFINE_MESSAGE(AddScore, int*);          // A message carrying a pointer to a mutable `int`
//! QAO_DEFINE_MESSAGE(Ping, QAO_NO_PAYLOAD);    // A message carrying no payload at all
#define QAO_DEFINE_MESSAGE(_message_name_, _payload_ptr_)                                             \
    static_assert(::std::is_pointer_v<_payload_ptr_>, #_payload_ptr_ " is not a pointer type!");      \
    class _message_name_ {                                                                            \
    public:                                                                                           \
        using PayloadPtr = _payload_ptr_;                                                             \
        using Payload    = ::std::remove_pointer_t<_payload_ptr_>;                                    \
                                                                                                      \
    private:                                                                                          \
        using QAO_Base              = ::jbatnozic::hobgoblin::qao::QAO_Base;                          \
        using QAO_ClassMetadata     = ::jbatnozic::hobgoblin::qao::QAO_ClassMetadata;                 \
        using QAO_MessageHandlerMap = ::jbatnozic::hobgoblin::qao::qao_detail::QAO_MessageHandlerMap; \
        template <class taMessage>                                                                    \
        friend bool ::jbatnozic::hobgoblin::qao::QAO_SendMessage(                                     \
            QAO_Base&,                                                                                \
            ::jbatnozic::hobgoblin::Nullable<typename taMessage::PayloadPtr>);                        \
        template <class taMessage>                                                                    \
        friend bool ::jbatnozic::hobgoblin::qao::QAO_SendMessage(                                     \
            const QAO_Base&,                                                                          \
            ::jbatnozic::hobgoblin::Nullable<typename taMessage::PayloadPtr>);                        \
        inline static QAO_MessageHandlerMap& _getMessageHandlerMap() {                                \
            static QAO_MessageHandlerMap map{                                                         \
                static_cast<std::size_t>(QAO_ClassMetadata::getClassCount())};                        \
            return map;                                                                               \
        }                                                                                             \
    }

//! \brief Declares a message type that can be sent to QAO classes via `QAO_SendMessage`.
//!
//! Defines a new type named `_message_name_` that acts as a compile-time tag identifying the
//! message when registering handlers and dispatching. This type carries no state and exposes
//!  two member aliases derived from `_payload_ptr_`:
//! - `_message_name_::PayloadPtr` - the pointer type itself (equal to `_payload_ptr_`).
//! - `_message_name_::Payload`    - the pointed-to type (`_payload_ptr_` with the pointer removed).
//!
//! As static methods are to regular methods in C++, so `QAO_DEFINE_CLASS_MESSAGE` is to
//! `QAO_DEFINE_MESSAGE`. Usage is the same.
//!
//! \param _message_name_ the name of the message class to define.
//! \param _payload_ptr_ the pointer type of the payload carried by the message. This MUST be a
//!                      pointer type; use `QAO_NO_PAYLOAD` for messages that carry no data.
//!
//! Example:
//! QAO_DEFINE_MESSAGE(SetPower, const double*); // A message carrying a pointer to a read-only `double`
//! QAO_DEFINE_MESSAGE(AddScore, int*);          // A message carrying a pointer to a mutable `int`
//! QAO_DEFINE_MESSAGE(Ping, QAO_NO_PAYLOAD);    // A message carrying no payload at all
#define QAO_DEFINE_CLASS_MESSAGE(_message_name_, _payload_ptr_)                                  \
    static_assert(::std::is_pointer_v<_payload_ptr_>, #_payload_ptr_ " is not a pointer type!"); \
    struct _message_name_ {                                                                      \
        using PayloadPtr = _payload_ptr_;                                                        \
        using Payload    = ::std::remove_pointer_t<_payload_ptr_>;                               \
    }

//! \brief Sends a message of type `taMessage` to a single QAO instance.
//!
//! Receivers handle messages based on their dynamic type (same as virtual functions). If no handler
//! was registered for `taMessage` on the receiver's most-derived class (see
//! `QAO_ClassMetadata::setMessageHandler`), then its parent class's handler will be used, and if that
//! also doesn't exist, its parent's handler, and so on. If no handler is found using this process,
//! nothing happens and this function returns `false`.
//!
//! Due to the limitation of the system, message handlers must always be implemented as non-const
//! methods (though QAO objects cannot be truly constant objects, only references to them can). To
//! overcome that, two overloads of this function are provided, distinguished by the constness of
//! the receiver. The handler is always invoked with a `bool aConst` argument telling this: if
//! `aConst` is `true`, the message should not mutate the receiver.
//!
//! \tparam taMessage message type previously declared with `QAO_DEFINE_MESSAGE`.
//!
//! \param aReceiverInstance instance to receive the message.
//! \param aMessagePayloadPtr the payload pointer to pass to the handler (of type
//!                           `taMessage::PayloadPtr`). May be `nullptr` (receiver is expected to handle
//!                           this).
//!
//! \returns `true` if a handler was found and invoked, `false` otherwise.
//!
//! Example:
//! struct MyPayload { ... };
//! QAO_DEFINE_MESSAGE(MyMessage, const MyPayload*);
//! ...
//! MyPayload payload = GetPayload();
//! const bool didSend = QAO_SendMessage<MyMessage>(instanceRef, &payload);
template <class taMessage>
bool QAO_SendMessage(QAO_Base&                                aReceiverInstance,
                     Nullable<typename taMessage::PayloadPtr> aMessagePayloadPtr);

//! \brief Overload of `QAO_SendMessage` for a const receiver.
//!
//! Behaves like the non-const overload, but the registered handler is invoked with `aConst == true`,
//! signalling that it should treat the receiver as read-only and not mutate it.
template <class taMessage>
bool QAO_SendMessage(const QAO_Base&                          aReceiverInstance,
                     Nullable<typename taMessage::PayloadPtr> aMessagePayloadPtr);

//! \brief Send a message of type `taMessage` to a class itself rather than an instance of a class.
//!
//! Unlike 'regular' messages, class messages don't interact with inheritance - the specific class
//! to which a message is sent has to have a registered handler for that message, or nothing will
//! happen.
//!
//! \tparam taMessage message type previously declared with `QAO_DEFINE_CLASS_MESSAGE`.
//!
//! \param aReceiverClass class metadata instance to receive the message. This reference is also
//!                       passed to the handler if one is found.
//! \param aMessagePayloadPtr the payload pointer to pass to the handler (of type
//!                           `taMessage::PayloadPtr`). May be `nullptr` (receiver is expected to
//!                           handle this).
//!
//! \returns `true` if a handler was found and invoked, `false` otherwise.
//!
//! Example:
//! struct MyPayload { ... };
//! QAO_DEFINE_CLASS_MESSAGE(MyMessage, const MyPayload*);
//! ...
//! MyPayload payload = GetPayload();
//! const bool didSend = QAO_SendMessage<MyMessage>(metadataRef, &payload);
template <class taMessage>
bool QAO_SendMessage(const QAO_ClassMetadata&                 aReceiverClass,
                     Nullable<typename taMessage::PayloadPtr> aMessagePayloadPtr);

///////////////////////////////////////////////////////////////////////////
// MARK: CLASS METADATA                                                  //
///////////////////////////////////////////////////////////////////////////

//! \brief Holds the reflection metadata associated with a single QAO class.
//!
//! One `QAO_ClassMetadata` object exists per class registered with `QAO_REGISTER_CLASS`. It records
//! information such as the class's name, superclass, child classes, registered message handlers, etc.
//! Metadata can be looked up at runtime by the class's unique name or by its type.
//!
//! In user code, you should never instantiate an instance of this class manually.
class QAO_ClassMetadata {
public:
    ///////////////////////////////////////////////////////////////////////////
    // CLASS METHODS                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Looks up the metadata of a registered class by its unique name.
    //!
    //! \param aUniqueName the unique name the class was registered with (see `QAO_REGISTER_CLASS`).
    //!
    //! \returns a pointer to the matching metadata, or `nullptr` if no class with that name is
    //!          registered.
    static const QAO_ClassMetadata* get(std::string_view aUniqueName);

    //! \brief Looks up the metadata of a registered class by its type index.
    //!
    //! \param aTypeIndex the `std::type_index` of the registered class.
    //!
    //! \returns a pointer to the matching metadata, or `nullptr` if no such class is registered.
    static const QAO_ClassMetadata* get(std::type_index aTypeIndex);

    //! \brief Looks up the metadata of a registered class by its type info.
    //!
    //! \param aTypeInfo the `std::type_info` of the registered class.
    //!
    //! \returns a pointer to the matching metadata, or `nullptr` if no such class is registered.
    static const QAO_ClassMetadata* get(const std::type_info& aTypeInfo);

    //! \brief Returns the total number of classes registered with `QAO_REGISTER_CLASS`.
    static PZInteger getClassCount();

    ///////////////////////////////////////////////////////////////////////////
    // INSTANCE METHODS                                                      //
    ///////////////////////////////////////////////////////////////////////////

    // Getters

    //! \brief return the unique name of the described class.
    std::string_view getUniqueName() const;

    //! \brief return the type info of the described class.
    const std::type_info& getTypeInfo() const;

    //! \brief return the type index of the returned class.
    std::type_index getTypeIndex() const;

    //! \brief get pointers to the described class's superclass.
    //! \note the returned pointer can be null if the desribed class is `QAO_Base`.
    const QAO_ClassMetadata* getSuperclass() const;

    //! \brief get pointers to the described class's child classes (if any).
    //! \note the pointers in the collection are guaranteed to be non-null.
    std::span<const QAO_ClassMetadata* const> getChildClasses() const;

    // Setters
    // NOTE: All setters return a reference to (*this) so that calls can be chained.

    //! \brief Declares the superclass of the class this metadata describes.
    //!
    //! \tparam taSuperclass the base class of this class.
    //!
    //! \warning setting the superclass for a registered class is MANDATORY!
    //!
    //! Example:
    //! QAO_REGISTER_CLASS(Derived, MyDerived) {
    //!     QAO_LOCAL_ALIAS(C, klass);
    //!     klass.setSuperclass<Base>();
    //! }
    template <class taSuperclass>
    QAO_ClassMetadata& setSuperclass() {
        _superclassTypeInfo = &typeid(taSuperclass);
        return SELF;
    }

    //! \brief Registers a member function as the handler for a given message type on this class.
    //!
    //! When `QAO_SendMessage<taMessage>` is called on an instance of `taReceiver`, the registered
    //! `taMethod` will be invoked with the message payload and a `bool` indicating whether the message
    //! was sent to a const receiver (see description of `QAO_DEFINE_MESSAGE` for more details on this).
    //!
    //! \tparam taReceiver the class receiving the message (the one being registered).
    //! \tparam taMessage the message type (declared with `QAO_DEFINE_MESSAGE`) to handle.
    //! \tparam taMethod pointer to the `taReceiver` member function that handles the message. It must
    //!                  be callable as `(taMessage::PayloadPtr, bool)`: the first argument is the
    //!                  payload pointer, and the second is `true` when the message was sent through
    //!                  the const overload of `QAO_SendMessage` (in which case the handler should not
    //!                  mutate the instance) and `false` otherwise.
    //!
    //! \warning the function being registered as the handler must be a non-const function! (You can try
    //!          making it const, it may even work, but it's not really supported.) The const-correctness
    //!          concept is expressed in an alternative way through the bool parameter.
    //!          Also, it's not recommended to make the handler function virtual. This *will* work,
    //!          but message handlers are already dispatched dynamically, and you'd be paying that cost
    //!          twice.
    //!
    //! Example:
    //! QAO_DEFINE_MESSAGE(SetPower, const double*);
    //!
    //! class MyClass : public QAO_Base {
    //! public:
    //!     void setPower(SetPower::PayloadPtr aPower, bool aConst) {
    //!         if (!aConst) {
    //!             power = *aPower;
    //!         }
    //!     }
    //!     double power = 0.0;
    //! };
    //!
    //! QAO_REGISTER_CLASS(MyClass, MyClass) {
    //!     QAO_LOCAL_ALIAS(C, klass);
    //!     klass.setMessageHandler<C, SetPower, &C::setPower>();
    //! }
    template <class taReceiver, class taMessage, auto taMethod>
    QAO_ClassMetadata& setMessageHandler();

    //! \brief Registers a free function as the handler for a given class message type on this class.
    //!
    //! When `QAO_SendMessage<taMessage>` is called on this class's metadata, the registered `taMethod`
    //! will be invoked with a reference to this metadata and the message payload. Unlike regular
    //! message handlers, class message handlers do not participate in inheritance: the handler must be
    //! registered on the exact class the message is sent to (see `QAO_DEFINE_CLASS_MESSAGE`).
    //!
    //! \tparam taMessage the class message type (declared with `QAO_DEFINE_CLASS_MESSAGE`) to handle.
    //! \tparam taMethod pointer to the function that handles the message. It must be callable as
    //!                  `(const QAO_ClassMetadata&, taMessage::PayloadPtr)`: the first argument is the
    //!                  metadata of the receiving class, and the second is the payload pointer (which
    //!                  can be null, and the receiver is expected to handle this).
    //!
    //! \note it's recommended that the registered function be a static member class of the receiving
    //!       class, but this is not required.
    //!
    //! Example:
    //! QAO_DEFINE_CLASS_MESSAGE(Describe, std::string*);
    //!
    //! class MyClass : public QAO_Base {
    //! public:
    //!     static void describe(const QAO_ClassMetadata& aClass, Describe::PayloadPtr aOut) {
    //!         *aOut = aClass.getUniqueName();
    //!     }
    //! };
    //!
    //! QAO_REGISTER_CLASS(MyClass, MyClass) {
    //!     QAO_LOCAL_ALIAS(C, klass);
    //!     klass.setClassMessageHandler<Describe, &C::describe>();
    //! }
    template <class taMessage, auto taMethod>
    QAO_ClassMetadata& setClassMessageHandler();

private:
    friend void QAO_InitializeMetadata();
    friend bool qao_detail::QAO_SendMessage(qao_detail::QAO_MessageHandlerMap&,
                                            std::type_index,
                                            QAO_Base&,
                                            void*,
                                            bool);
    template <class taMessage>
    friend bool QAO_SendMessage(const QAO_ClassMetadata&                 aReceiverClass,
                                Nullable<typename taMessage::PayloadPtr> aMessagePayloadPtr);

    // Superclass
    const std::type_info*    _superclassTypeInfo = nullptr;
    const QAO_ClassMetadata* _superclassMetadata = nullptr;

    // Self
    const std::type_info&           _selfTypeInfo;
    const std::string_view          _selfUniqueName;
    std::vector<QAO_ClassMetadata*> _childClasses;

    // Messaging
    std::unordered_map<std::type_index, qao_detail::QAO_UntypedMessage>      _messageHandlers;
    std::unordered_map<std::type_index, qao_detail::QAO_UntypedClassMessage> _classMessageHandlers;

public:
    //! \brief Constructor.
    //! \param[in] aTypeInfo type info object of the class this metadata describes.
    //! \param[in] aUniqueName unique name of the class this metadata describes.
    //! \warning DO NOT USE THIS C-TOR FROM USER CODE, IT IS FOR ENGINE USE ONLY!
    QAO_ClassMetadata(const std::type_info& aTypeInfo, const std::string_view aUniqueName)
        : _selfTypeInfo{aTypeInfo}
        , _selfUniqueName{aUniqueName} {}
};

using QAO_TypeIndexToClassMetadataMap  = std::unordered_map<std::type_index, QAO_ClassMetadata>;
using QAO_UniqueNameToClassMetadataMap = std::unordered_map<std::string_view, const QAO_ClassMetadata&>;

//! \brief Returns a const reference to the global map from `std::type_index` to the metadata of every
//!        registered class.
//!
//! Example:
//! for (const auto& [typeIndex, metadata] : QAO_GetAllTypeIndexToClassMetadataMappings()) {
//!     // ... inspect every registered class ...
//! }
const QAO_TypeIndexToClassMetadataMap& QAO_GetAllTypeIndexToClassMetadataMappings();

//! \brief Returns a const reference to the global map from unique class name to the metadata of every
//!        registered class.
//! Example:
//! for (const auto& [name, metadata] : QAO_GetAllUniqueNameToClassMetadataMappings()) {
//!     // ... inspect every registered class by name ...
//! }
const QAO_UniqueNameToClassMetadataMap& QAO_GetAllUniqueNameToClassMetadataMappings();

//! \brief Registers a QAO class for reflection and opens a body in which to configure its metadata.
//!
//! The macro associates `_actual_type_` with the given `_unique_name_` and must be immediately followed
//! by a function body (delimited by {}) in which the class's metadata can be configured (setting the
//! superclass, setting message handlers, etc.). Inside this function body, use `QAO_LOCAL_ALIAS` to
//! access the associated `QAO_ClassMetadata` object.
//!
//! \note No matter if this macro is used in a header or a .cpp file, its body will be executed only
//!       once, automatically, before entering `main()`.
//!
//! \warning setting the superclass for a registered class is MANDATORY!
//!
//! \param _actual_type_ the C++ type being registered.
//! \param _unique_name_ an identifier that uniquely names the class among all registered classes.
//!                      Registering two classes with the same name (or the same type) is an error.
//!
//! Example:
//! class MyClass : public QAO_Base { /* ... */ };
//!
//! QAO_REGISTER_CLASS(MyClass, MyUniqueClassName) {
//!     QAO_LOCAL_ALIAS(C, klass);
//!     klass.setSuperclass<QAO_Base>();
//! }
#define QAO_REGISTER_CLASS(_actual_type_, _unique_name_)                    \
    /* Defer through `HG_PP_INVOKE` so `__LINE__` gets expanded properly */ \
    HG_PP_INVOKE(UHOBGOBLIN_QAO_REGISTER_CLASS_IMPL, _actual_type_, _unique_name_, __LINE__)

//! \brief Introduces convenient local aliases inside a `QAO_REGISTER_CLASS` body.
//!
//! Must be used within the body of a `QAO_REGISTER_CLASS` block. It declares `_class_type_` as a
//! type alias for the class being registered, and `_class_metadata_object_` as a reference to that
//! class's `QAO_ClassMetadata`, so they can be referred to with short, local names.
//!
//! \param _class_type_ name for the local type alias of the class being registered.
//! \param _class_metadata_object_ name for the local reference to the class's metadata object.
//!
//! Example:
//! QAO_REGISTER_CLASS(MyClass, MyClass) {
//!     QAO_LOCAL_ALIAS(C, klass); // `C` == `MyClass`, `klass` == this class's metadata
//!     klass.setSuperclass<QAO_Base>();
//!     klass.setMessageHandler<C, SetPower, &C::setPower>();
//! }
#define QAO_LOCAL_ALIAS(_class_type_, _class_metadata_object_)                                 \
    using _class_type_            = std::remove_pointer_t<decltype(UHOBGOBLIN_QAO_dummy_ptr)>; \
    auto& _class_metadata_object_ = UHOBGOBLIN_QAO_classmd_ref

///////////////////////////////////////////////////////////////////////////
// MARK: IMPLEMENTATION DETAILS                                          //
///////////////////////////////////////////////////////////////////////////

#define UHOBGOBLIN_QAO_REGISTER_CLASS_IMPL(_type_, _name_, _ln_)                                     \
    static_assert(::std::is_base_of_v<::jbatnozic::hobgoblin::qao::QAO_Base, _type_>);               \
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

template <class taReceiver, class taMessage, auto taMethod>
QAO_ClassMetadata& QAO_ClassMetadata::setMessageHandler() {
    assert(typeid(taReceiver) == _selfTypeInfo);
    auto lambda = [](QAO_Base& aInstance, void* aPayload, bool aConst) {
        ((taReceiver&)(aInstance).*taMethod)((typename taMessage::Payload*)aPayload, aConst);
    };
    auto [_, didInsert] =
        _messageHandlers.insert(std::make_pair(std::type_index{typeid(taMessage)}, lambda));
    assert(didInsert && "Cannot have multiple message handlers for the same message type!");
    return SELF;
}

template <class taMessage, auto taMethod>
QAO_ClassMetadata& QAO_ClassMetadata::setClassMessageHandler() {
    auto lambda = [](const QAO_ClassMetadata& aClass, void* aPayload) {
        (taMethod)(aClass, (typename taMessage::Payload*)aPayload);
    };
    auto [_, didInsert] =
        _classMessageHandlers.insert(std::make_pair(std::type_index{typeid(taMessage)}, lambda));
    assert(didInsert && "Cannot have multiple message handlers for the same message type!");
    return SELF;
}

namespace qao_detail {

QAO_ClassMetadata& QAO_RegisterClass(const std::type_info& aTypeInfo, std::string_view aUniqueName);

template <class T>
QAO_ClassMetadata& QAO_RegisterClass(std::string_view aUniqueName) {
    return QAO_RegisterClass(typeid(T), aUniqueName);
}

} // namespace qao_detail

template <class taMessage>
bool QAO_SendMessage(QAO_Base& aReceiverInstance, typename taMessage::PayloadPtr aMessagePayloadPtr) {
    return qao_detail::QAO_SendMessage(taMessage::_getMessageHandlerMap(),
                                       std::type_index{typeid(taMessage)},
                                       aReceiverInstance,
                                       (void*)aMessagePayloadPtr,
                                       false);
}

template <class taMessage>
bool QAO_SendMessage(const QAO_Base&                aReceiverInstance,
                     typename taMessage::PayloadPtr aMessagePayloadPtr) {
    return qao_detail::QAO_SendMessage(taMessage::_getMessageHandlerMap(),
                                       std::type_index{typeid(taMessage)},
                                       const_cast<QAO_Base&>(aReceiverInstance),
                                       (void*)aMessagePayloadPtr,
                                       true);
}

template <class taMessage>
bool QAO_SendMessage(const QAO_ClassMetadata&                 aReceiverClass,
                     Nullable<typename taMessage::PayloadPtr> aMessagePayloadPtr) {
    auto iter = aReceiverClass._classMessageHandlers.find(std::type_index{typeid(taMessage)});
    if (iter == aReceiverClass._classMessageHandlers.end()) {
        return false;
    }
    (iter->second)(aReceiverClass, aMessagePayloadPtr);
    return true;
}

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_REFLECTION_HPP

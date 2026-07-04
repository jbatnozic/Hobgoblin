// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_REFLECTION_HPP
#define UHOBGOBLIN_QAO_REFLECTION_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Preprocessor.hpp>

#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_ClassMetadata {
public:
    ///////////////////////////////////////////////////////////////////////////
    // CLASS METHODS                                                         //
    ///////////////////////////////////////////////////////////////////////////

    static const QAO_ClassMetadata* get(std::string_view aUniqueName);
    static const QAO_ClassMetadata* get(std::type_index aTypeIndex);
    static const QAO_ClassMetadata* get(std::type_info& aTypeInfo);
    static PZInteger                getCount();

    ///////////////////////////////////////////////////////////////////////////
    // INSTANCE METHODS                                                      //
    ///////////////////////////////////////////////////////////////////////////

    template <class taSuperclass>
    QAO_ClassMetadata& setSuperclass() {
        return SELF;
    }

    template <class taReceiver, class taMessage>
    QAO_ClassMetadata& setMessageHandler() {
        return SELF;
    }

private:
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
#define QAO_LOCAL_ALIAS(_class_type_, _class_metadata_object_)                               \
    using _class_type_            = std::remove_cvref_t<decltype(UHOBGOBLIN_QAO_dummy_ref)>; \
    auto& _class_metadata_object_ = UHOBGOBLIN_QAO_classmd_ref

///////////////////////////////////////////////////////////////////////////
// MARK: IMPLEMENTATION DETAILS                                          //
///////////////////////////////////////////////////////////////////////////

#define UHOBGOBLIN_QAO_REGISTER_CLASS_IMPL(_type_, _name_, _ln_)                                     \
    void UHOBGOBLIN_QAO_RegisterClassUserFunc_##_name_##_##_ln_(                                     \
        ::jbatnozic::hobgoblin::qao::QAO_ClassMetadata&,                                             \
        const _type_&);                                                                              \
    struct UHOBGOBLIN_QAO_RegisterClassImpl_##_name_##_##_ln_ {                                      \
        explicit inline UHOBGOBLIN_QAO_RegisterClassImpl_##_name_##_##_ln_() {                       \
            static constexpr const char* UNIQUE_NAME = #_name_;                                      \
            auto&                        klass =                                                     \
                ::jbatnozic::hobgoblin::qao::qao_detail::QAO_RegisterClass<_type_>(UNIQUE_NAME);     \
            UHOBGOBLIN_QAO_RegisterClassUserFunc_##_name_##_##_ln_(                                  \
                klass,                                                                               \
                *reinterpret_cast<_type_*>(0x12345678));                                             \
        }                                                                                            \
    };                                                                                               \
    inline UHOBGOBLIN_QAO_RegisterClassImpl_##_name_##_##_ln_                                        \
                UHOBGOBLIN_QAO_RegisterClassInstance_##_name_##_##_ln_{};                            \
    inline void UHOBGOBLIN_QAO_RegisterClassUserFunc_##_name_##_##_ln_(                              \
        [[maybe_unused]] ::jbatnozic::hobgoblin::qao::QAO_ClassMetadata& UHOBGOBLIN_QAO_classmd_ref, \
        [[maybe_unused]] const _type_&                                   UHOBGOBLIN_QAO_dummy_ref)

namespace qao_detail {

QAO_ClassMetadata& QAO_RegisterClass(const std::type_info& aTypeInfo, std::string_view aUniqueName);

template <class T>
QAO_ClassMetadata& QAO_RegisterClass(std::string_view aUniqueName) {
    return QAO_RegisterClass(typeid(T), aUniqueName);
}

} // namespace qao_detail

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_REFLECTION_HPP

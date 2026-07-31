// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_NAME_REF_HPP
#define UHOBGOBLIN_QAO_NAME_REF_HPP

#include <Hobgoblin/Common/Nullability.hpp>

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_NameRef {
public:
    QAO_NameRef(const QAO_NameRef&) = default;

    QAO_NameRef(NeverNull<const char*> aString, bool aStatic = false)
        : string{aString}
        , stringLength{static_cast<decltype(stringLength)>(std::strlen(string))}
        , stringIsStatic{aStatic} {}

    QAO_NameRef(const char* aString, bool aStatic = false)
        : QAO_NameRef(MakeNeverNull(aString), aStatic) {}

    QAO_NameRef(const std::string& aString, bool aStatic = false)
        : string{aString.c_str()}
        , stringLength{static_cast<decltype(stringLength)>(aString.length())}
        , stringIsStatic{aStatic} {}

    QAO_NameRef(std::string_view aStringView, bool aStatic = false)
        : string{aStringView.data()}
        , stringLength{static_cast<decltype(stringLength)>(aStringView.length())}
        , stringIsStatic{aStatic} {}

    const char*   string         = nullptr;
    std::uint32_t stringLength   = 0;
    bool          stringIsStatic = false;
};

#define QAO_STATIC_NAME(_string_literal_) \
    (::jbatnozic::hobgoblin::qao::QAO_NameRef{_string_literal_, true})

#define QAO_DEFAULT_NAME QAO_STATIC_NAME("<<n/a>>")

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_NAME_REF_HPP

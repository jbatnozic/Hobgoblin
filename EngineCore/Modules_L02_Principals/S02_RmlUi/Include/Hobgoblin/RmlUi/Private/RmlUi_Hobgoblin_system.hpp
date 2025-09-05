// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_HOBGOBLIN_SYSTEM_HPP
#define UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_HOBGOBLIN_SYSTEM_HPP

#include <Hobgoblin/Utility/Time_utils.hpp>
#include <RmlUi/Core/SystemInterface.h>

#include <Hobgoblin/Private/Pmacro_define.hpp>
HOBGOBLIN_NAMESPACE_BEGIN
namespace rml {
namespace detail {

class RmlUiHobgoblinSystem : public Rml::SystemInterface {
public:
    ~RmlUiHobgoblinSystem() override;

    double GetElapsedTime() override;

    bool LogMessage(Rml::Log::Type aType, const Rml::String& aMessage) override;

    void SetClipboardText(const Rml::String& aText) override;

    void GetClipboardText(Rml::String& aText) override;

private:
    util::Stopwatch _clock;
};

} // namespace detail
} // namespace rml
HOBGOBLIN_NAMESPACE_END
#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_HOBGOBLIN_SYSTEM_HPP

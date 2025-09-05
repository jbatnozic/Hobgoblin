// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common/Build_type.hpp>
#include <Hobgoblin/HGExcept.hpp>

#include "SFML/System_impl.hpp"

#include <cstring>

#if HG_BUILD_TYPE == HG_DEBUG
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Font.hpp>
#include <Hobgoblin/UWGA/Image.hpp>
#include <Hobgoblin/UWGA/Private/Draw_batching_decorator_fnl.hpp>
#include <Hobgoblin/UWGA/Private/Draw_batching_decorator_strict.hpp>
#include <Hobgoblin/UWGA/Private/Draw_batching_utils.hpp>
#endif

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

std::shared_ptr<System> CreateGraphicsSystem(System::Provider aProvider) {
    switch (aProvider) {
    case System::Provider::SFML:
        return std::make_shared<SFMLSystemImpl>();
    }

    HG_THROW_TRACED(TracedLogicError,
                    0,
                    "Graphics system provider with designator '{}' does not exist.",
                    (int)aProvider);
}

std::shared_ptr<System> CreateGraphicsSystem(const char* aSystemProviderName) {
    if (std::strcmp(aSystemProviderName, "SFML") == 0) {
        return std::make_shared<SFMLSystemImpl>();
    }

    HG_THROW_TRACED(TracedLogicError,
                    0,
                    "Graphics system provider named '{}' does not exist.",
                    aSystemProviderName);
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

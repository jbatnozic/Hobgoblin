// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Stream_output.hpp>

#include <Hobgoblin/Logging.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

namespace {
constexpr auto LOG_ID = "Hobgoblin.Utility";
} // namespace

void OutputStream::_logAppendingError(const char* aErrorMessage) {
    HG_LOG_WARN(LOG_ID,
                "Exception caught while appending data to hg::util::OutputStream in "
                "no-throw mode: {}",
                aErrorMessage);
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

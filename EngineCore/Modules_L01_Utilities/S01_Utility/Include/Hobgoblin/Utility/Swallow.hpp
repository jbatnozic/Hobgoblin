// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UTIL_SWALLOW_HPP
#define UHOBGOBLIN_UTIL_SWALLOW_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>

#include <exception>
#include <source_location>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

//! \brief Call the provided callable object and swallow all exceptions that occur.
//!
//! If an exception is indeed caught (and swallowed), an error message is logged with the details.
template <class taCallable>
void Swallow(taCallable&& aCallable, std::source_location aSrcLoc = std::source_location::current()) {
    static constexpr auto LOG_ID = "Hobgoblin.Utility";
    try {
        aCallable();
    } catch (const TracedException& e) {
        HG_LOG_ERROR(LOG_ID,
                     "Traced exception swallowed at {}@{}:{}! Details: {}",
                     aSrcLoc.function_name(),
                     aSrcLoc.file_name(),
                     aSrcLoc.line(),
                     e.getFullFormattedDescription());
    } catch (const std::exception& e) {
        HG_LOG_ERROR(LOG_ID,
                     "Standard exception swallowed at {}@{}:{}! Details: {}",
                     aSrcLoc.function_name(),
                     aSrcLoc.file_name(),
                     aSrcLoc.line(),
                     e.what());
    } catch (...) {
        HG_LOG_ERROR(LOG_ID,
                     "Unknown exception swallowed at {}@{}:{}! Details: n/a",
                     aSrcLoc.function_name(),
                     aSrcLoc.file_name(),
                     aSrcLoc.line());
    }
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UTIL_SWALLOW_HPP

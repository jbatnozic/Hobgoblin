#ifndef UHOBGOBLIN_LOGGING_OBSERVERS_HPP
#define UHOBGOBLIN_LOGGING_OBSERVERS_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace log {

HG_DYNAPI
class LoggingObserverInterface {
public:
    //! IMPORTANT: When inheriting this method, make the calling convention HGCALL!
    virtual HGCALL ~LoggingObserverInterface() = default;

    //! IMPORTANT: When inheriting this method, make the calling convention HGCALL!
    virtual void HGCALL onLog(Severity aSeverity,
                              const char* aLogId, 
                              const std::string& aMessage,
                              const std::string& aFullOutput) = 0;
};

//! TODO(description)
HG_DYNAPI void HGCALL RegisterLoggingObserver(LoggingObserverInterface& aObserver);

//! TODO(description)
HG_DYNAPI void HGCALL UnregisterLoggingObserver(LoggingObserverInterface& aObserver);

} // namespace log
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_LOGGING_OBSERVERS_HPP

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Time_utils.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Utility/Semaphore.hpp>

#include <cassert>
#include <cstdint>
#include <thread>

// clang-format off
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Mmsystem.h>
    #include <Windows.h>
#else
    #include <errno.h>
    #include <time.h>
    #ifdef __APPLE__
        #include <dispatch/dispatch.h>
        #include <mach/clock.h>
        #include <mach/mach.h>
    #endif // __APPLE__
#endif // _WIN32
// clang-format on

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

void Sleep(milliseconds timeToSleep) {
    std::this_thread::sleep_for(timeToSleep);
}

#if defined(_WIN32)
// ****************** WINDOWS ******************

namespace detail {
void CALLBACK SignallingTimerCallback(UINT timerID,
                                      UINT /* reserved */,
                                      DWORD_PTR userData,
                                      DWORD_PTR /* reserved */,
                                      DWORD_PTR /* reserved */) {
    static_assert(sizeof(DWORD_PTR) == sizeof(void*));
    auto* sem = reinterpret_cast<Semaphore*>(userData);
    sem->signal();
}
} // namespace detail

void PreciseSleep(milliseconds timeToSleep) {
    if (timeToSleep.count() <= 0) {
        return;
    }

#ifndef NDEBUG
    TIMECAPS tc;
    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) {
        assert(false && "Win32 API call 'timeGetDevCaps()' failed");
    }
    assert(tc.wPeriodMin == 1);
#endif // !NDEBUG

    Semaphore sem{0};
    auto      mmresult = timeSetEvent(timeToSleep.count(),
                                 1 /* minimal resolution in milliseconds */,
                                 &detail::SignallingTimerCallback,
                                 reinterpret_cast<DWORD_PTR>(&sem),
                                 TIME_CALLBACK_FUNCTION);
    if (mmresult == NULL) {
        HG_THROW_TRACED(TracedRuntimeError, 0, "Windows Multimedia timer could not be initialized.");
    }
    sem.wait();
}

#elif defined(__APPLE__)
// ****************** MAC ******************

void PreciseSleep(milliseconds aTimeToSleep) {
    dispatch_queue_t queue = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);
    if (queue == nullptr) {
        return;
    }

    dispatch_source_t timer =
        dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, queue);
    if (timer == nullptr) {
        return;
    }

    dispatch_source_set_timer(
        timer,
        dispatch_time(DISPATCH_TIME_NOW, duration_cast<nanoseconds>(aTimeToSleep).count()),
        DISPATCH_TIME_FOREVER,
        0 /* leeway 0 = high precision timer */);

    Semaphore sem;
    auto*     semPtr = &sem;
    dispatch_source_set_event_handler(timer, ^{
      semPtr->signal();
      dispatch_source_cancel(timer); // prevent further calls of this event handler
    });

    dispatch_resume(timer); // start the timer

    sem.wait();

    dispatch_release(timer);
}

#else
// ****************** UNIX ******************

void PreciseSleep(milliseconds timeToSleep) {
    static constexpr auto MILLISECONDS_PER_SECOND     = 1'000LL;
    static constexpr auto NANOSECONDS_PER_MILLISECOND = 1'000'000LL;

    struct timespec ts;
    ts.tv_sec = static_cast<decltype(ts.tv_sec)>(timeToSleep.count() / MILLISECONDS_PER_SECOND);
    ts.tv_nsec =
        static_cast<decltype(ts.tv_nsec)>((timeToSleep.count() % 1'000) * NANOSECONDS_PER_MILLISECOND);

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
}

#endif

void SuperPreciseSleep(microseconds aTimeToSleep) {
    if (aTimeToSleep.count() <= 0) {
        return;
    }

    static constexpr microseconds PRECISE_SLEEP_GRANULARITY{1000};

    Stopwatch stopwatch{};

    if (aTimeToSleep > PRECISE_SLEEP_GRANULARITY) {
        PreciseSleep(duration_cast<milliseconds>(aTimeToSleep - PRECISE_SLEEP_GRANULARITY));
    }

    while (stopwatch.getElapsedTime<std::chrono::microseconds>() < aTimeToSleep) {
        // Busy wait for the remainder of the time. yield() provides a hint to the
        // implementation to reschedule the execution of threads, allowing other
        // threads to run.
        std::this_thread::yield();
    }
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

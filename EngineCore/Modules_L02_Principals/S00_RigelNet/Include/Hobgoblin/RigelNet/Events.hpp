// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_RN_EVENTS_HPP
#define UHOBGOBLIN_RN_EVENTS_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Utility/Visitor.hpp>

#include <deque>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace rn {

struct RN_Event {

    struct BadPassphrase {
        std::optional<PZInteger> clientIndex; // Not used on client side
        std::string incorrectPassphrase;
    };

    struct ConnectAttemptFailed {
        enum class Reason {
            Error, TimedOut
        };
        Reason reason;
    };

    struct Connected {
        std::optional<PZInteger> clientIndex; // Not used on client side
    };

    struct Disconnected {
        enum class Reason {
            Graceful, Error, TimedOut
        };
        std::optional<PZInteger> clientIndex; // Not used on client side
        Reason reason;
        std::string message;
    };

    using EventVariant = std::variant<
        BadPassphrase,
        ConnectAttemptFailed,
        Connected,
        Disconnected>;

    RN_Event() = default;

    template <class T>
    RN_Event(T&& arg)
        : eventVariant{std::forward<T>(arg)}
    {
    }

    std::optional<EventVariant> eventVariant;

    template <class ...Callables>
    void visit(Callables&&... callables) {
        std::visit(
            util::MakeVisitor([](const auto&) {}, std::forward<Callables>(callables)...),
            eventVariant.value()
        );
    }

    template <class ...Callables>
    void visit(Callables&&... callables) const {
        std::visit(
            util::MakeVisitor([](const auto&) {}, std::forward<Callables>(callables)...),
            eventVariant.value()
        );
    }

    //! Unlike visit, a call to strictVisit will not compile unless a
    //! matching callable is provided for each event type.
    template <class ...Callables>
    void strictVisit(Callables&&... callables) {
        std::visit(util::MakeVisitor(std::forward<Callables>(callables)...), eventVariant.value());
    }

    //! Unlike visit, a call to strictVisit will not compile unless a
    //! matching callable is provided for each event type.
    template <class ...Callables>
    void strictVisit(Callables&&... callables) const {
        std::visit(util::MakeVisitor(std::forward<Callables>(callables)...), eventVariant.value());
    }
};

namespace rn_detail {
    
class EventFactory {
public:
    explicit EventFactory(std::deque<RN_Event>& eventQueue);
    explicit EventFactory(std::deque<RN_Event>& eventQueue, PZInteger clientIndex);

    void createBadPassphrase(std::string incorrectPassphrase) const;
    void createConnectAttemptFailed(RN_Event::ConnectAttemptFailed::Reason reason) const;
    void createConnected() const;
    void createDisconnected(RN_Event::Disconnected::Reason reason, std::string message) const;

private:
    std::deque<RN_Event>& _eventQueue;
    std::optional<PZInteger> _clientIndex;
};

} // namespace rn_detail

} // namespace rn
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RN_EVENTS_HPP

// clang-format on

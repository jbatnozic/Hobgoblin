// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef SPEMPE_SERVICES_AUTHORIZATION_SERVICE_HPP
#define SPEMPE_SERVICES_AUTHORIZATION_SERVICE_HPP

#include <SPeMPE/GameContext/Context_components.hpp>

#include <optional>
#include <string>

namespace jbatnozic {
namespace spempe {

// Possible strategies:
//  - first connected client (default)
//  - client from lobby by unique ID
//  - by slot
//  - ?

using AuthToken = std::string;

class AuthorizationService : public ContextComponent {
public:
    ~AuthorizationService() override = default;

    enum class Mode {
        Uninitialized,
        Host,
        Client
    };

    virtual void setToHostMode(/* TODO: provide auth strategy*/) = 0;

    virtual void setToClientMode() = 0;

    virtual Mode getMode() const = 0;

    virtual std::optional<AuthToken> getLocalAuthToken() = 0;

private:
    SPEMPE_CTXCOMP_TAG("jbatnozic::spempe::AuthorizationService");
};


} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_SERVICES_AUTHORIZATION_SERVICE_HPP

// clang-format on

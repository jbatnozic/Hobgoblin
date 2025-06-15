// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef SPEMPE_MANAGERS_AUTHORIZATION_MANAGER_DEFAULT_HPP
#define SPEMPE_MANAGERS_AUTHORIZATION_MANAGER_DEFAULT_HPP

#include <Hobgoblin/QAO.hpp>
#include <SPeMPE/GameObjectFramework/Game_object_bases.hpp>
#include <SPeMPE/Managers/Authorization_manager_interface.hpp>
#include <SPeMPE/Managers/Networking_manager_interface.hpp>
#include <SPeMPE/Managers/Synced_varmap_manager_interface.hpp>

#include <functional>

namespace jbatnozic {
namespace spempe {

namespace detail {
struct PlayerInfoWithIndex {
    std::string name;
    std::string uniqueId;
    std::string ipAddress;

    int clientIndex = CLIENT_INDEX_UNKNOWN;

    friend bool operator==(const PlayerInfoWithIndex& aRhs, 
                           const PlayerInfoWithIndex& aLhs);
};
} // namespace detail

class DefaultAuthorizationManager
    : public AuthorizationManagerInterface
    , public NonstateObject
{
public:
    DefaultAuthorizationManager(hobgoblin::QAO_IKey aIKey, int aExecutionPriority);

    ~DefaultAuthorizationManager() override;

    void setToHostMode(/* TODO: provide auth strategy*/) override;

    void setToClientMode() override;

    Mode getMode() const override;

    std::optional<AuthToken> getLocalAuthToken() override;
private:
    Mode _mode = Mode::Uninitialized;
    std::optional<AuthToken> _localAuthToken;

    detail::PlayerInfoWithIndex _currentAuthorizedPlayer;

    void _eventBeginUpdate() override;

    bool _hasCurrentlyAuthorizedPlayer() const;
    void _authorizePlayer(
        const detail::PlayerInfoWithIndex& aPlayerToAuthorize,
        NetworkingManagerInterface& aNetMgr,
        SyncedVarmapManagerInterface& aSvmMgr
    );

    friend void USPEMPE_DefaultAuthorizationManager_SetLocalAuthToken(
        DefaultAuthorizationManager& aAuthMgr,
        const AuthToken& aToken
    );
};

namespace detail {
inline
bool operator==(const PlayerInfoWithIndex& aRhs,
                const PlayerInfoWithIndex& aLhs) {
    return aLhs.name == aRhs.name
        && aLhs.uniqueId == aRhs.uniqueId
        && aLhs.ipAddress == aRhs.ipAddress
        && aLhs.clientIndex == aRhs.clientIndex;
}
} // namespace detail

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_MANAGERS_AUTHORIZATION_MANAGER_DEFAULT_HPP

// clang-format on

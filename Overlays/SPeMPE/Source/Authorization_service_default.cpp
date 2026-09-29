// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off


#include <SPeMPE/Services/Authorization_service_default.hpp>

#include <SPeMPE/Services/Lobby_backend_service.hpp>
#include <SPeMPE/Services/Networking_service.hpp>
#include <SPeMPE/Utility/Rpc_receiver_context_template.hpp>

#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Utility/Hash.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <functional>
#include <unordered_set>

#include <Hobgoblin/RigelNet_macros.hpp>

namespace jbatnozic {
namespace spempe {

namespace hg = hobgoblin;
using namespace hg::rn;

namespace {

constexpr auto LOG_ID = "MultiplayerFoundation";

std::string GenerateRandomString(hg::PZInteger aStringLength) {
    return hg::util::DoWith32bitRNG(
        [=](std::mt19937& aRNG) {
            std::uniform_int_distribution<int> dist{33, 126};
            std::string result;
            result.resize(hg::pztos(aStringLength), ' ');
            for (hg::PZInteger i = 0; i < aStringLength; i += 1) {
                result[hg::pztos(i)] = static_cast<char>(dist(aRNG));
            }
            return result;
        });
}

} // namespace
} // namespace spempe
} // namespace jbatnozic

namespace std {
#define PInfo jbatnozic::spempe::detail::PlayerInfoWithIndex
template <>
struct hash<PInfo> {
    size_t operator()(const PInfo& aInfo) const {
        using jbatnozic::hobgoblin::util::CalcHashAndCombine;
        size_t result = 0;
        result = CalcHashAndCombine(aInfo.name,        result);
        result = CalcHashAndCombine(aInfo.uniqueId,    result);
        result = CalcHashAndCombine(aInfo.ipAddress,   result);
        result = CalcHashAndCombine(aInfo.clientIndex, result);
        return result;
    }
};
#undef PInfo
} // namespace std

namespace jbatnozic {
namespace spempe {
namespace {

using PlayerInfoWISet = std::unordered_set<detail::PlayerInfoWithIndex>;

PlayerInfoWISet ScanLobbyServiceForAllCurrentlyConnectedPlayers(LobbyBackendService& aLobbySvc) {
    PlayerInfoWISet result;
    for (hg::PZInteger i = 0; i < aLobbySvc.getSize(); i += 1) {
        const auto& playerInfo = aLobbySvc.getPendingPlayerInfo(i);
        if (playerInfo.isComplete()) {
            result.insert({
                playerInfo.name,
                playerInfo.uniqueId,
                playerInfo.ipAddress,
                aLobbySvc.clientIdxOfPlayerInPendingSlot(i)
            });
        }
    }
    return result;
}
} // namespace

void USPEMPE_DefaultAuthorizationService_SetLocalAuthToken(
    DefaultAuthorizationService& aAuthSvc,
    const AuthToken& aToken
) {
    aAuthSvc._localAuthToken = aToken;
    HG_LOG_INFO(LOG_ID, "Local node authorized by the host.");
}

RN_DEFINE_RPC(SetLocalAuthToken, RN_ARGS(AuthToken&, aToken)) {
    RN_NODE_IN_HANDLER().callIfClient(
        [&](RN_ClientInterface& aClient) {
            const auto rc = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aClient);
            auto& authSvc = dynamic_cast<DefaultAuthorizationService&>(
                rc.gameContext.getComponent<AuthorizationService>()
            );
            USPEMPE_DefaultAuthorizationService_SetLocalAuthToken(authSvc, aToken);
        });
    RN_NODE_IN_HANDLER().callIfServer(
        [&](RN_ServerInterface&) {
            throw RN_IllegalMessage();
        });
}

#define AUTH_TOKEN_LENGTH 50

DefaultAuthorizationService::DefaultAuthorizationService(hobgoblin::QAO_InstGuard aInstGuard,
                                                         int aExecutionPriority)
    : NonstateObject(aInstGuard,
                     hg::QAO_ExeCon::INTERACTIVITY,
                     aExecutionPriority,
                     QAO_STATIC_NAME("jbatnozic::spempe::DefaultAuthorizationService")) {}

DefaultAuthorizationService::~DefaultAuthorizationService() = default;

void DefaultAuthorizationService::setToHostMode(/* TODO: provide auth strategy*/) {
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged==true, networking==true);
    _mode = Mode::Host;
    _localAuthToken = "<placeholder-token>";
}


void DefaultAuthorizationService::setToClientMode() {
    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged==false, networking==true);
    _mode = Mode::Client; // TODO
}

DefaultAuthorizationService::Mode DefaultAuthorizationService::getMode() const {
    return _mode;
}

std::optional<AuthToken> DefaultAuthorizationService::getLocalAuthToken() {
    return _localAuthToken;
}

void DefaultAuthorizationService::_eventBeginUpdate() {
    // TODO - Temporary implementation
    if (_mode != Mode::Host) {
        return;
    }

    auto& aLobbySvc = ccomp<LobbyBackendService>();
    const auto players = ScanLobbyServiceForAllCurrentlyConnectedPlayers(aLobbySvc);

    if (_hasCurrentlyAuthorizedPlayer()) {
        if (players.find(_currentAuthorizedPlayer) == players.end()) {
            _currentAuthorizedPlayer.clientIndex = CLIENT_INDEX_UNKNOWN;

            // Authorized player disconnected; authorize another one if able
            for (const auto& player : players) {
                // We never want to authorize the local player (they always
                // have full privileges to do anything they want)
                if (player.clientIndex != CLIENT_INDEX_LOCAL) {
                    _localAuthToken = GenerateRandomString(AUTH_TOKEN_LENGTH);
                    _authorizePlayer(
                        player,
                        ccomp<NetworkingService>(),
                        ccomp<SyncedVarmapService>()
                    );
                    break;
                }
            }
        }
    }
    else {
        if (!players.empty()) {
            for (const auto& player : players) {
                // We never want to authorize the local player (they always
                // have full privileges to do anything they want)
                if (player.clientIndex != CLIENT_INDEX_LOCAL) {
                    _localAuthToken = GenerateRandomString(AUTH_TOKEN_LENGTH);
                    _authorizePlayer(
                        player,
                        ccomp<NetworkingService>(),
                        ccomp<SyncedVarmapService>()
                    );
                    break;
                }
            }
        }
    }
}

bool DefaultAuthorizationService::_hasCurrentlyAuthorizedPlayer() const {
    return _currentAuthorizedPlayer.clientIndex != CLIENT_INDEX_UNKNOWN;
}

void DefaultAuthorizationService::_authorizePlayer(
    const detail::PlayerInfoWithIndex& aPlayerToAuthorize,
    NetworkingService& aNetSvc,
    SyncedVarmapService& aSvmSvc
) {
    _currentAuthorizedPlayer = aPlayerToAuthorize;
    Compose_SetLocalAuthToken(aNetSvc.getNode(), aPlayerToAuthorize.clientIndex, *_localAuthToken);
    // TODO set varmap values...
    HG_LOG_INFO(LOG_ID, "Authorized player {} ({}).", aPlayerToAuthorize.name, aPlayerToAuthorize.ipAddress);
}

} // namespace spempe
} // namespace jbatnozic

// clang-format on

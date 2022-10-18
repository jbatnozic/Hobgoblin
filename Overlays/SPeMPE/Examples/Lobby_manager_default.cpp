
#include "Lobby_manager_default.hpp"

#include <cassert>
#include <sstream>
#include <utility>

#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/RigelNet_macros.hpp>
#include <SPeMPE/Managers/Networking_manager_interface.hpp>
#include <SPeMPE/Managers/Synced_varmap_manager_interface.hpp>
#include <SPeMPE/Managers/Window_manager_interface.hpp>
#include <SFML/Graphics.hpp>

namespace jbatnozic {
namespace spempe {

namespace hg = hobgoblin;
using namespace hg::rn;

namespace {
constexpr auto LOG_ID = "::jbatnozic::spempe::DefaultLobbyManager";

constexpr auto CUSTOM_DATA_LEN = hg::stopz(std::tuple_size_v<decltype(PlayerInfo::customData)>);

bool IsConnected(const RN_ConnectorInterface& client) {
    return (client.getStatus() == RN_ConnectorStatus::Connected);
}

const std::string& MakeVarmapKey_LobbySize() {
    static std::string KEY = "::jbatnozic::spempe::DefaultLobbyManager_LOBBYSIZE";
    return KEY;
}

std::string MakeVarmapKey_LockedIn_Name(hg::PZInteger aSlotIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_LI_NAME_{}", aSlotIndex);
}

std::string MakeVarmapKey_LockedIn_UniqueId(hg::PZInteger aSlotIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_LI_UUID_{}", aSlotIndex);
}

std::string MakeVarmapKey_LockedIn_IpAddr(hg::PZInteger aSlotIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_LI_IPADDR_{}", aSlotIndex);
}

std::string MakeVarmapKey_LockedIn_CData(hg::PZInteger aSlotIndex, hg::PZInteger aCDataIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_LI_CDAT{}_{}", aCDataIndex, aSlotIndex);
}

std::string MakeVarmapKey_Desired_Name(hg::PZInteger aSlotIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_DE_NAME_{}", aSlotIndex);
}

std::string MakeVarmapKey_Desired_UniqueId(hg::PZInteger aSlotIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_DE_UUID_{}", aSlotIndex);
}

std::string MakeVarmapKey_Desired_IpAddr(hg::PZInteger aSlotIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_DE_IPADDR_{}", aSlotIndex);
}

std::string MakeVarmapKey_Desired_CData(hg::PZInteger aSlotIndex, hg::PZInteger aCDataIndex) {
    return fmt::format("::jbatnozic::spempe::DefaultLobbyManager_DE_CDAT{}_{}", aCDataIndex, aSlotIndex);
}

} // namespace

void USPEMPE_DefaultLobbyManager_SetPlayerInfo_Impl(
    DefaultLobbyManager& aLobbyMgr,
    const int aClientIndex,
    const std::string& aName,
    const std::string& aUniqueId,
    const std::string& aCustomData_0,
    const std::string& aCustomData_1,
    const std::string& aCustomData_2,
    const std::string& aCustomData_3
) {
    auto& self = aLobbyMgr;

    hg::PZInteger l;
    for (l = 0; l < self._getSize(); l += 1) {
        auto& entry = self._lockedIn[hg::pztos(l)];
        if (entry.clientIndex == aClientIndex) {
            entry.name = aName;
            entry.uniqueId = aUniqueId;
            entry.customData[0] = aCustomData_0;
            entry.customData[1] = aCustomData_1;
            entry.customData[2] = aCustomData_2;
            entry.customData[3] = aCustomData_3;
            break;
        }
    }
    if (l != self._getSize()) {
        self._updateVarmapForLockedInEntry(l);
    }
    else {
        HG_LOG_WARN(LOG_ID, "USPEMPE_DefaultLobbyManager_SetPlayerInfo_Impl - "
                    "Could not find client with corresponding idx amongst locked in clients.");
    }

    hg::PZInteger d;
    for (d = 0; d < self._getSize(); d += 1) {
        auto& entry = self._desired[hg::pztos(d)];
        if (entry.clientIndex == aClientIndex) {
            entry.name = aName;
            entry.uniqueId = aUniqueId;
            entry.customData[0] = aCustomData_0;
            entry.customData[1] = aCustomData_1;
            entry.customData[2] = aCustomData_2;
            entry.customData[3] = aCustomData_3;
            break;
        }
    }
    if (d != self._getSize()) {
        self._updateVarmapForDesiredEntry(d);
    }
    else {
        HG_LOG_WARN(LOG_ID, "USPEMPE_DefaultLobbyManager_SetPlayerInfo_Impl - "
                    "Could not find client with corresponding idx amongst pending clients.");
    }
}

RN_DEFINE_RPC(USPEMPE_DefaultLobbyManager_SetPlayerInfo,
              RN_ARGS(std::string&, aName, 
                      std::string&, aUniqueId,
                      std::string&, aCustomData_0,
                      std::string&, aCustomData_1,
                      std::string&, aCustomData_2,
                      std::string&, aCustomData_3)) {
    RN_NODE_IN_HANDLER().callIfServer(
        [&](RN_ServerInterface& aServer) {
            const auto sp = SPEMPE_GET_SYNC_PARAMS(aServer);
            USPEMPE_DefaultLobbyManager_SetPlayerInfo_Impl(
                dynamic_cast<DefaultLobbyManager&>(sp.context.getComponent<LobbyManagerInterface>()),
                sp.senderIndex,
                aName,
                aUniqueId,
                aCustomData_0,
                aCustomData_1,
                aCustomData_2,
                aCustomData_3
            );
        });
    RN_NODE_IN_HANDLER().callIfClient(
        [](RN_ClientInterface&) {
            throw RN_IllegalMessage();
        });
}

void USPEMPE_DefaultLobbyManager_SetPlayerIndex_Impl(
    DefaultLobbyManager& aLobbyMgr,
    hobgoblin::PZInteger aPlayerIndex
) {
    auto& self = aLobbyMgr;
    self._localPlayerIndex = aPlayerIndex;
}

RN_DEFINE_RPC(USPEMPE_DefaultLobbyManager_SetPlayerIndex,
              RN_ARGS(hg::PZInteger, aPlayerIndex)) {
    RN_NODE_IN_HANDLER().callIfServer(
        [](RN_ServerInterface&) {
            throw RN_IllegalMessage();
        });
    RN_NODE_IN_HANDLER().callIfClient(
        [=](RN_ClientInterface& aClient) {
            const auto sp = SPEMPE_GET_SYNC_PARAMS(aClient);
            USPEMPE_DefaultLobbyManager_SetPlayerIndex_Impl(
                dynamic_cast<DefaultLobbyManager&>(sp.context.getComponent<LobbyManagerInterface>()),
                aPlayerIndex
            );
        });
}

bool operator==(const DefaultLobbyManager::ExtendedPlayerInfo& aLhs,
                const DefaultLobbyManager::ExtendedPlayerInfo& aRhs) {
    return (static_cast<const PlayerInfo&>(aLhs) == static_cast<const PlayerInfo&>(aRhs) &&
            aLhs.clientIndex == aRhs.clientIndex &&
            aLhs.port == aRhs.port);
}

bool operator!=(const DefaultLobbyManager::ExtendedPlayerInfo& aLhs,
                const DefaultLobbyManager::ExtendedPlayerInfo& aRhs) {
    return !(aLhs == aRhs);
}


bool DefaultLobbyManager::ExtendedPlayerInfo::isSameAs(const hg::RN_ConnectorInterface& aClient) const {
    return (aClient.getRemoteInfo().ipAddress == ipAddress &&
            aClient.getRemoteInfo().port == port);
}

DefaultLobbyManager::DefaultLobbyManager(hg::QAO_RuntimeRef aRuntimeRef, int aExecutionPriority)
    : NonstateObject(aRuntimeRef, SPEMPE_TYPEID_SELF, aExecutionPriority, "::jbatnozic::spempe::DefaultLobbyManager")
{    
}

DefaultLobbyManager::~DefaultLobbyManager() = default;

void DefaultLobbyManager::setToHostMode(hobgoblin::PZInteger aLobbySize) {
    _mode = Mode::Host;
    
    resize(aLobbySize);

    ccomp<SyncedVarmapManagerInterface>().setInt64(
        MakeVarmapKey_LobbySize(),
        aLobbySize
    );

    // Set local player
    _lockedIn[0].name = "<< SPeMPE SERVER >>";
    _lockedIn[0].uniqueId = "n/a";
    _lockedIn[0].ipAddress = "localhost";
    _lockedIn[0].port = 0;
    _lockedIn[0].clientIndex = CLIENT_INDEX_LOCAL;

    _desired[0] = _lockedIn[0];

    _localPlayerIndex = 0;
    _clientIdxToPlayerIdxMapping[CLIENT_INDEX_LOCAL] = 0;

    _updateVarmapForLockedInEntry(0);
    _updateVarmapForDesiredEntry(0);
}

void DefaultLobbyManager::setToClientMode(hobgoblin::PZInteger aLobbySize) {
    _mode = Mode::Client;
    resize(aLobbySize);
}

DefaultLobbyManager::Mode DefaultLobbyManager::getMode() const {
    return _mode;
}

///////////////////////////////////////////////////////////////////////////
// HOST-MODE METHODS                                                     //
///////////////////////////////////////////////////////////////////////////

hg::PZInteger DefaultLobbyManager::clientIdxToPlayerIdx(int aClientIdx) const {
    if (_mode != Mode::Host) {
        throw hg::TracedLogicError("DefaultLobbyManager::clientIdxToPlayerIdx can only "
                                   "be called while in Host mode!");
    }

    const auto iter = _clientIdxToPlayerIdxMapping.find(aClientIdx);
    if (iter != _clientIdxToPlayerIdxMapping.end()) {
        return iter->second;
    }
    // else...? TODO
}

int DefaultLobbyManager::playerIdxToClientIdx(hg::PZInteger aPlayerIdx) const {
    if (_mode != Mode::Host) {
        throw hg::TracedLogicError("DefaultLobbyManager::playerIdxToClientIdx can only "
                                   "be called while in Host mode!");
    }

    return _lockedIn.at(hg::pztos(aPlayerIdx)).clientIndex;
}

void DefaultLobbyManager::beginSwap(hobgoblin::PZInteger aSlotIndex1, hobgoblin::PZInteger aSlotIndex2) {
    throw hg::NotImplementedError{}; // TODO
}

bool DefaultLobbyManager::lockInPendingChanges() {
    if (_mode != Mode::Host) {
        throw hg::TracedLogicError("DefaultLobbyManager::lockInPendingChanges can only "
                                   "be called while in Host mode!");
    }

    if (_lockedIn == _desired) {
        return false; // Nothing to change
    }

    _lockedIn = _desired;
    for (hg::PZInteger slotIndex = 0; slotIndex < _getSize(); slotIndex += 1) {
        const auto clientIndex = _lockedIn[slotIndex].clientIndex;

        _clientIdxToPlayerIdxMapping[clientIndex] = slotIndex;

        _updateVarmapForLockedInEntry(slotIndex);

        auto& netMgr = ccomp<NetworkingManagerInterface>();
        if (clientIndex >= 0) {
            Compose_USPEMPE_DefaultLobbyManager_SetPlayerIndex(
                netMgr.getNode(),
                _lockedIn[slotIndex].clientIndex,
                slotIndex
            );
        } else if (clientIndex == CLIENT_INDEX_LOCAL) {
            _localPlayerIndex = slotIndex;
        }
    }

    return true;
}

bool DefaultLobbyManager::resetPendingChanges() {
    if (_mode != Mode::Host) {
        throw hg::TracedLogicError("DefaultLobbyManager::resetPendingChanges can only "
                                   "be called while in Host mode!");
    }
    return true; // TODO
}

///////////////////////////////////////////////////////////////////////////
// CLIENT-MODE METHODS                                                   //
///////////////////////////////////////////////////////////////////////////

void DefaultLobbyManager::uploadLocalInfo() const {
    if (_mode != Mode::Client) {
        throw hg::TracedLogicError("DefaultLobbyManager::uploadLocalInfo can only "
                                   "be called while in Client mode!");
    }

    auto& netMgr = ccomp<NetworkingManagerInterface>();
    Compose_USPEMPE_DefaultLobbyManager_SetPlayerInfo(
        netMgr.getNode(),
        RN_COMPOSE_FOR_ALL,
        _localPlayerInfo.name,
        _localPlayerInfo.uniqueId,
        _localPlayerInfo.customData[0],
        _localPlayerInfo.customData[1],
        _localPlayerInfo.customData[2],
        _localPlayerInfo.customData[3]
    );
}

///////////////////////////////////////////////////////////////////////////
// MODE-INDEPENDENT METHODS                                              //
///////////////////////////////////////////////////////////////////////////

void DefaultLobbyManager::setLocalName(const std::string& aName) {
    _localPlayerInfo.name = aName;
}

const std::string& DefaultLobbyManager::getLocalName() const {
    return _localPlayerInfo.name;
}

void DefaultLobbyManager::setLocalUniqueId(const std::string& aUniqueId) {
    _localPlayerInfo.uniqueId = aUniqueId;
}

const std::string& DefaultLobbyManager::getLocalUniqueId() const {
    return _localPlayerInfo.uniqueId;
}

void DefaultLobbyManager::setLocalCustomData(hobgoblin::PZInteger aIndex,
                                             const std::string& aCustomData) {
    _localPlayerInfo.customData.at(hg::pztos(aIndex)) = aCustomData;
}

const std::string& DefaultLobbyManager::getLocalCustomData(hobgoblin::PZInteger aIndex) const {
    return _localPlayerInfo.customData.at(hg::pztos(aIndex));
}

hg::PZInteger DefaultLobbyManager::getSize() const {
    return _getSize();
}

void DefaultLobbyManager::resize(hobgoblin::PZInteger aNewLobbySize) {
    if (aNewLobbySize < 1) {
        throw hg::TracedLogicError("DefaultLobbyManager::resize - aNewLobbySize must be at least 1!");
    }

    _lockedIn.resize(hg::pztos(aNewLobbySize));
    _desired.resize(hg::pztos(aNewLobbySize));

    if (_mode == Mode::Host) {
        ccomp<SyncedVarmapManagerInterface>().setInt64(
            MakeVarmapKey_LobbySize(),
            aNewLobbySize
        );
    }
}

const PlayerInfo& DefaultLobbyManager::getLockedInPlayerInfo(hobgoblin::PZInteger aSlotIndex) const {
    return _lockedIn.at(hg::pztos(aSlotIndex));
}

const PlayerInfo& DefaultLobbyManager::getPendingPlayerInfo(hobgoblin::PZInteger aSlotIndex) const {
    return _desired.at(hg::pztos(aSlotIndex));
}

bool  DefaultLobbyManager::areChangesPending(hobgoblin::PZInteger aSlotIndex) const {
    return (_lockedIn.at(hg::pztos(aSlotIndex)) != _desired[hg::pztos(aSlotIndex)]);
}

bool  DefaultLobbyManager::areChangesPending() const {
    for (hg::PZInteger i = 0; i < _getSize(); i += 1) {
        if (areChangesPending(i)) {
            return true;
        }
    }
    return false;
}

int DefaultLobbyManager::getLocalPlayerIndex() const {
    return _localPlayerIndex;
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS                                                       //
///////////////////////////////////////////////////////////////////////////

// TODO: break up into smaller functions
void DefaultLobbyManager::_eventPreUpdate() {
    auto& netMgr = ccomp<NetworkingManagerInterface>();
    if (netMgr.isServer()) {
        // ***** SERVER ***** //

        auto& server = netMgr.getServer();

        _removeDesiredEntriesForDisconnectedPlayers();

        // Check if all connected clients are represented in _desired. If not, add them.
        for (hg::PZInteger i = 0; i < server.getSize(); i += 1) {
            const auto& client = server.getClientConnector(i);

            if (IsConnected(client) && !_hasEntryForClient(client)) {
                const auto pos = _findOptimalPositionForClient(client);

                _desired[hg::pztos(pos)].name        = "<name>";
                _desired[hg::pztos(pos)].uniqueId    = "<uuid>";
                _desired[hg::pztos(pos)].ipAddress   = client.getRemoteInfo().ipAddress.toString();
                _desired[hg::pztos(pos)].port        = client.getRemoteInfo().port;
                _desired[hg::pztos(pos)].clientIndex = i;

                _updateVarmapForDesiredEntry(pos);

                HG_LOG_INFO(LOG_ID, "Inserted new player into slot {}", pos);
            }
        }

        if (ccomp<WindowManagerInterface>().getKeyboardInput().checkPressed(KbKey::L, KbInput::Mode::Direct)) {
            if (_mode == Mode::Host) {
                HG_LOG_INFO(LOG_ID, "Locking in slots");
                lockInPendingChanges(); // TODO Temp.
            }
        }
    } else {
        // ***** CLIENT ***** //

        const auto& varmap = ccomp<SyncedVarmapManagerInterface>();

        // Check that size is correct and adjust if needed
        const auto& size = varmap.getInt64(MakeVarmapKey_LobbySize());
        if (size && *size != _getSize()) {
            resize(*size);
        }

        // Load names & other info
        for (hg::PZInteger i = 0; i < _getSize(); i += 1) {
            // Locked in:
            {
                const auto n = varmap.getString(MakeVarmapKey_LockedIn_Name(i));
                _lockedIn[i].name = n ? *n : "";
            }
            {
                const auto a = varmap.getString(MakeVarmapKey_LockedIn_IpAddr(i));
                _lockedIn[i].ipAddress = a ? *a : "";
            }
            {
                const auto u = varmap.getString(MakeVarmapKey_LockedIn_UniqueId(i));
                _lockedIn[i].uniqueId = u ? *u : "";
            }
            {
                for (hg::PZInteger j = 0; j < CUSTOM_DATA_LEN; j += 1) {
                    const auto c = varmap.getString(MakeVarmapKey_LockedIn_CData(i, j));
                    _lockedIn[i].customData[j] = c ? *c : "";
                }
            }
            // Desired:
            {
                const auto n = varmap.getString(MakeVarmapKey_Desired_Name(i));
                _desired[i].name = n ? *n : "";
            }
            {
                const auto a = varmap.getString(MakeVarmapKey_Desired_IpAddr(i));
                _desired[i].ipAddress = a ? *a : "";
            }
            {
                const auto u = varmap.getString(MakeVarmapKey_Desired_UniqueId(i));
                _desired[i].uniqueId = u ? *u : "";
            }
            {
                for (hg::PZInteger j = 0; j < CUSTOM_DATA_LEN; j += 1) {
                    const auto c = varmap.getString(MakeVarmapKey_Desired_CData(i, j));
                    _desired[i].customData[j] = c ? *c : "";
                }
            }
        }
    }
    HG_LOG_INFO(LOG_ID, "Local player idx = {}", getLocalPlayerIndex());
}

void DefaultLobbyManager::_eventDrawGUI() {
    if (auto* winMgr = ctx().getComponentPtr<WindowManagerInterface>()) {
        std::stringstream ss;
        for (std::size_t i = 0; i < _lockedIn.size(); i += 1) {
            ss << "POSITION " << i << ":\n";
            ss << fmt::format("    {} {}:{}", _lockedIn[i].name, _lockedIn[i].ipAddress, _lockedIn[i].port);

            if (static_cast<const PlayerInfo&>(_lockedIn[i])
                != static_cast<const PlayerInfo&>(_desired[i])) {
                //HG_LOG_INFO(LOG_ID, "{} {} {} != {} {} {}",
                //            _lockedIn[i].name,
                //            _lockedIn[i].uniqueId,
                //            _lockedIn[i].ipAddress,
                //            _desired[i].name,
                //            _desired[i].uniqueId,
                //            _desired[i].ipAddress);
                ss << fmt::format(" <- {} {}:{}\n", _desired[i].name, _desired[i].ipAddress, _desired[i].port);
            } else {
                ss << '\n';
            }
        }

        auto& canvas = winMgr->getCanvas();
        sf::Text text;
        text.setPosition(32.f, 32.f);
        text.setFont(hg::gr::BuiltInFonts::getFont(hg::gr::BuiltInFonts::TitilliumRegular));
        text.setFillColor(sf::Color::Yellow);
        text.setString(ss.str());
        canvas.draw(text);
    }
}

hg::PZInteger DefaultLobbyManager::_getSize() const {
    assert(_lockedIn.size() == _desired.size());
    return hg::stopz(_lockedIn.size());
}

bool DefaultLobbyManager::_hasEntryForClient(const RN_ConnectorInterface& aClient) const {
    for (const auto& entry : _desired) {
        if (entry.isSameAs(aClient)) {
            return true;
        }
    }
    return false;
}

hg::PZInteger DefaultLobbyManager::_findOptimalPositionForClient(const RN_ConnectorInterface& aClient) const {
    struct {
        int pos = -1;
        int score = -1;
    } currentBest;

    for (std::size_t j = 0; j < _lockedIn.size(); j += 1) {
        if (_desired[j].clientIndex == CLIENT_INDEX_UNKNOWN) {
            // Apply scoring
            int score = 0;
            if (_lockedIn[j].ipAddress == aClient.getRemoteInfo().ipAddress) {
                score += 1;
            }
            // TODO score based on name?
            // TODO score based on ID?

            // Replace current best?
            if (score > currentBest.score) {
                currentBest.pos = static_cast<int>(j);
                currentBest.score = score;
            }
        }
    }

    if (currentBest.pos < 0) {
        throw hg::TracedRuntimeError("No slot available for new client!");
    }

    return currentBest.pos;
}

void DefaultLobbyManager::_removeDesiredEntriesForDisconnectedPlayers() {
    const auto& server = ccomp<NetworkingManagerInterface>().getServer();

    for (std::size_t i = 0; i < _desired.size(); i += 1) {
        auto& player = _desired[i];

        if (player.clientIndex == CLIENT_INDEX_UNKNOWN || player.clientIndex == CLIENT_INDEX_LOCAL) {
            continue;
        }

        const auto& client = server.getClientConnector(player.clientIndex);
        if (!IsConnected(client) || !player.isSameAs(client)) {
            HG_LOG_INFO(LOG_ID, "Removing disconnected client {}", player.clientIndex);

            player.name.clear();
            player.uniqueId.clear();
            player.ipAddress.clear();
            player.port = 0;
            player.clientIndex = CLIENT_INDEX_UNKNOWN;

            _updateVarmapForDesiredEntry(i);
        }
    }
}

void  DefaultLobbyManager::_updateVarmapForLockedInEntry(hobgoblin::PZInteger aSlotIndex) const {
    const auto& entry = _lockedIn[hg::pztos(aSlotIndex)];
    auto& varmap = ccomp<SyncedVarmapManagerInterface>();

    varmap.setString(MakeVarmapKey_LockedIn_Name(aSlotIndex), entry.name);
    varmap.setString(MakeVarmapKey_LockedIn_IpAddr(aSlotIndex), entry.ipAddress);
    varmap.setString(MakeVarmapKey_LockedIn_UniqueId(aSlotIndex), entry.uniqueId);

    for (hg::PZInteger i = 0; i < CUSTOM_DATA_LEN; i += 1) {
        varmap.setString(MakeVarmapKey_LockedIn_CData(aSlotIndex, i), entry.customData[i]);
    }
}

void  DefaultLobbyManager::_updateVarmapForDesiredEntry(hobgoblin::PZInteger aSlotIndex) const {
    const auto& entry = _desired[hg::pztos(aSlotIndex)];
    auto& varmap = ccomp<SyncedVarmapManagerInterface>();

    varmap.setString(MakeVarmapKey_Desired_Name(aSlotIndex), entry.name);
    varmap.setString(MakeVarmapKey_Desired_IpAddr(aSlotIndex), entry.ipAddress);
    varmap.setString(MakeVarmapKey_Desired_UniqueId(aSlotIndex), entry.uniqueId);

    for (hg::PZInteger i = 0; i < CUSTOM_DATA_LEN; i += 1) {
        varmap.setString(MakeVarmapKey_Desired_CData(aSlotIndex, i), entry.customData[i]);
    }
}

} // namespace spempe
} // namespace jbatnozic

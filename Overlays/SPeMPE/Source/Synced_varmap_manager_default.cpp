// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/Managers/Networking_manager_interface.hpp>
#include <SPeMPE/Managers/Synced_varmap_manager_default.hpp>
#include <SPeMPE/Utility/Rpc_receiver_context_template.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/RigelNet_macros.hpp>

#include <Hobgoblin/Common.hpp>

namespace jbatnozic {
namespace spempe {

using namespace hobgoblin::rn;

void USPEMPE_DefaultSyncedVarmapManager_SetValues(DefaultSyncedVarmapManager& aMgr,
                                                  hobgoblin::util::Packet&    aPacket) {
    if (aMgr._mode != DefaultSyncedVarmapManager::Mode::Client) {
        throw RN_IllegalMessage{"DefaultSyncedVarmapManager_SetValues - Called on non-Client."};
    }
    aMgr._unpackValues(aPacket);
}

void USPEMPE_DefaultSyncedVarmapManager_SetValueRequested(DefaultSyncedVarmapManager& aMgr,
                                                          hobgoblin::PZInteger        aPlayerIndex,
                                                          hobgoblin::util::Packet&    aPacket) {
    if (aMgr._mode != DefaultSyncedVarmapManager::Mode::Host) {
        throw RN_IllegalMessage{"DefaultSyncedVarmapManager_SetValueRequested - Called on non-Host."};
    }
    if (!aMgr._setValueRequested(aPlayerIndex, aPacket)) {
        throw RN_IllegalMessage{"DefaultSyncedVarmapManager_SetValueRequested - Illegal request."};
    }
}

namespace {
constexpr char TYPE_TAG_INT64  = 'L';
constexpr char TYPE_TAG_DOUBLE = 'D';
constexpr char TYPE_TAG_STRING = 'S';

RN_DEFINE_RPC(USPEMPE_DefaultSyncedVarmapManager_SetValues, RN_ARGS(hobgoblin::util::Packet&, aPacket)) {
    RN_NODE_IN_HANDLER().callIfServer([&](RN_ServerInterface& aServer) {
        const auto rc     = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aServer);
        auto&      svmMgr = dynamic_cast<DefaultSyncedVarmapManager&>(
            rc.gameContext.getComponent<SyncedVarmapManagerInterface>());
        USPEMPE_DefaultSyncedVarmapManager_SetValues(svmMgr, aPacket);
    });
    RN_NODE_IN_HANDLER().callIfClient([&](RN_ClientInterface& aClient) {
        const auto rc     = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aClient);
        auto&      svmMgr = dynamic_cast<DefaultSyncedVarmapManager&>(
            rc.gameContext.getComponent<SyncedVarmapManagerInterface>());
        USPEMPE_DefaultSyncedVarmapManager_SetValues(svmMgr, aPacket);
    });
}

RN_DEFINE_RPC(USPEMPE_DefaultSyncedVarmapManager_RequestToSet,
              RN_ARGS(hobgoblin::util::Packet&, aPacket)) {
    RN_NODE_IN_HANDLER().callIfServer([&](RN_ServerInterface& aServer) {
        const auto rc     = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aServer);
        auto&      svmMgr = dynamic_cast<DefaultSyncedVarmapManager&>(
            rc.gameContext.getComponent<SyncedVarmapManagerInterface>());
        USPEMPE_DefaultSyncedVarmapManager_SetValueRequested(svmMgr, rc.senderIndex + 1, aPacket);
    });
    RN_NODE_IN_HANDLER().callIfClient([&](RN_ClientInterface& aClient) {
        const auto rc     = SPEMPE_GET_RPC_RECEIVER_CONTEXT(aClient);
        auto&      svmMgr = dynamic_cast<DefaultSyncedVarmapManager&>(
            rc.gameContext.getComponent<SyncedVarmapManagerInterface>());
        USPEMPE_DefaultSyncedVarmapManager_SetValueRequested(svmMgr, 0, aPacket);
    });
}
} // namespace

DefaultSyncedVarmapManager::DefaultSyncedVarmapManager(hobgoblin::QAO_IKey aIKey, int aExecutionPriority)
    : NonstateObject{aIKey,
                     SPEMPE_TYPEID_SELF,
                     aExecutionPriority,
                     "::jbatnozic::spempe::DefaultSyncedVarmapManager"} {}

DefaultSyncedVarmapManager::~DefaultSyncedVarmapManager() = default;

void DefaultSyncedVarmapManager::_didAttach(hobgoblin::QAO_Runtime& aRuntime) {
    _netMgr = &ccomp<NetworkingManagerInterface>();
    _netMgr->addEventListener(this);
}

void DefaultSyncedVarmapManager::_willDetach(hobgoblin::QAO_Runtime& aRuntime) {
    _netMgr->removeEventListener(this);
    _netMgr = nullptr;
}

void DefaultSyncedVarmapManager::setToMode(Mode aMode) {
    if (_mode != Mode::Uninitialized) {
        HG_NOT_IMPLEMENTED();
    }

    if (aMode == Mode::Host) {
        SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged == true, networking = true);
    } else if (aMode == Mode::Client) {
        SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged == false, networking = true);
    }

    _mode = aMode;
}

void DefaultSyncedVarmapManager::onNetworkingEvent(const hg::RN_Event& aEvent) {
    if (_mode != Mode::Host) {
        return;
    }

    using namespace hg::rn;
    aEvent.strictVisit(
        [](const RN_Event::BadPassphrase&) {
            // Don't care
        },
        [](const RN_Event::ConnectAttemptFailed&) {
            // Don't care
        },
        [this](const RN_Event::Connected& aConnectedEvent) {
            _netMgr->getNode().callIfServer([&, this](RN_ServerInterface& aServer) {
                const auto clientIndex = *(aConnectedEvent.clientIndex);
                if (aServer.getClientConnector(clientIndex).getStatus() ==
                    RN_ConnectorStatus::Connected) {
                    this->_sendFullState(clientIndex);
                }
            });
        },
        [](const RN_Event::Disconnected&) {
            // Don't care
        });
}

///////////////////////////////////////////////////////////////////////////
// VALUE GETTERS                                                         //
///////////////////////////////////////////////////////////////////////////

auto DefaultSyncedVarmapManager::getInt64(const std::string& aKey) const -> std::optional<std::int64_t> {
    const auto iter = _int64Values.find(aKey);
    if (iter != _int64Values.end() && iter->second.value.has_value()) {
        return *(iter->second.value);
    }
    return {};
}

auto DefaultSyncedVarmapManager::getDouble(const std::string& aKey) const -> std::optional<double> {
    const auto iter = _doubleValues.find(aKey);
    if (iter != _doubleValues.end() && iter->second.value.has_value()) {
        return *(iter->second.value);
    }
    return {};
}

auto DefaultSyncedVarmapManager::getString(const std::string& aKey) const -> std::optional<std::string> {
    const auto iter = _stringValues.find(aKey);
    if (iter != _stringValues.end() && iter->second.value.has_value()) {
        return *(iter->second.value);
    }
    return {};
}

///////////////////////////////////////////////////////////////////////////
// VALUE SETTERS (HOST SIDE)                                             //
///////////////////////////////////////////////////////////////////////////

void DefaultSyncedVarmapManager::setInt64(const std::string& aKey, std::int64_t aValue) {
    if (_mode != Mode::Host) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only set values while in Host mode.");
    }

    const auto iter = _int64Values.find(aKey);
    if (iter != _int64Values.end() && iter->second.value.has_value()) {
        if (aValue != *(iter->second.value)) {
            iter->second.value = aValue;
            _packValue(aKey, aValue, _stateUpdates);
        }
    } else {
        _int64Values[aKey].value = aValue;
        _packValue(aKey, aValue, _stateUpdates);
    }
}

void DefaultSyncedVarmapManager::setDouble(const std::string& aKey, double aValue) {
    if (_mode != Mode::Host) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only set values while in Host mode.");
    }

    const auto iter = _doubleValues.find(aKey);
    if (iter != _doubleValues.end() && iter->second.value.has_value()) {
        if (aValue != *(iter->second.value)) {
            iter->second.value = aValue;
            _packValue(aKey, aValue, _stateUpdates);
        }
    } else {
        _doubleValues[aKey].value = aValue;
        _packValue(aKey, aValue, _stateUpdates);
    }
}

void DefaultSyncedVarmapManager::setString(const std::string& aKey, const std::string& aValue) {
    if (_mode != Mode::Host) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only set values while in Host mode.");
    }

    const auto iter = _stringValues.find(aKey);
    if (iter != _stringValues.end() && iter->second.value.has_value()) {
        if (aValue != *(iter->second.value)) {
            iter->second.value = aValue;
            _packValue(aKey, aValue, _stateUpdates);
        }
    } else {
        _stringValues[aKey].value = aValue;
        _packValue(aKey, aValue, _stateUpdates);
    }
}

///////////////////////////////////////////////////////////////////////////
// VALUE SET REQUESTERS (CLIENT SIDE)                                    //
///////////////////////////////////////////////////////////////////////////

void DefaultSyncedVarmapManager::requestToSetInt64(const std::string& aKey, std::int64_t aValue) {
    if (_mode != Mode::Client) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only request to set values while in Client mode.");
    }

    hobgoblin::util::Packet packet;
    _packValue(aKey, aValue, packet);
    Compose_USPEMPE_DefaultSyncedVarmapManager_RequestToSet(_netMgr->getNode(),
                                                            RN_COMPOSE_FOR_ALL,
                                                            packet);
}

void DefaultSyncedVarmapManager::requestToSetDouble(const std::string& aKey, double aValue) {
    if (_mode != Mode::Client) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only request to set values while in Client mode.");
    }

    hobgoblin::util::Packet packet;
    _packValue(aKey, aValue, packet);
    Compose_USPEMPE_DefaultSyncedVarmapManager_RequestToSet(_netMgr->getNode(),
                                                            RN_COMPOSE_FOR_ALL,
                                                            packet);
}

void DefaultSyncedVarmapManager::requestToSetString(const std::string& aKey, const std::string& aValue) {
    if (_mode != Mode::Client) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only request to set values while in Client mode.");
    }

    hobgoblin::util::Packet packet;
    _packValue(aKey, aValue, packet);
    Compose_USPEMPE_DefaultSyncedVarmapManager_RequestToSet(_netMgr->getNode(),
                                                            RN_COMPOSE_FOR_ALL,
                                                            packet);
}

///////////////////////////////////////////////////////////////////////////
// WRITE PERMISSION SETTERS (HOST SIDE)                                  //
///////////////////////////////////////////////////////////////////////////

void DefaultSyncedVarmapManager::int64SetClientWritePermission(const std::string& aKey,
                                                               hg::PZInteger      aPlayerIndex,
                                                               bool               aAllowed) {
    if (_mode != Mode::Host) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only set permissions while in Host mode.");
    }

    auto& perms = _int64Values[aKey].permissions;
    if (aAllowed == ALLOWED) {
        perms.setBit(aPlayerIndex);
    } else {
        perms.clearBit(aPlayerIndex);
    }
}

void DefaultSyncedVarmapManager::doubleSetClientWritePermission(const std::string& aKey,
                                                                hg::PZInteger      aPlayerIndex,
                                                                bool               aAllowed) {
    if (_mode != Mode::Host) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only set permissions while in Host mode.");
    }

    auto& perms = _doubleValues[aKey].permissions;
    if (aAllowed == ALLOWED) {
        perms.setBit(aPlayerIndex);
    } else {
        perms.clearBit(aPlayerIndex);
    }
}

void DefaultSyncedVarmapManager::stringSetClientWritePermission(const std::string& aKey,
                                                                hg::PZInteger      aPlayerIndex,
                                                                bool               aAllowed) {
    if (_mode != Mode::Host) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Can only set permissions while in Host mode.");
    }

    auto& perms = _stringValues[aKey].permissions;
    if (aAllowed == ALLOWED) {
        perms.setBit(aPlayerIndex);
    } else {
        perms.clearBit(aPlayerIndex);
    }
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS                                                       //
///////////////////////////////////////////////////////////////////////////

void DefaultSyncedVarmapManager::_eventEndUpdate() {
    using namespace hg::rn;
    if (_mode == Mode::Host) {
        Compose_USPEMPE_DefaultSyncedVarmapManager_SetValues(_netMgr->getNode(),
                                                             RN_COMPOSE_FOR_ALL,
                                                             _stateUpdates);
        _stateUpdates.clear();
    }
}

void DefaultSyncedVarmapManager::_packValue(const std::string&       aKey,
                                            std::int64_t             aValue,
                                            hobgoblin::util::Packet& aPacket) {
    aPacket << std::int8_t{TYPE_TAG_INT64} << aKey << aValue;
}

void DefaultSyncedVarmapManager::_packValue(const std::string&       aKey,
                                            double                   aValue,
                                            hobgoblin::util::Packet& aPacket) {
    aPacket << std::int8_t{TYPE_TAG_DOUBLE} << aKey << aValue;
}

void DefaultSyncedVarmapManager::_packValue(const std::string&       aKey,
                                            const std::string&       aValue,
                                            hobgoblin::util::Packet& aPacket) {
    aPacket << std::int8_t{TYPE_TAG_STRING} << aKey << aValue;
}

void DefaultSyncedVarmapManager::_unpackValues(hobgoblin::util::Packet& aPacket) {
    while (!aPacket.endOfPacket()) {
        const auto type = aPacket.extract<std::int8_t>();
        const auto key  = aPacket.extract<std::string>();

        switch (type) {
        case TYPE_TAG_INT64:
            _int64Values[key].value = aPacket.extract<std::int64_t>();
            break;

        case TYPE_TAG_DOUBLE:
            _doubleValues[key].value = aPacket.extract<double>();
            break;

        case TYPE_TAG_STRING:
            _stringValues[key].value = aPacket.extract<std::string>();
            break;

        default:
            HG_THROW_TRACED(hobgoblin::TracedRuntimeError,
                            0,
                            "Unknown type tag ({}) encountered while unpacking.",
                            (int)type);
        }
    }
}

bool DefaultSyncedVarmapManager::_setValueRequested(hobgoblin::PZInteger     aPlayerIndex,
                                                    hobgoblin::util::Packet& aPacket) {
    const auto type = aPacket.extract<std::int8_t>();
    const auto key  = aPacket.extract<std::string>();

    switch (type) {
    case TYPE_TAG_INT64:
        {
            const auto iter = _int64Values.find(key);
            if (iter == _int64Values.end()) {
                return false;
            }
            if (iter->second.permissions.getBit(aPlayerIndex) == false) {
                return false;
            }
            setInt64(key, aPacket.extract<std::int64_t>());
        }
        break;

    case TYPE_TAG_DOUBLE:
        {
            const auto iter = _doubleValues.find(key);
            if (iter == _doubleValues.end()) {
                return false;
            }
            if (iter->second.permissions.getBit(aPlayerIndex) == false) {
                return false;
            }
            setDouble(key, aPacket.extract<double>());
        }
        break;

    case TYPE_TAG_STRING:
        {
            const auto iter = _stringValues.find(key);
            if (iter == _stringValues.end()) {
                return false;
            }
            if (iter->second.permissions.getBit(aPlayerIndex) == false) {
                return false;
            }
            setString(key, aPacket.extract<std::string>());
        }
        break;

    default:
        return false;
    }

    return true;
}

void DefaultSyncedVarmapManager::_sendFullState(hg::PZInteger aClientIndex) const {
    hobgoblin::util::Packet packet;

    for (const auto& pair : _int64Values) {
        if (pair.second.value.has_value()) {
            _packValue(pair.first, *(pair.second.value), packet);
        }
    }

    for (const auto& pair : _doubleValues) {
        if (pair.second.value.has_value()) {
            _packValue(pair.first, *(pair.second.value), packet);
        }
    }

    for (const auto& pair : _stringValues) {
        if (pair.second.value.has_value()) {
            _packValue(pair.first, *(pair.second.value), packet);
        }
    }

    Compose_USPEMPE_DefaultSyncedVarmapManager_SetValues(_netMgr->getNode(), aClientIndex, packet);
}

} // namespace spempe
} // namespace jbatnozic

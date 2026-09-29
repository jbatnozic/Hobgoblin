// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/Services/Networking_service_default.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>

#include <algorithm>
#include <cassert>
#include <type_traits>

namespace jbatnozic {
namespace spempe {
namespace {
constexpr const char* LOG_ID = "SPeMPE";
} // namespace

DefaultNetworkingService::DefaultNetworkingService(hobgoblin::QAO_InstGuard aInstGuard,
                                                   int                      aExecutionPriority,
                                                   hg::PZInteger            aStateBufferingLength)
    : NonstateObject{aInstGuard,
                     hg::QAO_ExeCon::ESSENTIAL,
                     aExecutionPriority,
                     QAO_STATIC_NAME("::jbatnozic::spempe::DefaultNetworkingService")}
    , _node{hg::RN_ServerFactory::createDummyServer()}
    , _syncObjReg{*_node, aStateBufferingLength} {}

DefaultNetworkingService::~DefaultNetworkingService() {
    if (_mode != Mode::Uninitialized) {
        _node->removeEventListener(this);
    }
}

///////////////////////////////////////////////////////////////////////////
// CONFIGURATION                                                         //
///////////////////////////////////////////////////////////////////////////

void DefaultNetworkingService::setToServerMode(hg::RN_Protocol        aProtocol,
                                               std::string            aPassphrase,
                                               hg::PZInteger          aServerSize,
                                               hg::PZInteger          aMaxPacketSize,
                                               hg::RN_NetworkingStack aNetworkingStack) {
    assert(_mode == Mode::Uninitialized);

    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged == true, networking == true);
    _localClientIndex = CLIENT_INDEX_LOCAL;
    _node             = hg::RN_ServerFactory::createServer(aProtocol,
                                               std::move(aPassphrase),
                                               aServerSize,
                                               aMaxPacketSize,
                                               aNetworkingStack);
    _node->setUserData(&ctx());
    _node->addEventListener(this);
    _syncObjReg.setNode(*_node);

    _mode = Mode::Server;
}

void DefaultNetworkingService::setToClientMode(hg::RN_Protocol        aProtocol,
                                               std::string            aPassphrase,
                                               hg::PZInteger          aMaxPacketSize,
                                               hg::RN_NetworkingStack aNetworkingStack) {
    assert(_mode == Mode::Uninitialized);

    SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(ctx(), privileged == false, networking == true);
    _localClientIndex = CLIENT_INDEX_UNKNOWN;
    _node             = hg::RN_ClientFactory::createClient(aProtocol,
                                               std::move(aPassphrase),
                                               aMaxPacketSize,
                                               aNetworkingStack);
    _node->setUserData(&ctx());
    _node->addEventListener(this);
    _syncObjReg.setNode(*_node);

    _mode = Mode::Client;
}

NetworkingService::Mode DefaultNetworkingService::getMode() const {
    return _mode;
}

bool DefaultNetworkingService::isUninitialized() const {
    return _mode == Mode::Uninitialized;
}

bool DefaultNetworkingService::isServer() const {
    return _mode == Mode::Server;
}

bool DefaultNetworkingService::isClient() const {
    return _mode == Mode::Client;
}

///////////////////////////////////////////////////////////////////////////
// NODE ACCESS                                                           //
///////////////////////////////////////////////////////////////////////////

NetworkingService::NodeType& DefaultNetworkingService::getNode() const {
    return *_node;
}

NetworkingService::ServerType& DefaultNetworkingService::getServer() const {
    assert(isServer());
    return static_cast<ServerType&>(getNode());
}

NetworkingService::ClientType& DefaultNetworkingService::getClient() const {
    assert(isClient());
    return static_cast<ClientType&>(getNode());
}

///////////////////////////////////////////////////////////////////////////
// LISTENER MANAGEMENT                                                   //
///////////////////////////////////////////////////////////////////////////

void DefaultNetworkingService::addEventListener(hg::NeverNull<NetworkingEventListener*> aEventListener) {
    const auto iter = std::find_if(_eventListeners.begin(),
                                   _eventListeners.end(),
                                   [=](hg::RN_EventListener* aFoundListener) {
                                       return (aFoundListener == aEventListener);
                                   });
    if (iter == _eventListeners.end()) {
        _eventListeners.push_back(aEventListener);
    } else {
        HG_LOG_WARN(LOG_ID,
                    "Listener matching pointer {:#x} was already added.",
                    reinterpret_cast<std::uintptr_t>(aEventListener.get()));
    }
}

void DefaultNetworkingService::removeEventListener(
    hg::NeverNull<NetworkingEventListener*> aEventListener) {
    const auto iter = std::find_if(_eventListeners.begin(),
                                   _eventListeners.end(),
                                   [=](hg::RN_EventListener* aFoundListener) {
                                       return (aFoundListener == aEventListener);
                                   });
    if (iter != _eventListeners.end()) {
        _eventListeners.erase(iter);
    } else {
        HG_LOG_WARN(LOG_ID,
                    "No listener matching pointer {:#x} was found to remove.",
                    reinterpret_cast<std::uintptr_t>(aEventListener.get()));
    }
}

///////////////////////////////////////////////////////////////////////////
// SYNCHRONIZATION                                                       //
///////////////////////////////////////////////////////////////////////////

hg::PZInteger DefaultNetworkingService::getStateBufferingLength() const {
    return _syncObjReg.getDefaultDelay();
}

void DefaultNetworkingService::setStateBufferingLength(hg::PZInteger aNewStateBufferingLength) {
    _syncObjReg.setDefaultDelay(aNewStateBufferingLength);
}

void DefaultNetworkingService::setPacemakerPulsePeriod(hg::PZInteger aPeriod) {
    _syncObjReg.setPacemakerPulsePeriod(aPeriod);
}

void DefaultNetworkingService::setSyncCreateExeconFilter(ExeConSyncFilter aFilter) {
    _syncObjReg.setSyncCreateExeconFilter(aFilter);
}

void DefaultNetworkingService::setSyncUpdateExeconFilter(ExeConSyncFilter aFilter) {
    _syncObjReg.setSyncUpdateExeconFilter(aFilter);
}

void DefaultNetworkingService::setSyncDestroyExeconFilter(ExeConSyncFilter aFilter) {
    _syncObjReg.setSyncDestroyExeconFilter(aFilter);
}

auto DefaultNetworkingService::getSyncCreateExeconFilter() const -> ExeConSyncFilter {
    return _syncObjReg.getSyncCreateExeconFilter();
}

auto DefaultNetworkingService::getSyncUpdateExeconFilter() const -> ExeConSyncFilter {
    return _syncObjReg.getSyncUpdateExeconFilter();
}

auto DefaultNetworkingService::getSyncDestroyExeconFilter() const -> ExeConSyncFilter {
    return _syncObjReg.getSyncDestroyExeconFilter();
}

void DefaultNetworkingService::setAutomaticStateSyncForNewConnectionsEnabled(bool aEnabled) {
    // TODO
}

void DefaultNetworkingService::syncCompleteStateToClient(hg::PZInteger aClientIndex, bool aCleanFirst) {
    // TODO
}

///////////////////////////////////////////////////////////////////////////
// TELEMETRY                                                             //
///////////////////////////////////////////////////////////////////////////

void DefaultNetworkingService::setTelemetryCycleLimit(hg::PZInteger aCycleLimit) {
    while (hg::stopz(_telemetry.size()) < aCycleLimit) {
        _telemetry.emplace_front();
    }

    while (hg::stopz(_telemetry.size()) > aCycleLimit) {
        _telemetry.pop_front();
    }
}

hg::RN_Telemetry DefaultNetworkingService::getTelemetry(hg::PZInteger aCycleCount) const {
    if (aCycleCount > hg::stopz(_telemetry.size())) {
        HG_THROW_TRACED(hg::TracedLogicError,
                        0,
                        "aCycleCount ({}) must not be greater than the telemetry cycle limit ({}).",
                        aCycleCount,
                        _telemetry.size());
    }

    hg::RN_Telemetry result;
    for (int i = static_cast<int>(_telemetry.size()) - 1;
         i > static_cast<int>(_telemetry.size()) - 1 - aCycleCount;
         i -= 1) {
        result += _telemetry[static_cast<std::size_t>(i)];
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////
// MISC.                                                                 //
///////////////////////////////////////////////////////////////////////////

int DefaultNetworkingService::getLocalClientIndex() const {
    return _localClientIndex;
}

hg::NeverNull<void*> DefaultNetworkingService::__spempeimpl_getRegistryAddress() {
    return &_syncObjReg;
}

///////////////////////////////////////////////////////////////////////////
// PROTECTED & PRIVATE METHODS                                           //
///////////////////////////////////////////////////////////////////////////

void DefaultNetworkingService::_eventPreUpdate() {
    _telemetry.emplace_back();
    _telemetry.pop_front();
}

void DefaultNetworkingService::_eventBeginUpdate() {
    const auto telemetry = _node->update(hg::RN_UpdateMode::Receive);
    if (!_telemetry.empty()) {
        _telemetry.back() += telemetry;
    }
}

void DefaultNetworkingService::_eventEndUpdate() {
    // Update all Synchronized objects
    if (_node->isServer()) {
        _syncObjReg.update();
        _syncObjReg.syncStateUpdates();
    }

    const auto telemetry = _node->update(hg::RN_UpdateMode::Send);
    if (!_telemetry.empty()) {
        _telemetry.back() += telemetry;
    }
}

void DefaultNetworkingService::onNetworkingEvent(const hg::RN_Event& aEvent) {
    using hg::RN_Event;

    aEvent.strictVisit(
        [](const RN_Event::BadPassphrase&) {
            HG_LOG_INFO(LOG_ID, "Received event: Bad passphrase.");
        },
        [](const RN_Event::ConnectAttemptFailed&) {
            HG_LOG_INFO(LOG_ID, "Received event: Connection attempt failed ({}).", /*ev.reason*/ "?");
        },
        [this](const RN_Event::Connected& aEventData) {
            if (_node->isServer()) {
                HG_LOG_INFO(LOG_ID,
                            "Received event: New client ({}) connected.",
                            *aEventData.clientIndex);
                _syncObjReg.syncCompleteState(*aEventData.clientIndex);
            } else {
                HG_LOG_INFO(LOG_ID, "Received event: Connected to server.");
                _localClientIndex = getClient().getClientIndex();
            }
        },
        [this](const RN_Event::Disconnected& aEventData) {
            HG_LOG_INFO(LOG_ID, "Received event: Disconnect ({}).", aEventData.message);
            if (!_node->isServer()) {
                _localClientIndex = CLIENT_INDEX_UNKNOWN;
            }
        });

    for (const auto& listener : _eventListeners) {
        listener->onNetworkingEvent(aEvent);
    }
}

} // namespace spempe
} // namespace jbatnozic

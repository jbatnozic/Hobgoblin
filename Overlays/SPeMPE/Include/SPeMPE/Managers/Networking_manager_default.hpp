// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_MANAGERS_NETWORKING_MANAGER_DEFAULT_HPP
#define SPEMPE_MANAGERS_NETWORKING_MANAGER_DEFAULT_HPP

#include <SPeMPE/GameObjectFramework/Game_object_bases.hpp>
#include <SPeMPE/Managers/Networking_manager.hpp>

#include <deque>
#include <memory>
#include <vector>

namespace jbatnozic {
namespace spempe {

/*!
 * \brief Default implementation of `NetworkingManager`.
 *
 * Dependencies:
 * - <none>
 *
 * Event listing:
 * - _eventPreUpdate()   => Adjust telemetry data.
 * - _eventBeginUpdate() => Process received packets and messages.
 * - _eventEndUpdate()   => Send composed messages.
 *
 * Execution priority hint:
 * - Highest, executes before everything else (except `SyncedVarmapManager`).
 * - NOTE: the execution priority for sending data during _eventEndUpdate() is separately configurable.
 */
class DefaultNetworkingManager
    : public NetworkingManager
    , public NonstateObject
    , private hg::RN_EventListener {
public:
    DefaultNetworkingManager(hobgoblin::QAO_InstGuard aInstGuard,
                             int                      aExecutionPriority,
                             int                      aSendExecutionPriority,
                             hg::PZInteger            aStateBufferingLength);

    DefaultNetworkingManager(hobgoblin::QAO_InstGuard aInstGuard,
                             int                      aExecutionPriority,
                             hg::PZInteger            aStateBufferingLength);

    ~DefaultNetworkingManager() override;

    ///////////////////////////////////////////////////////////////////////////
    // CONFIGURATION                                                         //
    ///////////////////////////////////////////////////////////////////////////

    void setToServerMode(hg::RN_Protocol        aProtocol,
                         std::string            aPassphrase,
                         hg::PZInteger          aServerSize,
                         hg::PZInteger          aMaxPacketSize,
                         hg::RN_NetworkingStack aNetworkingStack) override;

    void setToClientMode(hg::RN_Protocol        aProtocol,
                         std::string            aPassphrase,
                         hg::PZInteger          aMaxPacketSize,
                         hg::RN_NetworkingStack aNetworkingStack) override;

    Mode getMode() const override;

    bool isUninitialized() const override;
    bool isServer() const override;
    bool isClient() const override;

    void setSendExecutionPriority(int aSendExecutionPriority) override;
    int  getSendExecutionPriority() const override;

    ///////////////////////////////////////////////////////////////////////////
    // NODE ACCESS                                                           //
    ///////////////////////////////////////////////////////////////////////////

    NodeType& getNode() const override;

    ServerType& getServer() const override;

    ClientType& getClient() const override;

    ///////////////////////////////////////////////////////////////////////////
    // LISTENER MANAGEMENT                                                   //
    ///////////////////////////////////////////////////////////////////////////

    void addEventListener(hg::NeverNull<NetworkingEventListener*> aEventListener) override;

    void removeEventListener(hg::NeverNull<NetworkingEventListener*> aEventListener) override;

    ///////////////////////////////////////////////////////////////////////////
    // SYNCHRONIZATION                                                       //
    ///////////////////////////////////////////////////////////////////////////

    hg::PZInteger getStateBufferingLength() const override;

    void setStateBufferingLength(hg::PZInteger aNewStateBufferingLength) override;

    void setPacemakerPulsePeriod(hg::PZInteger aPeriod) override;

    void setSyncCreateExeconFilter(ExeConSyncFilter aFilter) override;

    void setSyncUpdateExeconFilter(ExeConSyncFilter aFilter) override;

    void setSyncDestroyExeconFilter(ExeConSyncFilter aFilter) override;

    ExeConSyncFilter getSyncCreateExeconFilter() const override;

    ExeConSyncFilter getSyncUpdateExeconFilter() const override;

    ExeConSyncFilter getSyncDestroyExeconFilter() const override;

    void setAutomaticStateSyncForNewConnectionsEnabled(bool aEnabled) override;

    void syncCompleteStateToClient(hg::PZInteger aClientIndex, bool aCleanFirst) override;

    ///////////////////////////////////////////////////////////////////////////
    // TELEMETRY                                                             //
    ///////////////////////////////////////////////////////////////////////////

    void setTelemetryCycleLimit(hg::PZInteger aCycleLimit) override;

    hg::RN_Telemetry getTelemetry(hg::PZInteger aCycleCount) const override;

    ///////////////////////////////////////////////////////////////////////////
    // MISC.                                                                 //
    ///////////////////////////////////////////////////////////////////////////

    int getLocalClientIndex() const override;

    hg::NeverNull<void*> __spempeimpl_getRegistryAddress() override;

    SynchronizedObjectBase* mapSyncIdToObject(SyncId aSyncId) const override;

protected:
    void _didAttach(hg::QAO_Runtime& aRuntime) override;
    void _willDetach(hg::QAO_Runtime& aRuntime) override;

    void _eventPreUpdate() override;
    void _eventBeginUpdate() override;
    void _eventEndUpdate() override;

private:
    Mode _mode             = Mode::Uninitialized;
    int  _localClientIndex = CLIENT_INDEX_UNKNOWN;

    hg::QAO_GenericHandle _sendHelper;

    std::unique_ptr<NodeType>          _node;
    detail::SynchronizedObjectRegistry _syncObjReg;

    std::vector<NetworkingEventListener*> _eventListeners;

    std::deque<hg::RN_Telemetry> _telemetry;

    void _doSend();

    void onNetworkingEvent(const hg::RN_Event& aEvent) override;
};

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_MANAGERS_NETWORKING_MANAGER_DEFAULT_HPP

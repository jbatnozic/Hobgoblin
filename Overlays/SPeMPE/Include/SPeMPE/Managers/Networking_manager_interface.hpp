// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_MANAGERS_NETWORKING_MANAGER_INTERFACE_HPP
#define SPEMPE_MANAGERS_NETWORKING_MANAGER_INTERFACE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO/Execon.hpp>
#include <Hobgoblin/RigelNet.hpp>

#include <SPeMPE/GameContext/Context_components.hpp>

namespace jbatnozic {
namespace spempe {

namespace hg = ::jbatnozic::hobgoblin;

using NetworkingEventListener = hg::RN_EventListener;

constexpr int CLIENT_INDEX_UNKNOWN = -2; //! Client index not yet received from Server.
constexpr int CLIENT_INDEX_LOCAL   = -1; //! Denotes the same machine/process that's also the host.

class NetworkingManagerInterface : public ContextComponent {
public:
    virtual ~NetworkingManagerInterface() = default;

    using NodeType   = hg::RN_NodeInterface;
    using ServerType = hg::RN_ServerInterface;
    using ClientType = hg::RN_ClientInterface;
    using ExeCon     = hg::qao::QAO_ExeCon;

    ///////////////////////////////////////////////////////////////////////////
    // CONFIGURATION                                                         //
    ///////////////////////////////////////////////////////////////////////////

    enum class Mode {
        Uninitialized, //! Underlying node is a Dummy (fulfills the interface but does nothing)
        Server,        //! Underlying node is a Server
        Client,        //! Underlying node is a Client
    };

    //! Sets the manager into Host/Server mode and initializes the underlying RigelNet Node as
    //! an implementation of RN_ServerInterface with the given parameters.
    virtual void setToServerMode(hg::RN_Protocol        aProtocol,
                                 std::string            aPassphrase,
                                 hg::PZInteger          aServerSize,
                                 hg::PZInteger          aMaxPacketSize,
                                 hg::RN_NetworkingStack aNetworkingStack) = 0;

    //! Sets the manager into Client mode and initializes the underlying RigelNet Node as
    //! an implementation of RN_ClientInterface with the given parameters.
    virtual void setToClientMode(hg::RN_Protocol        aProtocol,
                                 std::string            aPassphrase,
                                 hg::PZInteger          aMaxPacketSize,
                                 hg::RN_NetworkingStack aNetworkingStack) = 0;

    virtual Mode getMode() const = 0;

    virtual bool isUninitialized() const = 0;
    virtual bool isServer() const        = 0;
    virtual bool isClient() const        = 0;

    ///////////////////////////////////////////////////////////////////////////
    // NODE ACCESS                                                           //
    ///////////////////////////////////////////////////////////////////////////

    virtual NodeType& getNode() const = 0;

    virtual ServerType& getServer() const = 0;

    virtual ClientType& getClient() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // LISTENER MANAGEMENT                                                   //
    ///////////////////////////////////////////////////////////////////////////

    virtual void addEventListener(hg::NeverNull<NetworkingEventListener*> aEventListener) = 0;

    virtual void removeEventListener(hg::NeverNull<NetworkingEventListener*> aEventListener) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // SYNCHRONIZATION                                                       //
    ///////////////////////////////////////////////////////////////////////////

    virtual hg::PZInteger getStateBufferingLength() const = 0;

    //! buffering = 0: no delay, everything displayed immediately
    //! buffering > 0: everything delayed by a number of steps to give time to
    //!                account for and amortize latency, reduce jittering etc.
    //! WARNING: This only sets the value locally! If you want to have the same
    //! buffering lengths for all players, you need to solve that problem manually
    //! (there are no hard blockers to everyone having different buffering
    //! lengths, everything will still work).
    virtual void setStateBufferingLength(hg::PZInteger aNewStateBufferingLength) = 0;

    //! Every N frames the server sends a pacemaker pulse to correct any synchronized objects
    //! that may have fallen out of sync due to lag or network delays. This method sets N.
    //! N must be at least 2, and also for technical reasons it has to be an even number.
    virtual void setPacemakerPulsePeriod(hg::PZInteger aPeriod) = 0;

    //! \brief synchronization filter based on synchronized objects' EXECON thresholds.
    //!
    //! When an instance of a class inheriting from `SynchronizedObject` is instantiated, it
    //! automatically registers itself to the context's `NetworkingManager` (or rather its associated
    //! synchronized object registry). Thus the instance's creation, and subsequent updates, and
    //! eventual destruction will all be automatically synced with all current and future connected
    //! clients (by default at least).
    //!
    //! By installing a filter, you can prevent some (or all) of this behaviour. This specific filter
    //! will prevent an instance's events from being synced if its EXECON threshold doesn't fall within
    //! a certain range.
    //!
    //! \note the default filter doesn't prevent any events from being synced.
    //! \note set both min and max to `ExeCon::META_EXECUTE_NONE` to filter out everything.
    struct ExeConSyncFilter {
        //! Filter max: instances with EXECON thresholds up to this value (inclusive) will have their
        //!             CREATE/UPDATE/DESTROY events (which ever the filter is created for) synced.
        ExeCon max = ExeCon::EXTRAS;

        //! Filter min: only instances with EXECON thresholds that meet or exceed this value will have
        //!             their CREATE/UPDATE/DESTROY events (which ever the filter is created for) synced.
        ExeCon min = ExeCon::ESSENTIAL;
    };

    //! \brief set an EXECON-based filter for instances' CREATE events.
    //! \warning the filter usually only applies to automatic synchronization, and any manually and
    //!          explicitly called `_doSyncCreate()` method will by default ignore all filters (though
    //!          this can be avoided by passing `false` for `aIgnoreFilters`).
    //! \warning ONLY USE THIS IF YOU REALLY KNOW WHAT YOU ARE DOING! Filtering out instance's CREATE
    //!          events can be really problematic unless you keep careful track of what's synced where,
    //!          and even in such cases, there are better ways to achieve this than EXECON filters.
    virtual void setSyncCreateExeconFilter(ExeConSyncFilter aFilter) = 0;

    //! \brief set an EXECON-based filter for instances' UPDATE events.
    //! \warning the filter usually only applies to automatic synchronization, and any manually and
    //!          explicitly called `_doSyncUpdate()` method will by default ignore all filters (though
    //!          this can be avoided by passing `false` for `aIgnoreFilters`).
    virtual void setSyncUpdateExeconFilter(ExeConSyncFilter aFilter) = 0;

    //! \brief set an EXECON-based filter for instances' DESTROY events.
    //! \warning the filter usually only applies to automatic synchronization, and any manually and
    //!          explicitly called `_doSyncDestroy()` method will by default ignore all filters (though
    //!          this can be avoided by passing `false` for `aIgnoreFilters`).
    //! \warning ONLY USE THIS IF YOU REALLY KNOW WHAT YOU ARE DOING! Filtering out instance's DESTROY
    //!          events can lead to dead-but-not-cleaned-up objects on the client side, with the only
    //!          way to get rid of them being a complete purge of all registered synchronized objects.
    virtual void setSyncDestroyExeconFilter(ExeConSyncFilter aFilter) = 0;

    //! \brief return the current EXECON filter for instances' CREATE events.
    virtual ExeConSyncFilter getSyncCreateExeconFilter() const = 0;

    //! \brief return the current EXECON filter for instances' UPDATE events.
    virtual ExeConSyncFilter getSyncUpdateExeconFilter() const = 0;

    //! \brief return the current EXECON filter for instances' DESTROY events.
    virtual ExeConSyncFilter getSyncDestroyExeconFilter() const = 0;

    //! Toggles whether all synchronized objects will be synchronized to a new client as soom
    //! as they are connected. By default this is enabled.
    virtual void setAutomaticStateSyncForNewConnectionsEnabled(bool aEnabled) = 0;

    //! Syncs all synchronized objects to the specified client. If 'aCleanFirst' is true, first
    //! all TODO
    virtual void syncCompleteStateToClient(hg::PZInteger aClientIndex, bool aCleanFirst) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // TELEMETRY                                                             //
    ///////////////////////////////////////////////////////////////////////////

    //! TODO
    virtual void setTelemetryCycleLimit(hg::PZInteger aCycleLimit) = 0;

    //! Returns the RigelNet telemetry for the last `aCycleCount` cycles.
    //! `aCycleCount` must not be greater than the limit set by setTelemetryCycleLimit().
    virtual hg::RN_Telemetry getTelemetry(hg::PZInteger aCycleCount) const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MISC.                                                                 //
    ///////////////////////////////////////////////////////////////////////////

    virtual int getLocalClientIndex() const = 0;

    //! \warning Internal implementation, do not call in user code!
    //!
    //! \brief return the address of the associated `SynchronizedObjectRegistry` instance.
    virtual hg::NeverNull<void*> __spempeimpl_getRegistryAddress() = 0;

private:
    SPEMPE_CTXCOMP_TAG("jbatnozic::spempe::NetworkingManagerInterface");
};

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_MANAGERS_NETWORKING_MANAGER_INTERFACE_HPP

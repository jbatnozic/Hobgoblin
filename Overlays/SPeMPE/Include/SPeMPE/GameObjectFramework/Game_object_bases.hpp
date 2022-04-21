#ifndef SPEMPE_GAME_OBJECT_FRAMEWORK_GAME_OBJECT_BASES_HPP
#define SPEMPE_GAME_OBJECT_FRAMEWORK_GAME_OBJECT_BASES_HPP

#include <Hobgoblin/QAO.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/RigelNet_Macros.hpp>
#include <Hobgoblin/Utility/Dynamic_bitset.hpp>
#include <Hobgoblin/Utility/State_scheduler_simple.hpp>
#include <SPeMPE/GameContext/Game_context.hpp>
#include <SPeMPE/GameObjectFramework/Synchronized_object_registry.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <optional>
#include <typeinfo>
#include <utility>
#include <vector>

#include <iostream>

#define SPEMPE_TYPEID_SELF (typeid(decltype(*this)))

#define SPEMPE_SINCLAIRE_DISABLED 0x80
#define SPEMPE_SINCLAIRE_YES 1
#define SPEMPE_SINCLAIRE_NO  0

namespace jbatnozic {
namespace spempe {

namespace hg = ::jbatnozic::hobgoblin;

//! Not to be inherited from directly. Use one of the classes below.
class GameObjectBase : public hg::QAO_Base {
public:
    using hg::QAO_Base::QAO_Base;

    //! Shorthand to get the GameContext in which this object lives.
    GameContext& ctx() const {
        // TODO Temporary implementation
        return *(getRuntime()->getUserData<GameContext>());
    }

    //! Shorthand to get one of the context components of the context 
    //! in which this object lives.
    template <class taComponent>
    taComponent& ccomp() {
        return ctx().getComponent<taComponent>();
    }
};

///////////////////////////////////////////////////////////////////////////
// NONSTATE OBJECTS                                                      //
///////////////////////////////////////////////////////////////////////////

//! I:
//! Objects which are not essential to the game's state and thus not saved (when
//! writing game state) nor synchronized with clients (in multiplayer sessions).
//! For example, particle effects and such.
//! II:
//! Controllers which are always created when the game starts and thus always
//! implicitly present, so we don't need to save them.
class NonstateObject : public GameObjectBase {
public:
    using GameObjectBase::GameObjectBase;
};

///////////////////////////////////////////////////////////////////////////
// STATE OBJECTS                                                         //
///////////////////////////////////////////////////////////////////////////

//! Objects which are essential for the game's state, but will not be synchronized
//! with clients. For use with singleplayer games, or for server-side controller
//! objects in multiplayer games.
class StateObject : public GameObjectBase {
public:
    using GameObjectBase::GameObjectBase;
};

///////////////////////////////////////////////////////////////////////////
// SYNCHRONIZED OBJECTS                                                  //
///////////////////////////////////////////////////////////////////////////

enum class SyncUpdateStatus {
    Normal,    //! TODO (desc)
    Skip,      //! TODO (desc)
    Deactivate //! TODO (desc)
};

struct IfMaster {};
struct IfDummy  {};

class SynchronizedObjectBase : public StateObject {
public:
    //! Big scary constructor with way too many arguments.
    SynchronizedObjectBase(hg::QAO_RuntimeRef aRuntimeRef,
                           const std::type_info& aTypeInfo,
                           int aExecutionPriority,
                           std::string aName,
                           RegistryId aRegId,
                           SyncId aSyncId);

    virtual ~SynchronizedObjectBase() override;

    SyncId getSyncId() const noexcept;
    bool isMasterObject() const noexcept;

    //! Internal implementation, do not call manually!
    void __spempeimpl_destroySelfIn(int aStepCount);

protected:
    // Call the following to sync this object's creation/update/destruction right away.

    void doSyncCreate() const;
    void doSyncUpdate() const;
    void doSyncDestroy() const;

    // These overloads will be called if the object is a Master object 
    // (that is, executing in a Privileged context).

    virtual void _eventStartFrame(IfMaster)    {}
    virtual void _eventPreUpdate(IfMaster)     {}
    virtual void _eventUpdate(IfMaster)        {}
    virtual void _eventPostUpdate(IfMaster)    {}
    virtual void _eventDraw1(IfMaster)         {}
    virtual void _eventDraw2(IfMaster)         {}
    virtual void _eventDrawGUI(IfMaster)       {}
    virtual void _eventFinalizeFrame(IfMaster) {}

    // These overloads will be called if the object is a Dummy object 
    // (that is, executing in a non-Privileged context).

    virtual void _eventStartFrame(IfDummy)    {}
    virtual void _eventPreUpdate(IfDummy)     {}
    virtual void _eventUpdate(IfDummy)         ; // This one is special!
    virtual void _eventPostUpdate(IfDummy)    {}
    virtual void _eventDraw1(IfDummy)         {}
    virtual void _eventDraw2(IfDummy)         {}
    virtual void _eventDrawGUI(IfDummy)       {}
    virtual void _eventFinalizeFrame(IfDummy) {}

    // Misc.
    bool _willDieAfterUpdate() const;

    // If you override any of the below, the overloads above will not be used.
    // The same code will be executed on both ends.

    void _eventStartFrame() override;
    void _eventPreUpdate() override;
    void _eventUpdate() override;
    void _eventPostUpdate() override;
    void _eventDraw1() override;
    void _eventDraw2() override;
    void _eventDrawGUI() override;
    void _eventFinalizeFrame() override;

    void _enableSinclaire() {
        _sinclaireState = SPEMPE_SINCLAIRE_YES;
        _setStateSchedulerDefaultDelay(_syncObjReg.getDefaultDelay() + 1);
    }

    mutable int _sinclaireState = SPEMPE_SINCLAIRE_DISABLED;

private:
    friend class detail::SynchronizedObjectRegistry;

    detail::SynchronizedObjectRegistry& _syncObjReg;
    const SyncId _syncId;

    hg::util::DynamicBitset _remoteStatuses;

    int _deathCounter = -1;

    //! Called when it's needed to sync this object's creation to one or more recepeints.
    virtual void _syncCreateImpl(SyncDetails& aSyncDetails) const = 0;

    //! Called when it's needed to sync this object's update to one or more recepeints.
    virtual void _syncUpdateImpl(SyncDetails& aSyncDetails) const = 0;

    //! Called when it's needed to sync this object's destruction to one or more recepeints.
    virtual void _syncDestroyImpl(SyncDetails& aSyncDetails) const = 0;

    virtual void _advanceDummyAndScheduleNewStates() = 0;

    virtual void _setStateSchedulerDefaultDelay(hg::PZInteger aNewDefaultDelaySteps) = 0;

    virtual void _deactivateSelfIn(hg::PZInteger aDelaySteps) = 0;
};

/*
Note:
    If an object drived from SynchronizedObject (below), and transitively from
    SynchronizedObjectBase overrides either _eventUpdate() or eventUpdate(IfDummy),
    note that the dummy object won't behave properly unless you call the following
    code at the start of its eventUpdate:
    if (!isMasterObject()) {
        const bool endOfLifetime = _willDieAfterUpdate();
        SynchronizedObjectBase::eventUpdate(IfDummy);
        if (endOfLifetime) return;
    }

    This can be expressed with the macro SPEMPE_SYNCOBJ_BEGIN_EVENT_UPDATE_OVERRIDE().
*/

#define SPEMPE_SYNCOBJ_BEGIN_EVENT_UPDATE_OVERRIDE() \
    do { if (!SynchronizedObjectBase::isMasterObject()) { \
        const bool endOfLifetime_ = SynchronizedObjectBase::_willDieAfterUpdate(); \
        SynchronizedObjectBase::_eventUpdate(::jbatnozic::spempe::IfDummy{}); \
        if (endOfLifetime_) { return; } \
    } } while (false)

//! Objects which are essential to the game's state, so they are both saved when
//! writing game state, and synchronized with clients in multiplayer sessions.
//! For example, units, terrain, interactible items (and, basically, most other 
//! game objects).
template <class taVisibleState>
class SynchronizedObject : public SynchronizedObjectBase {
public:
    using VisibleState = taVisibleState;

    bool isDeactivated() const {
        return _ssch.getCurrentState().second.isDeactivated();
    }

protected:
    using SyncObjSuper = SynchronizedObject;

    SynchronizedObject(hg::QAO_RuntimeRef aRuntimeRef,
                       const std::type_info& aTypeInfo,
                       int aExecutionPriority,
                       std::string aName,
                       RegistryId aRegId,
                       SyncId aSyncId = SYNC_ID_NEW)
        : SynchronizedObjectBase{ aRuntimeRef
                                , aTypeInfo
                                , aExecutionPriority
                                , std::move(aName)
                                , aRegId
                                , aSyncId
                                }
        , _ssch{
            isMasterObject() ? 0 // Masters don't need state scheduling
                             : reinterpret_cast<detail::SynchronizedObjectRegistry*>(
                                   aRegId.address
                               )->getDefaultDelay()
        }
    {
    }

    taVisibleState& _getCurrentState() {
        assert(isMasterObject() || !isDeactivated());
        return _ssch.getCurrentState().first;
    }

    taVisibleState& _getFollowingState() {
        assert(isMasterObject() || !isDeactivated());
        if (_ssch.getDefaultDelay() > 0) {
            return (_ssch.begin() + 1)->first;
        }
        else {
            return _ssch.getCurrentState().first;
        }
    }

    const taVisibleState& _getCurrentState() const {
        assert(isMasterObject() || !isDeactivated());
        return _ssch.getCurrentState().first;
    }

    const taVisibleState& _getFollowingState() const {
        assert(isMasterObject() || !isDeactivated());
        if (_ssch.getDefaultDelay() > 0) {
            return (_ssch.begin() + 1)->first;
        }
        else {
            return _ssch.getCurrentState().first;
        }
    }

private:
#define DUMMY_STATUS_NORMAL      0
#define DUMMY_STATUS_DEACTIVATED 1

    struct DummyStatus {
        DummyStatus(std::int8_t aStatus = DUMMY_STATUS_DEACTIVATED)
            : status{aStatus}
        {
        }

        bool isDeactivated() const {
            return (status == DUMMY_STATUS_DEACTIVATED);
        }

        std::int8_t status;
    };

    using SchedulerPair = std::pair<taVisibleState, DummyStatus>;

    hg::util::SimpleStateScheduler<SchedulerPair> _ssch;

    struct DeferredState {
        taVisibleState state;
        hg::PZInteger delay;
    };
    std::optional<DeferredState> _sinclaireDeferredState;

    bool _sinclaireMark = false; // true => Align on next update
    hg::PZInteger _sinclaireMarkDelay = 0;

    void _advanceDummyAndScheduleNewStates() override final {
        _ssch.advance();

        if (_sinclaireMark) {
            _ssch.alignToDelay(_sinclaireMarkDelay);
            _sinclaireMark = false;
        }

        if (_sinclaireDeferredState.has_value()) {
            _ssch.putNewState(SchedulerPair{(*_sinclaireDeferredState).state, {DUMMY_STATUS_NORMAL}},
                                (*_sinclaireDeferredState).delay);
            _sinclaireDeferredState.reset();
        }

        _ssch.scheduleNewStates();
    }

    void _setStateSchedulerDefaultDelay(hg::PZInteger aNewDefaultDelaySteps) override final {
        if (!isMasterObject()) {
            _ssch.setDefaultDelay(aNewDefaultDelaySteps);
        }
    }

    void _deactivateSelfIn(hg::PZInteger aDelaySteps) override final {
        _ssch.putNewState(SchedulerPair{{}, {DUMMY_STATUS_DEACTIVATED}}, aDelaySteps);
    }

public:
    //! Internal implementation, do not call manually!
    void __spempeimpl_applyUpdate(const VisibleState& aNewState, hg::PZInteger aDelaySteps) {
        if (_sinclaireState == SPEMPE_SINCLAIRE_DISABLED) {
            _ssch.putNewState(SchedulerPair{aNewState, {DUMMY_STATUS_NORMAL}}, aDelaySteps);
            _sinclaireMarkDelay = aDelaySteps;
        }
        else {
            if (_sinclaireDeferredState.has_value()) {
                _ssch.putNewState(SchedulerPair{(*_sinclaireDeferredState).state, {DUMMY_STATUS_NORMAL}},
                                  (*_sinclaireDeferredState).delay);
                _sinclaireDeferredState.reset();
            }

            _ssch.putNewState(SchedulerPair{aNewState, {DUMMY_STATUS_NORMAL}}, aDelaySteps);

            _sinclaireDeferredState = {aNewState, aDelaySteps};
            _sinclaireMarkDelay = aDelaySteps;
        }
#if 0
        if (_sinclaireState != SPEMPE_SINCLAIRE_DISABLED) {
            if (_sinclaireFlag && _ssch.getBlueStateCount() < 1) {
                return;
            }
        }

        _ssch.putNewState(SchedulerPair{aNewState, {DUMMY_STATUS_NORMAL}}, aDelaySteps);

        if (_sinclaireState != SPEMPE_SINCLAIRE_DISABLED) {
            _sinclaireDeferredState = {aNewState, aDelaySteps};
        }
#endif
    }

    //! Internal implementation, do not call manually!
    void __spempeimpl_applyCatchupUpdate(const VisibleState& aNewState, hg::PZInteger aDelaySteps) {
        if (_sinclaireState == SPEMPE_SINCLAIRE_DISABLED) {
            _ssch.putNewState(SchedulerPair{aNewState, {DUMMY_STATUS_NORMAL}}, aDelaySteps);
        }
        _sinclaireMark = true;
#if 0
        if (_ssch.getBlueStateCount() > 2) {
            return;
        }

        //std::cout << "Catchup!\n";

        const auto oldCount = _ssch.getBlueStateCount();
        //while (_ssch.getBlueStateCount() > std::min((int)_ssch.getDefaultDelay() - (int)aDelaySteps, -1)) {
        while (_ssch.getBlueStateCount() > 2) {
            _ssch.advance();
        }
        const auto newCount = _ssch.getBlueStateCount();

        std::cout << "Catchup! (" << oldCount << " -> " << newCount << ")\n";
        
        _ssch.putNewState(SchedulerPair{aNewState, {DUMMY_STATUS_NORMAL}}, aDelaySteps);

        if (_sinclaireState != SPEMPE_SINCLAIRE_DISABLED) {
            _sinclaireDeferredState = {aNewState, aDelaySteps};
        }
#endif
#if 0
        if (_sinclaireState != SPEMPE_SINCLAIRE_DISABLED) {
            if (_ssch.getBlueStateCount() > 2) {
                return;
            }
        }

        std::cout << "Catchup!\n";

        //if (_sinclaireDeferredState.has_value()) {
        //    _ssch.putNewState(SchedulerPair{(*_sinclaireDeferredState).state, {DUMMY_STATUS_NORMAL}},
        //                      (*_sinclaireDeferredState).delay);
        //    _sinclaireDeferredState.reset();
        //}

        _ssch.putNewState(SchedulerPair{aNewState, {DUMMY_STATUS_NORMAL}}, aDelaySteps);

        if (_sinclaireState != SPEMPE_SINCLAIRE_DISABLED) {
            //if (_ssch.getBlueStateCount() != 1) {
            _sinclaireDeferredState = {aNewState, aDelaySteps};
            //}
        }
#endif
    }

#undef DUMMY_STATUS_NORMAL
#undef DUMMY_STATUS_DEACTIVATED
#undef DUMMY_STATUS_DESTROY
};

///////////////////////////////////////////////////////////////////////////
// SYNCHRONIZATION HELPERS                                               //
///////////////////////////////////////////////////////////////////////////

namespace detail {

#define ROUNDTOI(_x_) (static_cast<int>(std::round(_x_)))

template <class taContext, class taNetwMgr>
struct SyncParameters {
    //! Reference to game context (spempe::GameContext)
    taContext& context;

    //! Reference to instance of spempe::NetworkingManagerInterface
    taNetwMgr& netwMgr;

    //! Index of the sender (always -1000 on client)
    int senderIndex;

    //! Latency to the sender
    std::chrono::microseconds meanLatency;
    std::chrono::microseconds optimisticLatency;
    std::chrono::microseconds pessimisticLatency;

    //! Latency in steps (approximately; calculated in regards to desired
    //! framerate in the context's runtime config).
    hg::PZInteger meanLatencyInSteps;
    hg::PZInteger optimisticLatencyInSteps;
    hg::PZInteger pessimisticLatencyInSteps;

    explicit SyncParameters(const hg::RN_ClientInterface& aClient)
        : context{*aClient.getUserDataOrThrow<taContext>()}
        , netwMgr{context.getComponent<taNetwMgr>()}
        , senderIndex{-1000}
        , meanLatency{aClient.getServerConnector().getRemoteInfo().meanLatency}
        , optimisticLatency{aClient.getServerConnector().getRemoteInfo().optimisticLatency }
        , pessimisticLatency{aClient.getServerConnector().getRemoteInfo().pessimisticLatency }
        , meanLatencyInSteps{ROUNDTOI(meanLatency / context.getRuntimeConfig().deltaTime)}
        , optimisticLatencyInSteps{ROUNDTOI(optimisticLatency / context.getRuntimeConfig().deltaTime)}
        , pessimisticLatencyInSteps{ROUNDTOI(pessimisticLatency / context.getRuntimeConfig().deltaTime)}
    {
    }

    explicit SyncParameters(const hg::RN_ServerInterface& aServer)
        : context{*aServer.getUserDataOrThrow<taContext>()}
        , netwMgr{context.getComponent<taNetwMgr>()}
        , senderIndex{aServer.getSenderIndex()}
        , meanLatency{aServer.getClientConnector(senderIndex).getRemoteInfo().meanLatency}
        , optimisticLatency{aServer.getClientConnector(senderIndex).getRemoteInfo().optimisticLatency}
        , pessimisticLatency{aServer.getClientConnector(senderIndex).getRemoteInfo().pessimisticLatency}
        , meanLatencyInSteps{ROUNDTOI(meanLatency / context.getRuntimeConfig().deltaTime)}
        , optimisticLatencyInSteps{ROUNDTOI(optimisticLatency / context.getRuntimeConfig().deltaTime)}
        , pessimisticLatencyInSteps{ROUNDTOI(pessimisticLatency / context.getRuntimeConfig().deltaTime)}
    {
    }
};

#undef ROUNDTOI

template <class taContext, class taNetwMgr>
SyncParameters<taContext, taNetwMgr> GetSyncParams(hg::RN_ClientInterface& aClient) {
    return SyncParameters<taContext, taNetwMgr>{aClient};
}

template <class taContext, class taNetwMgr>
SyncParameters<taContext, taNetwMgr> GetSyncParams(hg::RN_ServerInterface& aServer) {
    return SyncParameters<taContext, taNetwMgr>{aServer};
}

} // namespace detail

template <class taSyncObj, class taContext, class taNetwMgr>
void DefaultSyncCreateHandler(hg::RN_NodeInterface& node, 
                              SyncId syncId) {
    node.callIfClient(
        [&](hg::RN_ClientInterface& client) {
            auto& ctx     = *client.getUserDataOrThrow<taContext>();
            auto& runtime = ctx.getQAORuntime();
            auto  regId   = ctx.getComponent<taNetwMgr>().getRegistryId();

            hg::QAO_PCreate<taSyncObj>(&runtime, regId, syncId);
        });

    node.callIfServer(
        [](hg::RN_ServerInterface& server) {
            throw hg::RN_IllegalMessage("Server received a sync message");
        });
}

template <class taSyncObj, class taContext, class taNetwMgr>
void DefaultSyncUpdateHandler(hg::RN_NodeInterface& node,
                              SyncId syncId,
                              bool isCatchupFrame,
                              typename taSyncObj::VisibleState& state) {
    node.callIfClient(
        [&](hg::RN_ClientInterface& client) {
            detail::SyncParameters<taContext, taNetwMgr> sp{client};
            auto  regId      = sp.netwMgr.getRegistryId();
            auto& syncObjReg = *reinterpret_cast<detail::SynchronizedObjectRegistry*>(regId.address);
            auto& object     = *static_cast<taSyncObj*>(syncObjReg.getMapping(syncId));

            if (!isCatchupFrame) {
                object.__spempeimpl_applyUpdate(state, sp.pessimisticLatencyInSteps);
            }
            else {
                object.__spempeimpl_applyCatchupUpdate(state, sp.pessimisticLatencyInSteps);
            }
        });

    node.callIfServer(
        [](hg::RN_ServerInterface& server) {
            throw hg::RN_IllegalMessage("Server received a sync message");
        });
}

template <class taSyncObj, class taContext, class taNetwMgr>
void DefaultSyncDestroyHandler(hg::RN_NodeInterface& node, 
                               SyncId syncId) {
    node.callIfClient(
        [&](hg::RN_ClientInterface& client) {
            detail::SyncParameters<taContext, taNetwMgr> sp{client};
            auto  regId      = sp.context.getComponent<taNetwMgr>().getRegistryId();
            auto& syncObjReg = *reinterpret_cast<detail::SynchronizedObjectRegistry*>(regId.address);
            auto* object     = static_cast<taSyncObj*>(syncObjReg.getMapping(syncId));

            object->__spempeimpl_destroySelfIn(
                static_cast<int>(syncObjReg.getDefaultDelay()) - (sp.meanLatencyInSteps + 1));
        });

    node.callIfServer(
        [](hg::RN_ServerInterface& server) {
            throw hg::RN_IllegalMessage("Server received a sync message");
        });
}

// ===== All macros starting with USPEMPE_ are internal =====

#define USPEMPE_MACRO_CONCAT_WITHARG(_x_, _y_, _arg_) _x_##_y_(_arg_)
#define USPEMPE_MACRO_EXPAND(_x_) _x_
#define USPEMPE_MACRO_EXPAND_VA(...) __VA_ARGS__

#define USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_EMPTY(_class_name_) /* Empty */

#define USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_CREATE(_class_name_) \
    RN_DEFINE_RPC(USPEMPE_Create##_class_name_, \
                  RN_ARGS(::jbatnozic::spempe::SyncId, syncId)) { \
        ::jbatnozic::spempe::DefaultSyncCreateHandler<_class_name_, \
                                                      ::jbatnozic::spempe::GameContext, \
                                                      ::jbatnozic::spempe::NetworkingManagerInterface>( \
            RN_NODE_IN_HANDLER(), syncId); \
    }

#define USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_UPDATE(_class_name_) \
    RN_DEFINE_RPC(USPEMPE_Update##_class_name_, \
                  RN_ARGS(::jbatnozic::spempe::SyncId, syncId, bool, catchup, _class_name_::VisibleState&, state)) { \
        ::jbatnozic::spempe::DefaultSyncUpdateHandler<_class_name_, \
                                                      ::jbatnozic::spempe::GameContext, \
                                                      ::jbatnozic::spempe::NetworkingManagerInterface>( \
            RN_NODE_IN_HANDLER(), syncId, catchup, state); \
    }

#define USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_DESTROY(_class_name_) \
    RN_DEFINE_RPC(USPEMPE_Destroy##_class_name_, \
                  RN_ARGS(::jbatnozic::spempe::SyncId, syncId)) { \
        ::jbatnozic::spempe::DefaultSyncDestroyHandler<_class_name_, \
                                                       ::jbatnozic::spempe::GameContext, \
                                                       ::jbatnozic::spempe::NetworkingManagerInterface>( \
            RN_NODE_IN_HANDLER(), syncId); \
    }

#define USPEMPE_GENERATE_MULTIPLE_DEFAULT_SYNC_HANDLERS(_class_name_, _tag_1_, _tag_2_, _tag_3_, ...) \
    USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_##_tag_1_ (_class_name_) \
    USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_##_tag_2_ (_class_name_) \
    USPEMPE_GENERATE_DEFAULT_SYNC_HANDLER_##_tag_3_ (_class_name_)

#define USPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(_class_name_, ...) \
    USPEMPE_MACRO_EXPAND( \
        USPEMPE_GENERATE_MULTIPLE_DEFAULT_SYNC_HANDLERS(_class_name_, __VA_ARGS__, EMPTY, EMPTY) \
    )

//! Create default synchronization handlers for class _class_name_.
//! Events should be written together in parenthesis and can be CREATE, UPDATE and DESTROY.
//! Example of usage:
//!     SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(MyClass, (CREATE, DESTROY));
#define SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(_class_name_, _for_events_) \
    USPEMPE_MACRO_EXPAND( \
        USPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(_class_name_, USPEMPE_MACRO_EXPAND_VA _for_events_) \
    )

//! TODO (add description)
#define SPEMPE_SYNC_CREATE_DEFAULT_IMPL(_class_name_, _sync_details_) \
    Compose_USPEMPE_Create##_class_name_(_sync_details_.getNode(), \
                                         _sync_details_.getRecepients(), getSyncId())

//! TODO (add description)
#define SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(_class_name_, _sync_details_) \
    Compose_USPEMPE_Update##_class_name_(_sync_details_.getNode(), \
                                         _sync_details_.getRecepients(), \
                                         getSyncId(), \
                                         _sync_details_.isCatchupFrame(), \
                                         _getCurrentState())

//! TODO (add description)
#define SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(_class_name_, _sync_details_) \
    Compose_USPEMPE_Destroy##_class_name_(_sync_details_.getNode(), \
                                          _sync_details_.getRecepients(), getSyncId())

//! TODO (add description)
#define SPEMPE_GET_SYNC_PARAMS(_node_) \
    (::jbatnozic::spempe::detail::GetSyncParams<::jbatnozic::spempe::GameContext, \
                                                ::jbatnozic::spempe::NetworkingManagerInterface>(_node_))

} // namespace spempe
} // namespace jbatnozic

#include <SPeMPE/Managers/Networking_manager_interface.hpp>

#endif // !SPEMPE_GAME_OBJECT_FRAMEWORK_GAME_OBJECT_BASES_HPP

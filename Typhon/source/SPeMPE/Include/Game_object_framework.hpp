#ifndef SPEMPE_GAME_OBJECT_FRAMEWORK_HPP
#define SPEMPE_GAME_OBJECT_FRAMEWORK_HPP

#include <Hobgoblin/QAO.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/RigelNet_Macros.hpp>
#include <SPeMPE/Include/Synchronized_object_registry.hpp>

#include <typeinfo>

#define SPEMPE_TYPEID_SELF (typeid(decltype(*this)))

namespace spempe {

class GameContext;
class KbInputTracker;
class NetworkingManager;
class WindowManager;

class GameObjectBase : public hg::QAO_Base {
public:
    using hg::QAO_Base::QAO_Base;

    enum KeyboardInputTagEnum     { IKeyboard };
    enum NetworkingManagerTagEnum { MNetworking };
    enum WindowManagertagEnum     { MWindow };
    
    GameContext&       ctx() const;
    KbInputTracker&    ctx(KeyboardInputTagEnum) const;
    NetworkingManager& ctx(NetworkingManagerTagEnum) const;
    WindowManager&     ctx(WindowManagertagEnum) const;
};

// I:
// Objects which are not essential to the game's state and thus not saved (when
// writing game state) nor synchronized with clients (in multiplayer sessions).
// For example, particle effects and such.
// II:
// Controllers which are always created when the game starts and thus always
// implicitly present, so we don't need to save them.
class NonstateObject : public GameObjectBase {
public:
    using GameObjectBase::GameObjectBase;
};

// Objects which are essential for the game's state, but will not be synchronized
// with clients. For use with singleplayer games, or for server-side controller
// objects in multiplayer games.
class StateObject : public GameObjectBase {
public:
    using GameObjectBase::GameObjectBase;
};

// Objects which are essential to the game's state, so they are both saved when
// writing game state, and synchronized with clients in multiplayer sessions.
// For example, units, terrain, interactible items (and, basically, most other 
// game objects).
class SynchronizedObject : public StateObject {
public:
    // TODO: Implement methods in .cpp file

    SynchronizedObject(hg::QAO_RuntimeRef runtimeRef, const std::type_info& typeInfo,
                       int executionPriority, std::string name,
                       SynchronizedObjectRegistry& syncObjReg, SyncId syncId = SYNC_ID_NEW);

    virtual ~SynchronizedObject();

    SyncId getSyncId() const noexcept;

    bool isMasterObject() const noexcept;

protected:
    void syncCreate() const;
    void syncUpdate() const;
    void syncDestroy() const;

private:
    SynchronizedObjectRegistry& _syncObjReg;
    const SyncId _syncId;

    virtual void syncCreateImpl(hg::RN_Node& node, const std::vector<hg::PZInteger>& rec) const = 0;
    virtual void syncUpdateImpl(hg::RN_Node& node, const std::vector<hg::PZInteger>& rec) const = 0;
    virtual void syncDestroyImpl(hg::RN_Node& node, const std::vector<hg::PZInteger>& rec) const = 0;

    friend class SynchronizedObjectRegistry;
};

// ///////////////////////////////////////////////////////////////////////////////////////////// //
// CANNONICAL FORMS

// A Synchronized object has the cannonical forms if it fulfils the following criteria:
// + Inherits from GOF_SynchronizedObject
// + Publicly defines a nested struct named "VisibleState" which has defined operators << and >> for insertion and
//   extraction to and from a hg::util::Packet
// + Its entire visible (observable by the user/player) state can be described with an instance of the VisibleState
//   struct
// + It defines a constructor with the following form:
//   ClassName(QAO_RuntimeRef rtRef, SynchronizedObjectRegistry& syncObjReg,
//             SyncId syncId, const VisibleState& initialState);
//   (NOTE: to overcome the limitation of the fixed constructor form, we can utilize the two-step initialization
//   pattern using init- methods)
// + Is always created on the heap using QAO_PCreate or QAO_ICreate
// + Publicly defnes a method void cannonicalSyncApplyUpdate(const VisibleState& state, int delay) which will be
//   called when a new state description is received from the server
// + Defines a method (with any access modifier) with the following signature:
//   const VisibleState& getCurrentState() const;
//
// If a Synchronized object has the cannonical form, the following macros can be used to generate appropriate
// network handlers for that type

#define SPEMPE_GENERATE_CANNONICAL_HANDLERS(_class_name_) \
    RN_DEFINE_HANDLER(Create##_class_name_, RN_ARGS(::spempe::SyncId, syncId, _class_name_::VisibleState&, state)) { \
        ::spempe::CannonicalCreateImpl<_class_name_, ::spempe::GameContext>(RN_NODE_IN_HANDLER(), syncId, state); \
    } \
    RN_DEFINE_HANDLER(Update##_class_name_, RN_ARGS(::spempe::SyncId, syncId, _class_name_::VisibleState&, state)) { \
        ::spempe::CannonicalUpdateImpl<_class_name_, ::spempe::GameContext>(RN_NODE_IN_HANDLER(), syncId, state); \
    } \
    RN_DEFINE_HANDLER(Destroy##_class_name_, RN_ARGS(::spempe::SyncId, syncId)) { \
        ::spempe::CannonicalDestroyImpl<_class_name_, ::spempe::GameContext>(RN_NODE_IN_HANDLER(), syncId); \
    }

#define SPEMPE_GENERATE_CANNONICAL_SYNC_DECLARATIONS \
    void syncCreateImpl(::hg::RN_Node& node, const std::vector<hg::PZInteger>& rec) const override; \
    void syncUpdateImpl(::hg::RN_Node& node, const std::vector<hg::PZInteger>& rec) const override; \
    void syncDestroyImpl(::hg::RN_Node& node, const std::vector<hg::PZInteger>& rec) const override;

#define SPEMPE_GENERATE_CANNONICAL_SYNC_IMPLEMENTATIONS(_class_name_) \
    void _class_name_::syncCreateImpl(::hg::RN_Node& node, const std::vector<::hg::PZInteger>& rec) const { \
        Compose_Create##_class_name_(node, rec, getSyncId(), getCurrentState()); \
    } \
    void _class_name_::syncUpdateImpl(::hg::RN_Node& node, const std::vector<::hg::PZInteger>& rec) const { \
        Compose_Update##_class_name_(node, rec, getSyncId(), getCurrentState()); \
    } \
    void _class_name_::syncDestroyImpl(::hg::RN_Node& node, const std::vector<::hg::PZInteger>& rec) const { \
        Compose_Destroy##_class_name_(node, rec, getSyncId()); \
    }

// TODO Use NetworkingManager::Server/ClientType in cannonical handlers

template <class T, class TCtx>
void CannonicalCreateImpl(hg::RN_Node& node, SyncId syncId, typename T::VisibleState& state) {
    node.visit(
        [&](hg::RN_UdpClient& client) {
            auto& ctx = *client.getUserData<TCtx>();
            auto& runtime = ctx.getQaoRuntime();
            auto& syncObjReg = ctx.getSyncObjReg();

            hg::QAO_PCreate<T>(&runtime, syncObjReg, syncId, state);
        },
        [](hg::RN_UdpServer& server) {
            // TODO ERROR
        }
    );
}

template <class T, class TCtx>
void CannonicalUpdateImpl(hg::RN_Node& node, SyncId syncId, typename T::VisibleState& state) {
    node.visit(
        [&](hg::RN_UdpClient& client) {
            auto& ctx = *client.getUserData<TCtx>();
            auto& runtime = ctx.getQaoRuntime();
            auto& syncObjReg = ctx.getSyncObjReg();
            auto& object = *static_cast<T*>(syncObjReg.getMapping(syncId));

            const std::chrono::microseconds delay = client.getServer().getRemoteInfo().latency / 2LL;
            object.cannonicalSyncApplyUpdate(state, ctx.calcDelay(delay));
        },
        [](hg::RN_UdpServer& server) {
            // TODO ERROR
        }
    );
}

template <class T, class TCtx>
void CannonicalDestroyImpl(hg::RN_Node& node, SyncId syncId) {
    node.visit(
        [&](hg::RN_UdpClient& client) {
            auto& ctx = *client.getUserData<TCtx>();
            auto& runtime = ctx.getQaoRuntime();
            auto& syncObjReg = ctx.getSyncObjReg();
            auto* object = static_cast<T*>(syncObjReg.getMapping(syncId));

            hg::QAO_PDestroy(object);
        },
        [](hg::RN_UdpServer& server) {
            // TODO ERROR
        }
    );
}

} // namespace spempe 

#endif // !SPEMPE_GAME_OBJECT_FRAMEWORK_HPP
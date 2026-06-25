// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_GAME_OBJECT_FRAMEWORK_SYNCHRONIZED_OBJECT_REGISTRY_HPP
#define SPEMPE_GAME_OBJECT_FRAMEWORK_SYNCHRONIZED_OBJECT_REGISTRY_HPP

#include <SPeMPE/GameObjectFramework/Sync_control_delegate.hpp>
#include <SPeMPE/GameObjectFramework/Sync_id.hpp>
#include <SPeMPE/Managers/Networking_manager_interface.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>
#include <Hobgoblin/Utility/Packet.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace jbatnozic {
namespace spempe {

namespace hg = ::jbatnozic::hobgoblin;

class SynchronizedObjectBase;

namespace detail {

class SynchronizedObjectRegistry
    : public hg::util::NonCopyable
    , public hg::util::NonMoveable {
public:
    SynchronizedObjectRegistry(hg::RN_NodeInterface& node, hg::PZInteger defaultDelay);

    void                  setNode(hg::RN_NodeInterface& node);
    hg::RN_NodeInterface& getNode() const;

    SyncId registerMasterObject(SynchronizedObjectBase* object);
    void   registerDummyObject(SynchronizedObjectBase* object, SyncId masterSyncId);
    void   unregisterObject(SynchronizedObjectBase* object);

    void destroyAllRegisteredObjects();

    using ExeConSyncFilter = NetworkingManagerInterface::ExeConSyncFilter;

    void setSyncCreateExeconFilter(ExeConSyncFilter aFilter);
    void setSyncUpdateExeconFilter(ExeConSyncFilter aFilter);
    void setSyncDestroyExeconFilter(ExeConSyncFilter aFilter);

    ExeConSyncFilter getSyncCreateExeconFilter() const;
    ExeConSyncFilter getSyncUpdateExeconFilter() const;
    ExeConSyncFilter getSyncDestroyExeconFilter() const;

    //! Can return nullptr if there is no matching object.
    SynchronizedObjectBase* getMapping(SyncId syncId) const;

    void syncObjectCreate(const SynchronizedObjectBase* object, bool aIgnoreFilters);
    void syncObjectUpdate(const SynchronizedObjectBase* object, bool aIgnoreFilters);
    void syncObjectDestroy(const SynchronizedObjectBase* object, bool aIgnoreFilters);

    //! \brief call this once per update step to keep the registry up to date
    void update();
    void syncStateUpdates();
    void syncCompleteState(hg::PZInteger clientIndex);

    hg::PZInteger getDefaultDelay() const;
    hg::PZInteger adjustDelayForLag(hg::PZInteger aDelay) const;

    //! Sets the default delay (state buffering length) for all current and future
    //! registered synchronized objects.
    void setDefaultDelay(hg::PZInteger aNewDefaultDelaySteps);

    //! \param aPeriod must be at least 1
    void setPacemakerPulsePeriod(hg::PZInteger aPeriod);

    //! Checks the alternating updates flag:
    //! - true:  objects with alternating updates have synced in this cycle.
    //! - false: objects with alternating updates have NOT synced in this cycle.
    //! Note: this is meant to be called during _eventPostUpdate(), otherwise
    //! the returned value may not be reliable.
    bool getAlternatingUpdatesFlag() const;

    ///////////////////////////////////////////////////////////////////////////
    // HELPERS & ACCESSORS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    void deactivateObject(SyncId aObjectId, hg::PZInteger aDelayInSteps);

private:
    std::unordered_map<SyncId, SynchronizedObjectBase*> _mappings;
    std::unordered_set<const SynchronizedObjectBase*>   _newlyCreatedObjects;
    std::unordered_set<const SynchronizedObjectBase*>   _alreadyUpdatedObjects;
    std::unordered_set<const SynchronizedObjectBase*>   _alreadyDestroyedObjects;

    ExeConSyncFilter _execonCreateFilter;
    ExeConSyncFilter _execonUpdateFilter;
    ExeConSyncFilter _execonDestroyFilter;

    hg::NeverNull<hg::RN_NodeInterface*> _node;
    SyncControlDelegate                  _syncControlDelegate;

    SyncId        _syncIdCounter = 2;
    hg::PZInteger _defaultDelay;

    hg::PZInteger _pacemakerPulsePeriod  = 60;
    std::uint32_t _pacemakerPulseCounter = 0;
    bool          _alternatingUpdateFlag = true;

    static void Align(const SynchronizedObjectBase* aObject,
                      const SyncControlDelegate&    aSyncCtrl,
                      hg::RN_NodeInterface&         aLocalNode);
};

} // namespace detail

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_GAME_OBJECT_FRAMEWORK_SYNCHRONIZED_OBJECT_REGISTRY_HPP

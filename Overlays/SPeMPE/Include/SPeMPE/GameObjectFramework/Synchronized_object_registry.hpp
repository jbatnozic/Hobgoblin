#ifndef SPEMPE_GAME_OBJECT_FRAMEWORK_SYNCHRONIZED_OBJECT_REGISTRY_HPP
#define SPEMPE_GAME_OBJECT_FRAMEWORK_SYNCHRONIZED_OBJECT_REGISTRY_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace jbatnozic {
namespace spempe {

namespace hg = ::jbatnozic::hobgoblin;

using SyncId = std::uint64_t;
constexpr SyncId SYNC_ID_NEW = 0;

struct RegistryId {
    std::intptr_t address;
};

namespace detail {
class SynchronizedObjectRegistry;
} // namespace detail

struct SyncDetails {
    //! The node through which the sync messages need to be sent.
    hg::RN_NodeInterface& getNode() const;

    //! Vector of client indices of (recepients) to which sync messages
    //! need to be sent.
    const std::vector<hg::PZInteger>& getRecepients() const;

    bool isCatchupFrame() const {
        return _alignFrame;
    }

    enum class FilterResult {
        FullSync,   //! Sync whole state of this object (default behaviour)
        Skip,       //! Don't send anything during this update (no change)
        Deactivate  //! Don't send anything and deactivate the remote dummy
    };

    //! Function object that takes an index of a client as an argument ans should
    //! return whether (and how) to sync an object to this particular client.
    using FilterPrecidateFunc = std::function<FilterResult(hg::PZInteger)>;

    void filterSyncs(const FilterPrecidateFunc& aPredicate);

private:
    SyncDetails(detail::SynchronizedObjectRegistry& aRegistry);

    hg::not_null<detail::SynchronizedObjectRegistry*> _registry;
    std::vector<hg::PZInteger> _recepients;
    SyncId _forObject = SYNC_ID_NEW;
    bool _alignFrame = false;

    friend class detail::SynchronizedObjectRegistry;
};

class SynchronizedObjectBase;

namespace detail {

class SynchronizedObjectRegistry 
    : public hg::util::NonCopyable
    , public hg::util::NonMoveable {
public:
    SynchronizedObjectRegistry(hg::RN_NodeInterface& node, hg::PZInteger defaultDelay);

    void setNode(hg::RN_NodeInterface& node);
    hg::RN_NodeInterface& getNode() const;

    SyncId registerMasterObject(SynchronizedObjectBase* object);
    void registerDummyObject(SynchronizedObjectBase* object, SyncId masterSyncId);
    void unregisterObject(SynchronizedObjectBase* object);

    SynchronizedObjectBase* getMapping(SyncId syncId) const;

    void syncObjectCreate(const SynchronizedObjectBase* object);
    void syncObjectUpdate(const SynchronizedObjectBase* object);
    void syncObjectDestroy(const SynchronizedObjectBase* object);

    void syncStateUpdates();
    void syncCompleteState(hg::PZInteger clientIndex);

    hg::PZInteger getDefaultDelay() const;

    //! Sets the default delay (state buffering length) for all current and future
    //! registered synchronized objects.
    void setDefaultDelay(hg::PZInteger aNewDefaultDelaySteps);

    void deactivateObject(SyncId aObjectId, hg::PZInteger aDelayInSteps);

    bool isObjectDeactivatedForClient(SyncId aObjectId, hg::PZInteger aForClient);

    void setObjectDeactivatedFlagForClient(SyncId aObjectId, hg::PZInteger aForClient, bool aFlag);

private:
    std::unordered_map<SyncId, SynchronizedObjectBase*> _mappings;
    std::unordered_set<const SynchronizedObjectBase*> _newlyCreatedObjects;
    std::unordered_set<const SynchronizedObjectBase*> _alreadyUpdatedObjects;
    std::unordered_set<const SynchronizedObjectBase*> _alreadyDestroyedObjects;

    hg::not_null<hg::RN_NodeInterface*> _node;
    SyncDetails _syncDetails;

    SyncId _syncIdCounter = 2;
    hg::PZInteger _defaultDelay;

    bool _sinclaireDoSync = true;
    hg::PZInteger _sinclaireInversionCountdown = 15;
};

} // namespace detail

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_GAME_OBJECT_FRAMEWORK_SYNCHRONIZED_OBJECT_REGISTRY_HPP


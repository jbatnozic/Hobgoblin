// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "SPeMPE/GameObjectFramework/Sync_id.hpp"
#include <Hobgoblin/HGExcept.hpp>

#include <SPeMPE/GameContext/Game_context.hpp>
#include <SPeMPE/GameObjectFramework/Game_object_bases.hpp>
#include <SPeMPE/GameObjectFramework/Synchronized_object_registry.hpp>
#include <SPeMPE/Managers/Networking_manager_interface.hpp>

#include <algorithm>

namespace jbatnozic {
namespace spempe {

///////////////////////////////////////////////////////////////////////////
// MARK: SYNCHRONIZED OBJECT BASE                                        //
///////////////////////////////////////////////////////////////////////////

SynchronizedObjectBase::SynchronizedObjectBase(hg::QAO_InstGuard     aInstGuard,
                                               const std::type_info& aTypeInfo,
                                               int                   aExecutionPriority,
                                               std::string           aName,
                                               SyncId                aSyncId)
    : StateObject{aInstGuard, aTypeInfo, aExecutionPriority, std::move(aName)}
    , _syncId(aSyncId) {}

SynchronizedObjectBase::~SynchronizedObjectBase() = default;

void SynchronizedObjectBase::_didAttach(hg::QAO_Runtime& aRuntime) {
    StateObject::_didAttach(aRuntime);

    auto& context = *aRuntime.getUserData<GameContext>();
    auto& netMgr  = context.getComponent<NetworkingManagerInterface>();

    _syncObjReg = static_cast<detail::SynchronizedObjectRegistry*>(
        netMgr.__spempeimpl_getRegistryAddress().copy());

    if (context.isPrivileged()) {
        assert(_syncId == SYNC_ID_NEW);
        _syncId = _syncObjReg->registerMasterObject(this);
    } else {
        _syncObjReg->registerDummyObject(this, _syncId);
    }

    _setStateSchedulerDefaultDelay(_syncObjReg->getDefaultDelay());
}

void SynchronizedObjectBase::_willDetach(hg::QAO_Runtime& aRuntime) {
    if (isMasterObject()) {
        _doSyncDestroy();
    }

    _syncObjReg->unregisterObject(this);

    _syncObjReg = nullptr;
    _syncId     = SYNC_ID_NEW;

    StateObject::_willDetach(aRuntime);
}

//! Lowest bit of SyncId 1 on master objects and 0 on dummy objects
constexpr SyncId SYNC_ID_1 = 1;

SyncId SynchronizedObjectBase::getSyncId() const noexcept {
    return _syncId & ~SYNC_ID_1;
}

bool SynchronizedObjectBase::isMasterObject() const noexcept {
    return ((_syncId & SYNC_ID_1) != 0);
}

bool SynchronizedObjectBase::isUsingAlternatingUpdates() const noexcept {
    return _alternatingUpdatesEnabled;
}

void SynchronizedObjectBase::__spempeimpl_destroySelfIn(int aStepCount) {
    if (aStepCount <= 0) {
        _deathCounter = 0;
        return;
    }

    if (_deathCounter >= 0 && _deathCounter < aStepCount) {
        return;
    }

    _deathCounter = aStepCount;
}

void SynchronizedObjectBase::_doSyncCreate() const {
    if (!isMasterObject()) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Dummy objects cannot request synchronization.");
    }
    _syncObjReg->syncObjectCreate(this);
}

void SynchronizedObjectBase::_doSyncUpdate() const {
    if (!isMasterObject()) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Dummy objects cannot request synchronization.");
    }
    _syncObjReg->syncObjectUpdate(this);
}

void SynchronizedObjectBase::_doSyncDestroy() const {
    if (!isMasterObject()) {
        HG_THROW_TRACED(hg::TracedLogicError, 0, "Dummy objects cannot request synchronization");
    }
    _syncObjReg->syncObjectDestroy(this);
}

bool SynchronizedObjectBase::_willUpdateDeleteThis() const {
    return (_deathCounter == 0);
}

void SynchronizedObjectBase::_enableAlternatingUpdates() {
    _alternatingUpdatesEnabled = true;
    if (_syncObjReg != nullptr) {
        _setStateSchedulerDefaultDelay(_syncObjReg->getDefaultDelay());
    }
}

bool SynchronizedObjectBase::_didAlternatingUpdatesSync() const {
    if (ctx().getQAORuntime().getCurrentEvent() != hg::QAO_Event::POST_UPDATE) {
        HG_THROW_TRACED(hg::TracedLogicError,
                        0,
                        "This method may only be called during the POST_UPDATE event.");
    }
    return _syncObjReg->getAlternatingUpdatesFlag();
}

void SynchronizedObjectBase::_eventPreUpdate() {
    if (isMasterObject()) {
        _eventPreUpdate(IfMaster{});
    } else {
        _eventPreUpdate(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventBeginUpdate() {
    if (isMasterObject()) {
        _eventBeginUpdate(IfMaster{});
    } else {
        _eventBeginUpdate(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventUpdate1() {
    if (isMasterObject()) {
        _eventUpdate1(IfMaster{});
    } else {
        _advanceDummyAndScheduleNewStates();

        if (_deathCounter > 0) {
            _deathCounter -= 1;
        } else if (_deathCounter == 0) {
            QAO_Destroy(this);
            return;
        }

        _eventUpdate1(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventUpdate2() {
    if (isMasterObject()) {
        _eventUpdate2(IfMaster{});
    } else {
        _eventUpdate2(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventEndUpdate() {
    if (isMasterObject()) {
        _eventEndUpdate(IfMaster{});
    } else {
        _eventEndUpdate(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventPostUpdate() {
    if (isMasterObject()) {
        _eventPostUpdate(IfMaster{});
    } else {
        _eventPostUpdate(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventPreDraw() {
    if (isMasterObject()) {
        _eventPreDraw(IfMaster{});
    } else {
        _eventPreDraw(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventDraw1() {
    if (isMasterObject()) {
        _eventDraw1(IfMaster{});
    } else {
        _eventDraw1(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventDraw2() {
    if (isMasterObject()) {
        _eventDraw2(IfMaster{});
    } else {
        _eventDraw2(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventDrawGUI() {
    if (isMasterObject()) {
        _eventDrawGUI(IfMaster{});
    } else {
        _eventDrawGUI(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventPostDraw() {
    if (isMasterObject()) {
        _eventPostDraw(IfMaster{});
    } else {
        _eventPostDraw(IfDummy{});
    }
}

void SynchronizedObjectBase::_eventDisplay() {
    if (isMasterObject()) {
        _eventDisplay(IfMaster{});
    } else {
        _eventDisplay(IfDummy{});
    }
}

void SynchronizedObjectBase::__spempeimpl_syncCreateImpl(SyncControlDelegate& aSyncCtrl) const {
    _syncCreateImpl(aSyncCtrl);
}

void SynchronizedObjectBase::__spempeimpl_syncUpdateImpl(SyncControlDelegate& aSyncCtrl) const {
    _syncUpdateImpl(aSyncCtrl);
}

void SynchronizedObjectBase::__spempeimpl_syncDestroyImpl(SyncControlDelegate& aSyncCtrl) const {
    _syncDestroyImpl(aSyncCtrl);
}

//! Deactivated, Skipped, Autodiff-Skipped
#define PER_CLIENT_FLAG_COUNT 3

void SynchronizedObjectBase::__spempeimpl_setDeactivationFlagForClient(hg::PZInteger aClientIdx,
                                                                       bool          aFlag) const {
    const auto bitIdx = (aClientIdx * PER_CLIENT_FLAG_COUNT) + 0;
    if (aFlag) {
        _remoteSyncStatuses.setBit(bitIdx);
    } else {
        _remoteSyncStatuses.clearBit(bitIdx);
    }
}

bool SynchronizedObjectBase::__spempeimpl_getDeactivationFlagForClient(hg::PZInteger aClientIdx) const {
    const auto bitIdx = (aClientIdx * PER_CLIENT_FLAG_COUNT) + 0;
    return _remoteSyncStatuses.getBit(bitIdx);
}

void SynchronizedObjectBase::__spempeimpl_setSkipFlagForClient(hg::PZInteger aClientIdx,
                                                               bool          aFlag) const {
    const auto bitIdx = (aClientIdx * PER_CLIENT_FLAG_COUNT) + 1;
    if (aFlag) {
        _remoteSyncStatuses.setBit(bitIdx);
    } else {
        _remoteSyncStatuses.clearBit(bitIdx);
    }
}

bool SynchronizedObjectBase::__spempeimpl_getSkipFlagForClient(hg::PZInteger aClientIdx) const {
    const auto bitIdx = (aClientIdx * PER_CLIENT_FLAG_COUNT) + 1;
    return _remoteSyncStatuses.getBit(bitIdx);
}

void SynchronizedObjectBase::__spempeimpl_setNoDiffSkipFlagForClient(hg::PZInteger aClientIdx,
                                                                     bool          aFlag) const {
    const auto bitIdx = (aClientIdx * PER_CLIENT_FLAG_COUNT) + 2;
    if (aFlag) {
        _remoteSyncStatuses.setBit(bitIdx);
    } else {
        _remoteSyncStatuses.clearBit(bitIdx);
    }
}

bool SynchronizedObjectBase::__spempeimpl_getNoDiffSkipFlagForClient(hg::PZInteger aClientIdx) const {
    const auto bitIdx = (aClientIdx * PER_CLIENT_FLAG_COUNT) + 2;
    return _remoteSyncStatuses.getBit(bitIdx);
}

void SynchronizedObjectBase::__spempeimpl_setStateSchedulerDefaultDelay(
    hg::PZInteger aNewDefaultDelaySteps) {
    _setStateSchedulerDefaultDelay(aNewDefaultDelaySteps);
}

} // namespace spempe
} // namespace jbatnozic

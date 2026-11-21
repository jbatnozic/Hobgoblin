// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/InteriorWorld/Internal_runtime.hpp>
#include <CBR/QAOMessages/Sync_create_and_update_to_client.hpp>
#include <Engine.hpp>

namespace cinnabar {

class InteriorWorld;

// MARK: IW NonstateObject

//! Base class intended for nonstate objects internal to an `InteriorWorld`.
//! \see `spempe::NonstateObject`.
class InteriorWorldNonstateObject : public spe::NonstateObject {
public:
    //! Main constructor.
    InteriorWorldNonstateObject(QAO_InstGuard aInstGuard,
                                QAO_ExeCon    aExeconThreshold,
                                int           aExecutionPriority,
                                QAO_NameRef   aName)
        : spe::NonstateObject{aInstGuard, aExeconThreshold, aExecutionPriority, aName} {}

    InteriorWorld& getInteriorWorld() const {
        HG_ASSERT(_interiorWorld != nullptr);
        return *_interiorWorld;
    }

private:
    InteriorWorld* _interiorWorld = nullptr;

    void _didAttach(QAO_Runtime& aRuntime) {
        spe::NonstateObject::_didAttach(aRuntime);
        _interiorWorld = &(static_cast<InternalRuntime&>(aRuntime).getInteriorWorld());
    }

    void _willDetach(QAO_Runtime& aRuntime) {
        _interiorWorld = nullptr;
        spe::NonstateObject::_willDetach(aRuntime);
    }
};

QAO_REGISTER_CLASS(InteriorWorldNonstateObject, cinnabar_InteriorWorldNonstateObject) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

// MARK: IW SynchronizedObject

#define RSDATA_QUARTET                                                               \
    taMasterSpecificInlineData, taDummySpecificInlineData, taMasterSpecificHeapData, \
        taDummySpecificHeapData

//! Base class intended for synchronized objects internal to an `InteriorWorld`.
//! \see `spempe::SynchronizedObject`.
template <class taVisibleState,
          class taMasterSpecificInlineData = void,
          class taDummySpecificInlineData  = void,
          class taMasterSpecificHeapData   = void,
          class taDummySpecificHeapData    = void>
class InteriorWorldSynchronizedObject : public spe::SynchronizedObject<taVisibleState, RSDATA_QUARTET> {
public:
    //! Main constructor.
    InteriorWorldSynchronizedObject(hg::QAO_InstGuard aInstGuard,
                                    hg::QAO_ExeCon    aExeconThreshold,
                                    int               aExecutionPriority,
                                    hg::QAO_NameRef   aName,
                                    spe::SyncId       aSyncId = spe::SYNC_ID_NEW)
        : spe::SynchronizedObject<taVisibleState, RSDATA_QUARTET>{aInstGuard,
                                                                  aExeconThreshold,
                                                                  aExecutionPriority,
                                                                  aName,
                                                                  aSyncId} {}

    InteriorWorld& getInteriorWorld() const {
        HG_ASSERT(_interiorWorld != nullptr);
        return *_interiorWorld;
    }

    void msgSyncCreateAndUpdateToClient(SyncCreateAndUpdateToClient::PayloadPtr aPayload, bool aConst) {
        HG_ASSERT(aPayload != nullptr && aConst != true);
        spe::SynchronizedObjectBase::_doSyncCreate(aPayload->clientIndex, aPayload->syncFlags, true);
        spe::SynchronizedObjectBase::_doSyncUpdate(aPayload->clientIndex, aPayload->syncFlags, true);
    }

protected:
    using IWSyncObjSuper = InteriorWorldSynchronizedObject;

    void _didAttach(QAO_Runtime& aRuntime) override {
        spe::SynchronizedObject<taVisibleState, RSDATA_QUARTET>::_didAttach(aRuntime);
        _interiorWorld = &(static_cast<InternalRuntime&>(aRuntime).getInteriorWorld());
    }

    void _willDetach(QAO_Runtime& aRuntime) override {
        _interiorWorld = nullptr;
        spe::SynchronizedObject<taVisibleState, RSDATA_QUARTET>::_willDetach(aRuntime);
    }

private:
    friend class InteriorWorld;

    InteriorWorld* _interiorWorld = nullptr;
};

#undef RSDATA_QUARTET

} // namespace cinnabar

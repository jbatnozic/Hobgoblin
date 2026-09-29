// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>
#include <type_traits>
#include <vector>

using namespace hg::qao;

class QAO_TestWithRuntime : public ::testing::Test {
protected:
    QAO_TestWithRuntime()
        : _runtime{} {}

    void performStep() {
        _runtime.startStep();
        bool done = false;
        _runtime.advanceStep(done);
        ASSERT_TRUE(done);
    }

    std::vector<int> _numbers;
    QAO_Runtime      _runtime;

    template <class T>
    static T* _getObjectPtr(const QAO_Handle<T>& aHandle) {
        return aHandle.operator->();
    }
};

namespace {
class SimpleActiveObject : public QAO_Base {
public:
    SimpleActiveObject(QAO_InstGuard aInstGuard, std::vector<int>& vec, int number)
        : QAO_Base{aInstGuard, QAO_ExeCon::ESSENTIAL, 0, "SimpleActiveObject"}
        , _myVec{vec}
        , _myNumber{number} {}

    using QAO_Base::setExecutionPriority;

    void _eventUpdate1() override {
        _myVec.push_back(_myNumber);
    }

private:
    std::vector<int>& _myVec;
    int               _myNumber;
};
} // namespace

static_assert(std::is_nothrow_move_constructible_v<QAO_GenericHandle>);
static_assert(std::is_move_assignable_v<QAO_GenericHandle>);
static_assert(std::is_nothrow_move_constructible_v<QAO_Handle<SimpleActiveObject>>);
static_assert(std::is_move_assignable_v<QAO_Handle<SimpleActiveObject>>);

// MARK: Create/Destroy function tests

TEST_F(QAO_TestWithRuntime, QAO_CreateReturnsOwningHandleWhenNoRuntimeIsGiven) {
    auto handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0);
    EXPECT_TRUE(handle.isOwning());
}

TEST_F(QAO_TestWithRuntime, QAO_CreateReturnsOwningHandleWhenNonOwningRuntimeRefIsGiven) {
    auto handle = QAO_Create<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 0);
    EXPECT_TRUE(handle.isOwning());
}

TEST_F(QAO_TestWithRuntime, QAO_CreateReturnsNonOwningHandleWhenOwningRuntimeRefIsGiven) {
    auto handle = QAO_Create<SimpleActiveObject>(_runtime, _numbers, 0);
    EXPECT_FALSE(handle.isOwning());

    // Non owning handles can be copied

    QAO_Handle<SimpleActiveObject> h1{handle};
    EXPECT_NE(_getObjectPtr(handle), nullptr);
    EXPECT_EQ(_getObjectPtr(handle), _getObjectPtr(h1));
    EXPECT_FALSE(handle.isOwning());
    EXPECT_FALSE(h1.isOwning());

    QAO_Handle<SimpleActiveObject> h2;
    h2 = handle;
    EXPECT_NE(_getObjectPtr(handle), nullptr);
    EXPECT_EQ(_getObjectPtr(handle), _getObjectPtr(h2));
    EXPECT_FALSE(handle.isOwning());
    EXPECT_FALSE(h2.isOwning());

    // Non owning handles can also be moved

    QAO_Handle<SimpleActiveObject> h3{std::move(handle)};
    EXPECT_EQ(_getObjectPtr(handle), nullptr);
    EXPECT_EQ(_getObjectPtr(h3), _getObjectPtr(h3));
    EXPECT_FALSE(h3.isOwning());

    QAO_Handle<SimpleActiveObject> h4;
    h4 = std::move(h3);
    EXPECT_EQ(_getObjectPtr(h3), nullptr);
    EXPECT_NE(_getObjectPtr(h4), nullptr);
    EXPECT_FALSE(h4.isOwning());
}

TEST_F(QAO_TestWithRuntime, CreateObjectOwnedByHandleThenAttachToRuntimeLater) {
    auto handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0);
    ASSERT_TRUE(handle.isOwning());
    EXPECT_EQ(_runtime.getObjectCount(), 0);
    EXPECT_EQ(handle->getRuntime(), nullptr);

    _runtime.attachObject(handle);
    EXPECT_EQ(_runtime.getObjectCount(), 1);
    EXPECT_EQ(handle->getRuntime(), &_runtime);

    QAO_Destroy(std::move(handle));
    EXPECT_EQ(_runtime.getObjectCount(), 0);
}

#if 0
TEST_F(QAO_TestWithRuntime, ICreate) {
    auto id = QAO_ICreate<SimpleActiveObject>(&_runtime, _numbers, 0);
    ASSERT_EQ(_runtime.getObjectCount(), 1);

    auto* const obj = _runtime.find(id);
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->getRuntime(), &_runtime);

    QAO_IDestroy(id, _runtime);
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, ICreateFailsBecauseOfNull) {
    EXPECT_THROW(QAO_ICreate<SimpleActiveObject>(nullptr, _numbers, 0), hg::TracedLogicError);
}

TEST_F(QAO_TestWithRuntime, ICreateFailsBecauseOfNonOwningRef) {
    EXPECT_THROW(QAO_ICreate<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 0),
                 hg::TracedLogicError);
}
#endif

TEST_F(QAO_TestWithRuntime, DetachingObjectsFailsWhenObjectIsNotAttachedToRuntime) {
    auto handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0);

    EXPECT_THROW(_runtime.detachObject(*handle), hg::PreconditionNotMetError);
}

TEST_F(QAO_TestWithRuntime, DetachingObjectsReturnsOwningHandleWhenObjectIsOwnedByRuntime) {
    auto  handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0);
    auto* objRaw = handle.ptr();
    _runtime.attachObject(std::move(handle));
    ASSERT_EQ(_runtime.getObjectCount(), 1);

    auto detachedHandle = hg::MoveToUnderlying(_runtime.detachObject(*objRaw));
    ASSERT_TRUE(detachedHandle.isOwning());
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, DetachingObjectsReturnsNonOwningHandleWhenObjectIsNotOwnedByRuntime) {
    auto handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0);
    _runtime.attachObject(handle);
    ASSERT_EQ(_runtime.getObjectCount(), 1);

    auto detachedHandle = hg::MoveToUnderlying(_runtime.detachObject(*handle));
    ASSERT_FALSE(detachedHandle.isOwning());
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, ObjectCount_WithQAO_Destroy_NonOwningHandle) {
    auto handle = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, 0);
    EXPECT_FALSE(handle.isOwning());
    ASSERT_EQ(_runtime.getObjectCount(), 1);
    QAO_Destroy(std::move(handle));
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, ObjectCount_WithQAO_Destroy_OwningHandle) {
    auto handle = QAO_Create<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 0);
    EXPECT_TRUE(handle.isOwning());
    ASSERT_EQ(_runtime.getObjectCount(), 1);
    QAO_Destroy(std::move(handle));
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, ObjectCount_WithQAO_Destroy_Pointer) {
    auto* object = QAO_Create<SimpleActiveObject>(_runtime, _numbers, 0).ptr();
    EXPECT_NE(object, nullptr);
    ASSERT_EQ(_runtime.getObjectCount(), 1);
    QAO_Destroy(object);
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, ObjectCount_WithHandleGoingOutOfScope) {
    {
        auto handle = QAO_Create<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 0);
        EXPECT_TRUE(handle.isOwning());
        ASSERT_EQ(_runtime.getObjectCount(), 1);
    }
    ASSERT_EQ(_runtime.getObjectCount(), 0);
}

// MARK: Execution and ordering tests

TEST_F(QAO_TestWithRuntime, SimpleEvent) {
    constexpr int VALUE_0 = 1;
    auto          obj     = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_0);
    performStep();
    ASSERT_EQ(_numbers.size(), 1u);
    ASSERT_EQ(_numbers[0], VALUE_0);
}

TEST_F(QAO_TestWithRuntime, Ordering) {
    constexpr int VALUE_0 = 1;
    constexpr int VALUE_1 = 2;
    constexpr int VALUE_2 = 3;
    auto          obj0    = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_0);
    auto          obj1    = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_1);
    auto          obj2    = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_2);

    obj0->setExecutionPriority(80);
    obj1->setExecutionPriority(70);
    obj2->setExecutionPriority(60);

    performStep();
    ASSERT_EQ(_numbers.size(), 3u);
    ASSERT_EQ(_numbers[0], VALUE_0);
    ASSERT_EQ(_numbers[1], VALUE_1);
    ASSERT_EQ(_numbers[2], VALUE_2);

    _numbers.clear();

    // Reverse order
    obj0->setExecutionPriority(60);
    obj1->setExecutionPriority(70);
    obj2->setExecutionPriority(80);

    performStep();
    ASSERT_EQ(_numbers.size(), 3u);
    ASSERT_EQ(_numbers[0], VALUE_2);
    ASSERT_EQ(_numbers[1], VALUE_1);
    ASSERT_EQ(_numbers[2], VALUE_0);
}

TEST_F(QAO_TestWithRuntime, Execon) {
    auto execon = QAO_ExeCon::META_EXECUTE_ALL;
    _runtime.setExeconAddress(&execon);

    constexpr int VALUE_0 = 1;
    constexpr int VALUE_1 = 2;
    constexpr int VALUE_2 = 3;

    auto obj0 = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_0);
    obj0->setExecutionPriority(80);
    obj0->setExeconThreshold(QAO_ExeCon::ESSENTIAL);
    auto obj1 = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_1);
    obj1->setExecutionPriority(70);
    obj1->setExeconThreshold(QAO_ExeCon::SYNCHRONIZATION);
    auto obj2 = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, VALUE_2);
    obj2->setExecutionPriority(60);
    obj2->setExeconThreshold(QAO_ExeCon::GAMEPLAY);

    performStep();
    ASSERT_EQ(_numbers.size(), 3u);
    ASSERT_EQ(_numbers[0], VALUE_0);
    ASSERT_EQ(_numbers[1], VALUE_1);
    ASSERT_EQ(_numbers[2], VALUE_2);

    _numbers.clear();

    // Lower execon
    execon = QAO_ExeCon::SYNCHRONIZATION;

    performStep();
    ASSERT_EQ(_numbers.size(), 2u);
    ASSERT_EQ(_numbers[0], VALUE_0);
    ASSERT_EQ(_numbers[1], VALUE_1);

    _numbers.clear();

    // Lower execon again
    execon = QAO_ExeCon::INTERACTIVITY;

    _numbers.clear();

    performStep();
    ASSERT_EQ(_numbers.size(), 1u);
    ASSERT_EQ(_numbers[0], VALUE_0);
}

namespace {
class SimpleActiveObjectWhichDeletesItself : public QAO_Base {
public:
    SimpleActiveObjectWhichDeletesItself(QAO_InstGuard aInstGuard)
        : QAO_Base{aInstGuard, QAO_ExeCon::ESSENTIAL, 0, "SimpleActiveObjectWhichDeletesItself"} {}

    void _eventUpdate1() override {
        if (getRuntime()->ownsObject(*this)) {
            auto handleToSelf = hg::MoveToUnderlying(getRuntime()->detachObject(*this));
            ASSERT_TRUE(handleToSelf.isOwning());
            handleToSelf.reset();
        }
    }
};
} // namespace

TEST_F(QAO_TestWithRuntime, ObjectsDeleteThemselves) {
    QAO_Create<SimpleActiveObjectWhichDeletesItself>(&_runtime);
    QAO_Create<SimpleActiveObjectWhichDeletesItself>(&_runtime);
    QAO_Create<SimpleActiveObjectWhichDeletesItself>(&_runtime);
    auto controlObject = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, 0);
    controlObject->setExecutionPriority(-1000);

    ASSERT_EQ(_runtime.getObjectCount(), 4);

    performStep();

    ASSERT_EQ(_runtime.getObjectCount(), 1);

    performStep();
}

// MARK: GenericId tests

TEST_F(QAO_TestWithRuntime, NullIdEquality) {
    QAO_GenericId id1{};
    QAO_GenericId id2{nullptr};
    ASSERT_EQ(id1, id2);
}

TEST_F(QAO_TestWithRuntime, NullIdFindsNullptr) {
    QAO_GenericId nullId{};
    ASSERT_EQ(_runtime.find(nullId), nullptr);
}

// MARK: Room tests

namespace {
class DestructionTrackingObject : public QAO_Base {
public:
    DestructionTrackingObject(QAO_InstGuard aInstGuard, int& aDestructionCounter)
        : QAO_Base{aInstGuard, QAO_ExeCon::ESSENTIAL, 0, "DestructionTrackingObject"}
        , _destructionCounter{aDestructionCounter} {}

    ~DestructionTrackingObject() override {
        _destructionCounter += 1;
    }

private:
    int& _destructionCounter;
};
} // namespace

TEST_F(QAO_TestWithRuntime, RuntimeStartsWithDefaultRoom) {
    ASSERT_EQ(_runtime.getRoomCount(), 1);

    const auto* topRoom = _runtime.getTopRoom();
    ASSERT_NE(topRoom, nullptr);
    EXPECT_EQ(topRoom->name, "default");
    EXPECT_EQ(topRoom->id, 1);
    EXPECT_NE(topRoom->id, QAO_INVALID_ROOM_ID);
}

TEST_F(QAO_TestWithRuntime, PushRoomBecomesTopRoomWithIncrementedId) {
    auto& room = _runtime.pushRoom("room_a");
    EXPECT_EQ(room.name, "room_a");
    EXPECT_EQ(room.id, 2);
    EXPECT_EQ(_runtime.getRoomCount(), 2);
    EXPECT_EQ(_runtime.getTopRoom(), &room);

    _runtime.pushRoom("room_b");
    EXPECT_EQ(_runtime.getRoomCount(), 3);
    ASSERT_NE(_runtime.getTopRoom(), nullptr);
    EXPECT_EQ(_runtime.getTopRoom()->name, "room_b");
    EXPECT_EQ(_runtime.getTopRoom()->id, 3);
}

TEST_F(QAO_TestWithRuntime, PopRoomRestoresPreviousTopRoom) {
    _runtime.pushRoom("room_a");
    _runtime.pushRoom("room_b");
    ASSERT_EQ(_runtime.getRoomCount(), 3);

    _runtime.popRoom();
    EXPECT_EQ(_runtime.getRoomCount(), 2);
    ASSERT_NE(_runtime.getTopRoom(), nullptr);
    EXPECT_EQ(_runtime.getTopRoom()->name, "room_a");
    EXPECT_EQ(_runtime.getTopRoom()->id, 2);

    _runtime.popRoom();
    EXPECT_EQ(_runtime.getRoomCount(), 1);
    ASSERT_NE(_runtime.getTopRoom(), nullptr);
    EXPECT_EQ(_runtime.getTopRoom()->name, "default");
    EXPECT_EQ(_runtime.getTopRoom()->id, 1);

    _runtime.popRoom();
    EXPECT_EQ(_runtime.getRoomCount(), 0);
    EXPECT_EQ(_runtime.getTopRoom(), nullptr);
}

TEST_F(QAO_TestWithRuntime, PopRoomFailsWhenNoRoomsAreLeft) {
    _runtime.popRoom();
    ASSERT_EQ(_runtime.getRoomCount(), 0);
    EXPECT_THROW(_runtime.popRoom(), hg::PreconditionNotMetError);
}

TEST_F(QAO_TestWithRuntime, PushRoomFailsWhenMaxRoomCountWouldBeExceeded) {
    while (_runtime.getRoomCount() < QAO_MAX_ROOM_COUNT) {
        _runtime.pushRoom("room");
    }
    ASSERT_EQ(_runtime.getRoomCount(), QAO_MAX_ROOM_COUNT);
    ASSERT_NE(_runtime.getTopRoom(), nullptr);
    EXPECT_EQ(_runtime.getTopRoom()->id, static_cast<QAO_RoomId>(QAO_MAX_ROOM_COUNT));

    EXPECT_THROW(_runtime.pushRoom("one_too_many"), hg::PreconditionNotMetError);
    EXPECT_EQ(_runtime.getRoomCount(), QAO_MAX_ROOM_COUNT);
}

TEST_F(QAO_TestWithRuntime, AttachedObjectsGetRoomIdOfTopRoom) {
    auto obj0 = QAO_Create<SimpleActiveObject>(_runtime, _numbers, 0);
    EXPECT_EQ(obj0->getRoomId(), _runtime.getTopRoom()->id);

    const auto roomAId = _runtime.pushRoom("room_a").id;
    auto       obj1    = QAO_Create<SimpleActiveObject>(_runtime, _numbers, 1);
    EXPECT_EQ(obj1->getRoomId(), roomAId);
    EXPECT_NE(obj0->getRoomId(), obj1->getRoomId());
}

TEST_F(QAO_TestWithRuntime, ObjectsNotOwnedByRuntimeHaveInvalidRoomId) {
    auto obj0 = QAO_Create<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 0);
    ASSERT_TRUE(obj0.isOwning());
    EXPECT_EQ(obj0->getRoomId(), QAO_INVALID_ROOM_ID);

    _runtime.pushRoom("room_a");
    auto obj1 = QAO_Create<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 1);
    ASSERT_TRUE(obj1.isOwning());
    EXPECT_EQ(obj1->getRoomId(), QAO_INVALID_ROOM_ID);

    // Attaching via a copy of the handle (so ownership stays with the handle)
    auto handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 2);
    _runtime.attachObject(handle);
    ASSERT_FALSE(_runtime.ownsObject(*handle));
    EXPECT_EQ(handle->getRoomId(), QAO_INVALID_ROOM_ID);
}

TEST_F(QAO_TestWithRuntime, UnattachedAndDetachedObjectsHaveInvalidRoomId) {
    auto  handle = QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0);
    auto* objRaw = handle.ptr();
    EXPECT_EQ(objRaw->getRoomId(), QAO_INVALID_ROOM_ID);

    _runtime.attachObject(std::move(handle));
    ASSERT_TRUE(_runtime.ownsObject(*objRaw));
    EXPECT_NE(objRaw->getRoomId(), QAO_INVALID_ROOM_ID);

    auto detachedHandle = hg::MoveToUnderlying(_runtime.detachObject(*objRaw));
    ASSERT_TRUE(detachedHandle.isOwning());
    EXPECT_EQ(detachedHandle->getRoomId(), QAO_INVALID_ROOM_ID);
}

TEST_F(QAO_TestWithRuntime, PopRoomDestroysOwnedObjectsOfTopRoomOnly) {
    int destroyedInDefault = 0;
    int destroyedInRoomA   = 0;

    QAO_Create<DestructionTrackingObject>(_runtime, destroyedInDefault);
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedInDefault);

    _runtime.pushRoom("room_a");
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedInRoomA);
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedInRoomA);
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedInRoomA);
    ASSERT_EQ(_runtime.getObjectCount(), 5);

    _runtime.popRoom();
    EXPECT_EQ(destroyedInRoomA, 3);
    EXPECT_EQ(destroyedInDefault, 0);
    EXPECT_EQ(_runtime.getObjectCount(), 2);

    _runtime.popRoom();
    EXPECT_EQ(destroyedInRoomA, 3);
    EXPECT_EQ(destroyedInDefault, 2);
    EXPECT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, PopRoomDoesNotDestroyObjectsNotOwnedByRuntime) {
    int destroyedCount = 0;

    _runtime.pushRoom("room_a");
    auto handle = QAO_Create<DestructionTrackingObject>(_runtime.nonOwning(), destroyedCount);
    ASSERT_TRUE(handle.isOwning());
    EXPECT_EQ(handle->getRoomId(), QAO_INVALID_ROOM_ID);
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedCount);
    ASSERT_EQ(_runtime.getObjectCount(), 2);

    _runtime.popRoom();
    EXPECT_EQ(destroyedCount, 1);
    EXPECT_EQ(_runtime.getObjectCount(), 1);
    EXPECT_EQ(handle->getRuntime(), &_runtime);

    handle.reset();
    EXPECT_EQ(destroyedCount, 2);
    EXPECT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, PopRoomDestroysObjectsDetachedAndReattachedInTopRoom) {
    int destroyedCount = 0;

    auto  handle = QAO_Create<DestructionTrackingObject>(nullptr, destroyedCount);
    auto* objRaw = handle.ptr();
    _runtime.attachObject(std::move(handle));
    ASSERT_EQ(objRaw->getRoomId(), _runtime.getTopRoom()->id);

    const auto roomAId        = _runtime.pushRoom("room_a").id;
    auto       detachedHandle = hg::MoveToUnderlying(_runtime.detachObject(*objRaw));
    ASSERT_TRUE(detachedHandle.isOwning());
    _runtime.attachObject(std::move(detachedHandle));
    EXPECT_EQ(objRaw->getRoomId(), roomAId);

    _runtime.popRoom();
    EXPECT_EQ(destroyedCount, 1);
    EXPECT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, AttachingOwnedObjectFailsWhenNoRoomsAreLeft) {
    _runtime.popRoom();
    ASSERT_EQ(_runtime.getRoomCount(), 0);

    EXPECT_THROW(_runtime.attachObject(QAO_Create<SimpleActiveObject>(nullptr, _numbers, 0)),
                 hg::PreconditionNotMetError);
    EXPECT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, AttachingNonOwnedObjectSucceedsWhenNoRoomsAreLeft) {
    _runtime.popRoom();
    ASSERT_EQ(_runtime.getRoomCount(), 0);

    auto handle = QAO_Create<SimpleActiveObject>(_runtime.nonOwning(), _numbers, 0);
    ASSERT_TRUE(handle.isOwning());
    EXPECT_EQ(_runtime.getObjectCount(), 1);
    EXPECT_EQ(handle->getRuntime(), &_runtime);
    EXPECT_EQ(handle->getRoomId(), QAO_INVALID_ROOM_ID);
}

TEST_F(QAO_TestWithRuntime, CanAttachOwnedObjectsAgainAfterPushingRoomOnEmptyStack) {
    _runtime.popRoom();
    ASSERT_EQ(_runtime.getRoomCount(), 0);

    const auto roomId = _runtime.pushRoom("new_room").id;
    EXPECT_EQ(roomId, 1);

    int  destroyedCount = 0;
    auto obj            = QAO_Create<DestructionTrackingObject>(_runtime, destroyedCount);
    EXPECT_EQ(obj->getRoomId(), roomId);
    EXPECT_EQ(_runtime.getObjectCount(), 1);

    _runtime.popRoom();
    EXPECT_EQ(destroyedCount, 1);
    EXPECT_EQ(_runtime.getObjectCount(), 0);
}

TEST_F(QAO_TestWithRuntime, PopAllRoomsDestroysObjectsInAllRooms) {
    int destroyedCount = 0;

    QAO_Create<DestructionTrackingObject>(_runtime, destroyedCount);
    _runtime.pushRoom("room_a");
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedCount);
    _runtime.pushRoom("room_b");
    QAO_Create<DestructionTrackingObject>(_runtime, destroyedCount);
    auto nonOwned = QAO_Create<DestructionTrackingObject>(_runtime.nonOwning(), destroyedCount);
    ASSERT_EQ(_runtime.getObjectCount(), 4);

    _runtime.popAllRooms();
    EXPECT_EQ(destroyedCount, 3);
    EXPECT_EQ(_runtime.getObjectCount(), 1);
    EXPECT_EQ(nonOwned->getRuntime(), &_runtime);
}

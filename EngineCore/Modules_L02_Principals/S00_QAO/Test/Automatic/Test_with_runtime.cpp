// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>
#include <type_traits>
#include <vector>

using namespace hg::qao;

#define TYPEID_SELF typeid(decltype(*this))

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

class SimpleActiveObject : public QAO_Base {
public:
    SimpleActiveObject(QAO_IKey aIKey, std::vector<int>& vec, int number)
        : QAO_Base{aIKey, TYPEID_SELF, 0, "SimpleActiveObject"}
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

TEST_F(QAO_TestWithRuntime, ObjectCount) {
    auto handle = QAO_Create<SimpleActiveObject>(&_runtime, _numbers, 0);
    ASSERT_EQ(_runtime.getObjectCount(), 1);
    QAO_Destroy(handle);
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

namespace {
class SimpleActiveObjectWhichDeletesItself : public QAO_Base {
public:
    SimpleActiveObjectWhichDeletesItself(QAO_IKey aIKey)
        : QAO_Base{aIKey, TYPEID_SELF, 0, "SimpleActiveObjectWhichDeletesItself"} {}

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

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO.hpp>
#include <Hobgoblin/QAO/Registry.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

using namespace hg::qao;
using qao_detail::QAO_Registry;

namespace {
class DummyObject : public QAO_Base {
public:
    DummyObject(QAO_InstGuard aInstGuard)
        : QAO_Base{aInstGuard, QAO_ExeCon::ESSENTIAL, 0, "DummyObject"} {}
};

// The (serial, index) constructor of QAO_GenericId is protected; this lets the tests build IDs.
struct TestId : QAO_GenericId {
    TestId(QAO_Serial aSerial, QAO_Index aIndex)
        : QAO_GenericId{aSerial, aIndex} {}
};

QAO_GenericHandle MakeDummy() {
    return QAO_Create<DummyObject>(nullptr);
}
} // namespace

class QAO_RegistryTest : public ::testing::Test {
protected:
    QAO_Registry _registry{1};
};

// MARK: insert

TEST_F(QAO_RegistryTest, InsertNullHandleThrowsWithoutModifyingRegistry) {
    QAO_GenericHandle null;
    EXPECT_THROW(_registry.insert(std::move(null)), hg::InvalidArgumentError);
    EXPECT_EQ(_registry.instanceCount(), 0);
}

TEST_F(QAO_RegistryTest, InsertSerialClashThrowsWithoutModifyingRegistryOrHandle) {
    // Occupy the serial that the next insert() would hand out, at some other index.
    auto first = MakeDummy();
    _registry.insertWithId(std::move(first), TestId{QAO_STARTING_SERIAL, 5});
    ASSERT_EQ(_registry.instanceCount(), 1);

    auto       handle = MakeDummy();
    const auto ptr    = handle.ptr();
    EXPECT_THROW(_registry.insert(std::move(handle)), hg::TracedRuntimeError);

    // Handle not moved from, object not destroyed
    EXPECT_FALSE(handle.isNull());
    EXPECT_TRUE(handle.isOwning());
    EXPECT_EQ(handle.ptr(), ptr);

    // Registry unchanged: no slot leaked, no stray mapping
    EXPECT_EQ(_registry.instanceCount(), 1);
    for (QAO_Index i = 0; i < 5; i += 1) {
        EXPECT_TRUE(_registry.findObjectWithIndex(i).isNull()) << "index " << i;
    }
    EXPECT_FALSE(_registry.findObjectWithSerial(QAO_STARTING_SERIAL).isNull());
    EXPECT_TRUE(_registry.findObjectWithSerial(QAO_STARTING_SERIAL + 1).isNull());
}

TEST_F(QAO_RegistryTest, InsertGrowsAndAssignsSequentialIds) {
    const auto id0 = _registry.insert(MakeDummy());
    const auto id1 = _registry.insert(MakeDummy());
    const auto id2 = _registry.insert(MakeDummy());

    EXPECT_EQ(_registry.instanceCount(), 3);
    EXPECT_EQ(id0.getSerial(), QAO_STARTING_SERIAL);
    EXPECT_EQ(id1.getSerial(), QAO_STARTING_SERIAL + 1);
    EXPECT_EQ(id2.getSerial(), QAO_STARTING_SERIAL + 2);
    EXPECT_NE(id0.getIndex(), id1.getIndex());
    EXPECT_NE(id1.getIndex(), id2.getIndex());

    for (const auto& id : {id0, id1, id2}) {
        EXPECT_FALSE(_registry.findObjectWithId(id).isNull());
        EXPECT_FALSE(_registry.findObjectWithIndex(id.getIndex()).isNull());
        EXPECT_EQ(_registry.findObjectWithSerial(id.getSerial()).ptr(),
                  _registry.findObjectWithIndex(id.getIndex()).ptr());
    }
}

// MARK: insertWithId

TEST_F(QAO_RegistryTest, InsertWithIdNullHandleThrowsWithoutModifyingRegistry) {
    QAO_GenericHandle null;
    EXPECT_THROW(_registry.insertWithId(std::move(null), TestId{10, 3}), hg::InvalidArgumentError);
    EXPECT_EQ(_registry.instanceCount(), 0);
    EXPECT_TRUE(_registry.findObjectWithIndex(3).isNull());
    EXPECT_TRUE(_registry.findObjectWithSerial(10).isNull());
}

TEST_F(QAO_RegistryTest, InsertWithIdIndexClashThrowsWithoutModifyingRegistryOrHandle) {
    _registry.insertWithId(MakeDummy(), TestId{10, 3});
    ASSERT_EQ(_registry.instanceCount(), 1);
    const auto original = _registry.findObjectWithIndex(3).ptr();

    auto       handle = MakeDummy();
    const auto ptr    = handle.ptr();
    EXPECT_THROW(_registry.insertWithId(std::move(handle), TestId{11, 3}), hg::TracedLogicError);

    EXPECT_FALSE(handle.isNull());
    EXPECT_TRUE(handle.isOwning());
    EXPECT_EQ(handle.ptr(), ptr);

    EXPECT_EQ(_registry.instanceCount(), 1);
    EXPECT_EQ(_registry.findObjectWithIndex(3).ptr(), original);
    EXPECT_TRUE(_registry.findObjectWithSerial(11).isNull());
}

TEST_F(QAO_RegistryTest, InsertWithIdSerialClashThrowsWithoutModifyingRegistryOrHandle) {
    _registry.insertWithId(MakeDummy(), TestId{10, 3});
    ASSERT_EQ(_registry.instanceCount(), 1);

    auto       handle = MakeDummy();
    const auto ptr    = handle.ptr();
    EXPECT_THROW(_registry.insertWithId(std::move(handle), TestId{10, 7}), hg::TracedRuntimeError);

    EXPECT_FALSE(handle.isNull());
    EXPECT_TRUE(handle.isOwning());
    EXPECT_EQ(handle.ptr(), ptr);

    // Index 7 must not have leaked - it must still be usable afterwards
    EXPECT_EQ(_registry.instanceCount(), 1);
    EXPECT_TRUE(_registry.findObjectWithIndex(7).isNull());
    EXPECT_NO_THROW(_registry.insertWithId(std::move(handle), TestId{11, 7}));
    EXPECT_EQ(_registry.instanceCount(), 2);
    EXPECT_EQ(_registry.findObjectWithIndex(7).ptr(), ptr);
}

TEST_F(QAO_RegistryTest, InsertWithIdFarIndexGrowsRegistry) {
    auto       handle = MakeDummy();
    const auto ptr    = handle.ptr();
    _registry.insertWithId(std::move(handle), TestId{42, 100});

    EXPECT_EQ(_registry.instanceCount(), 1);
    EXPECT_EQ(_registry.findObjectWithIndex(100).ptr(), ptr);
    EXPECT_EQ(_registry.findObjectWithSerial(42).ptr(), ptr);
    EXPECT_TRUE(_registry.isObjectWithIndexOwned(100));
}

// MARK: remove

TEST_F(QAO_RegistryTest, RemoveReturnsHandleAndFreesSlot) {
    auto       handle = MakeDummy();
    const auto ptr    = handle.ptr();
    const auto id     = _registry.insert(std::move(handle));

    auto removed = _registry.remove(id.getIndex());
    EXPECT_EQ(removed.ptr(), ptr);
    EXPECT_TRUE(removed.isOwning());

    EXPECT_EQ(_registry.instanceCount(), 0);
    EXPECT_TRUE(_registry.findObjectWithId(id).isNull());
    EXPECT_TRUE(_registry.findObjectWithSerial(id.getSerial()).isNull());

    // Slot is reusable
    const auto id2 = _registry.insert(MakeDummy());
    EXPECT_EQ(id2.getIndex(), id.getIndex());
}

TEST_F(QAO_RegistryTest, RemoveEmptySlotReturnsNullHandle) {
    _registry.insert(MakeDummy());
    _registry.insertWithId(MakeDummy(), TestId{50, 4});
    EXPECT_TRUE(_registry.remove(2).isNull());
    EXPECT_EQ(_registry.instanceCount(), 2);
}

TEST_F(QAO_RegistryTest, RemoveOutOfRangeThrowsWithoutModifyingRegistry) {
    _registry.insert(MakeDummy());
    EXPECT_THROW(_registry.remove(1000), std::out_of_range);
    EXPECT_EQ(_registry.instanceCount(), 1);
}

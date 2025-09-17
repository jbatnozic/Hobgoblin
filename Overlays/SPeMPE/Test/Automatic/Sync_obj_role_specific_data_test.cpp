// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/SPeMPE.hpp>

#include <Hobgoblin/Utility/Autopack.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace jbatnozic {
namespace spempe {

namespace {

// MARK: Avatars

struct Avatar_VisibleState {
    std::int8_t i = '\0';
    HG_ENABLE_AUTOPACK(Avatar_VisibleState, i);
};

struct Avatar_RSData {
    int value = 12345;

    bool* flagPtr = nullptr;

    ~Avatar_RSData() {
        if (flagPtr) {
            *flagPtr = true;
        }
    }
};

class AvatarWithInlineRSData
    : public SynchronizedObject<Avatar_VisibleState,
                                SPEMPE_RSDATA_INLINE(Avatar_RSData, Avatar_RSData)> {
public:
    AvatarWithInlineRSData(hg::QAO_InstGuard aInstGuard, SyncId aSyncId = SYNC_ID_NEW)
        : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, 0, "AvatarWithInlineRSData", aSyncId} {}

    Avatar_RSData* getMasterData() {
        return _masterData;
    }

    Avatar_RSData* getDummyData() {
        return _dummyData;
    }

private:
    void _syncCreateImpl(SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(SyncControlDelegate& aSyncCtrl) const override;
};

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AvatarWithInlineRSData, (CREATE, UPDATE, DESTROY));

void AvatarWithInlineRSData::_syncCreateImpl(SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AvatarWithInlineRSData, aSyncCtrl);
}

void AvatarWithInlineRSData::_syncUpdateImpl(SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AvatarWithInlineRSData, aSyncCtrl);
}

void AvatarWithInlineRSData::_syncDestroyImpl(SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AvatarWithInlineRSData, aSyncCtrl);
}

class AvatarWithHeapRSData
    : public SynchronizedObject<Avatar_VisibleState, SPEMPE_RSDATA_HEAP(Avatar_RSData, Avatar_RSData)> {
public:
    AvatarWithHeapRSData(hg::QAO_InstGuard aInstGuard, SyncId aSyncId = SYNC_ID_NEW)
        : SyncObjSuper{aInstGuard, SPEMPE_TYPEID_SELF, 0, "AvatarWithHeapRSData", aSyncId} {}

    Avatar_RSData* getMasterData() {
        return _masterData.get();
    }

    Avatar_RSData* getDummyData() {
        return _dummyData.get();
    }

private:
    void _syncCreateImpl(SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(SyncControlDelegate& aSyncCtrl) const override;
};

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(AvatarWithHeapRSData, (CREATE, UPDATE, DESTROY));

void AvatarWithHeapRSData::_syncCreateImpl(SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(AvatarWithHeapRSData, aSyncCtrl);
}

void AvatarWithHeapRSData::_syncUpdateImpl(SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(AvatarWithHeapRSData, aSyncCtrl);
}

void AvatarWithHeapRSData::_syncDestroyImpl(SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(AvatarWithHeapRSData, aSyncCtrl);
}

} // namespace

// MARK: Fixture

using MNetworking = NetworkingManagerInterface;

class SPeMPE_SyncObjRoleSpecificDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        hg::RN_IndexHandlers();
    }

    std::optional<GameContext> _serverCtx{GameContext::RuntimeConfig{}};
    std::optional<GameContext> _clientCtx{GameContext::RuntimeConfig{}};

    hg::QAO_GenericHandle _masterObj;
    hg::QAO_GenericHandle _dummyObj;

    bool _createAndConnectContexts() {
        // Create
        {
            using namespace hobgoblin::rn;

            constexpr static int BUFFERING_LENGTH = 0;

            // Server context:
            _serverCtx->setToMode(GameContext::Mode::Server);

            auto netwMgr1 = QAO_Create<DefaultNetworkingManager>(_serverCtx->getQAORuntime().nonOwning(),
                                                                 0,
                                                                 BUFFERING_LENGTH);
            netwMgr1->setToServerMode(RN_Protocol::UDP, "pass", 2, 512, RN_NetworkingStack::Default);
            _serverCtx->attachAndOwnComponent(std::move(netwMgr1));

            // Client context:
            _clientCtx->setToMode(GameContext::Mode::Client);

            auto netwMgr2 = QAO_Create<DefaultNetworkingManager>(_clientCtx->getQAORuntime().nonOwning(),
                                                                 0,
                                                                 BUFFERING_LENGTH);
            netwMgr2->setToClientMode(RN_Protocol::UDP, "pass", 512, RN_NetworkingStack::Default);
            _clientCtx->attachAndOwnComponent(std::move(netwMgr2));
        }
        // Connect
        {
            EXPECT_TRUE(_serverCtx->getComponent<MNetworking>().isServer());
            EXPECT_TRUE(_clientCtx->getComponent<MNetworking>().isClient());
            if (!_serverCtx->getComponent<MNetworking>().isServer() ||
                !_clientCtx->getComponent<MNetworking>().isClient()) {
                return false;
            }

            auto& server = _serverCtx->getComponent<MNetworking>().getServer();
            auto& client = _clientCtx->getComponent<MNetworking>().getClient();

            server.setUserData(static_cast<GameContext*>(&*_serverCtx));
            server.start(0);

            client.setUserData(static_cast<GameContext*>(&*_clientCtx));
            client.connectLocal(server);

            _serverCtx->runFor(1);
            _clientCtx->runFor(1);
            _serverCtx->runFor(1);
            _clientCtx->runFor(1);

            EXPECT_EQ(server.getClientConnector(0).getStatus(), hg::RN_ConnectorStatus::Connected);
            EXPECT_EQ(client.getServerConnector().getStatus(), hg::RN_ConnectorStatus::Connected);
            if (server.getClientConnector(0).getStatus() != hg::RN_ConnectorStatus::Connected ||
                client.getServerConnector().getStatus() != hg::RN_ConnectorStatus::Connected) {
                return false;
            }
        }
        return true;
    }

    template <class taSyncObj>
    bool _createAndSyncAvatar() {
        _masterObj = hg::QAO_Create<taSyncObj>(&_serverCtx->getQAORuntime());

        _serverCtx->runFor(1);
        _clientCtx->runFor(1);

        for (auto handle : _clientCtx->getQAORuntime()) {
            if (handle->getTypeInfo() == typeid(taSyncObj)) {
                _dummyObj = handle;
                break;
            }
        }

        return (_masterObj && _dummyObj);
    }

    bool _destroyAvatarAndSyncDestruction() {
        auto master = _masterObj.downcastCopy<AvatarWithInlineRSData>();
        hg::QAO_Destroy(*master);

        _serverCtx->runFor(1);
        _clientCtx->runFor(1);

        _masterObj = {};
        _dummyObj  = {};

        return (_serverCtx->getQAORuntime().getObjectCount() == 1 &&
                _clientCtx->getQAORuntime().getObjectCount() == 1);
    }
};

// MARK: Test cases

TEST_F(SPeMPE_SyncObjRoleSpecificDataTest, TestWithInlineData) {
    ASSERT_TRUE(_createAndConnectContexts());
    ASSERT_TRUE(_createAndSyncAvatar<AvatarWithInlineRSData>());

    bool masterDataDestroyed = false;
    bool dummyDataDestroyed  = false;

    {
        auto master = _masterObj.downcastCopy<AvatarWithInlineRSData>();
        ASSERT_EQ(master->getDummyData(), nullptr);
        ASSERT_NE(master->getMasterData(), nullptr);
        EXPECT_EQ(master->getMasterData()->value, 12345);
        master->getMasterData()->flagPtr = &masterDataDestroyed;

        auto dummy = _dummyObj.downcastCopy<AvatarWithInlineRSData>();
        ASSERT_EQ(dummy->getMasterData(), nullptr);
        ASSERT_NE(dummy->getDummyData(), nullptr);
        EXPECT_EQ(dummy->getDummyData()->value, 12345);
        dummy->getDummyData()->flagPtr = &dummyDataDestroyed;
    }

    ASSERT_TRUE(_destroyAvatarAndSyncDestruction());

    EXPECT_TRUE(masterDataDestroyed);
    EXPECT_TRUE(dummyDataDestroyed);
}

TEST_F(SPeMPE_SyncObjRoleSpecificDataTest, TestWithHeapData) {
    ASSERT_TRUE(_createAndConnectContexts());
    ASSERT_TRUE(_createAndSyncAvatar<AvatarWithHeapRSData>());

    bool masterDataDestroyed = false;
    bool dummyDataDestroyed  = false;

    {
        auto master = _masterObj.downcastCopy<AvatarWithHeapRSData>();
        ASSERT_EQ(master->getDummyData(), nullptr);
        ASSERT_NE(master->getMasterData(), nullptr);
        EXPECT_EQ(master->getMasterData()->value, 12345);
        master->getMasterData()->flagPtr = &masterDataDestroyed;

        auto dummy = _dummyObj.downcastCopy<AvatarWithHeapRSData>();
        ASSERT_EQ(dummy->getMasterData(), nullptr);
        ASSERT_NE(dummy->getDummyData(), nullptr);
        EXPECT_EQ(dummy->getDummyData()->value, 12345);
        dummy->getDummyData()->flagPtr = &dummyDataDestroyed;
    }

    ASSERT_TRUE(_destroyAvatarAndSyncDestruction());

    EXPECT_TRUE(masterDataDestroyed);
    EXPECT_TRUE(dummyDataDestroyed);
}

} // namespace spempe
} // namespace jbatnozic

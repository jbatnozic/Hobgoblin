// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Logging.hpp>
#include <SPeMPE/SPeMPE.hpp>

#include <gtest/gtest.h>

#include <array>
#include <memory>

using namespace jbatnozic::spempe;
using namespace hg::qao;
using namespace hg::rn;

#define HOST 0
#define CLI1 1
#define CLI2 2

class DefaultLobbyBackendServiceTest : public ::testing::Test {
public:
    void SetUp() override {
        RN_IndexHandlers();

        GameContext::RuntimeConfig rc{};
        _ctx[HOST] = std::make_unique<GameContext>(rc);
        _ctx[HOST]->setToMode(GameContext::Mode::Server);

        // Add networking service
        _netSvc[HOST] = QAO_Create<DefaultNetworkingService>(_ctx[HOST]->getQAORuntime().nonOwning(),
                                                             PRIORITY_NETMGR,
                                                             0);
        _netSvc[HOST]->setToServerMode(RN_Protocol::UDP, "pass", 2, 512, RN_NetworkingStack::Default);
        _netSvc[HOST]->getServer().start(0);

        _ctx[HOST]->attachComponent(*_netSvc[HOST]);

        // Add varmap service
        _svmSvc[HOST] = QAO_Create<DefaultSyncedVarmapService>(_ctx[HOST]->getQAORuntime().nonOwning(), PRIORITY_SVMMGR);
        _svmSvc[HOST]->setToMode(SyncedVarmapService::Mode::Host);

        _ctx[HOST]->attachComponent(*_svmSvc[HOST]);

        // Add lobby service
        _lobbySvc[HOST] = QAO_Create<DefaultLobbyBackendService>(_ctx[HOST]->getQAORuntime().nonOwning(), PRIORITY_LOBMGR);
        _lobbySvc[HOST]->setToHostMode(3);

        _ctx[HOST]->attachComponent(*_lobbySvc[HOST]);
    }

    void TearDown() override {
        _cleanupContext(CLI2);
        _cleanupContext(CLI1);
        _cleanupContext(HOST);
    }

protected:
    constexpr static int PRIORITY_SVMMGR = 11;
    constexpr static int PRIORITY_NETMGR = 10;
    constexpr static int PRIORITY_LOBMGR =  9;

    std::array<std::unique_ptr<GameContext>, 3> _ctx;
    std::array<QAO_Handle<DefaultNetworkingService>, 3> _netSvc;
    std::array<QAO_Handle<DefaultSyncedVarmapService>, 3> _svmSvc;
    std::array<QAO_Handle<DefaultLobbyBackendService>, 3> _lobbySvc;

    void _initClientContext(hg::PZInteger aContextIndex) {
        const auto pos = hg::pztos(aContextIndex);

        GameContext::RuntimeConfig rc{};
        _ctx[pos] = std::make_unique<GameContext>(rc);
        _ctx[pos]->setToMode(GameContext::Mode::Client);

        // Add networking service
        _netSvc[pos] = QAO_Create<DefaultNetworkingService>(_ctx[pos]->getQAORuntime().nonOwning(),
                                                            PRIORITY_NETMGR,
                                                            0);
        _netSvc[pos]->setToClientMode(RN_Protocol::UDP, "pass", 512, RN_NetworkingStack::Default);
        _netSvc[pos]->getClient().connectLocal(_netSvc[HOST]->getServer());

        _ctx[pos]->attachComponent(*_netSvc[pos]);

        // Add varmap service
        _svmSvc[pos] = QAO_Create<DefaultSyncedVarmapService>(_ctx[pos]->getQAORuntime().nonOwning(), PRIORITY_SVMMGR);
        _svmSvc[pos]->setToMode(SyncedVarmapService::Mode::Client);

        _ctx[pos]->attachComponent(*_svmSvc[pos]);

        // Add lobby service
        _lobbySvc[pos] = QAO_Create<DefaultLobbyBackendService>(_ctx[pos]->getQAORuntime().nonOwning(), PRIORITY_LOBMGR);
        _lobbySvc[pos]->setToClientMode(1);

        _ctx[pos]->attachComponent(*_lobbySvc[pos]);
    }

    void _cleanupContext(hg::PZInteger aContextIndex) {
        const auto pos = hg::pztos(aContextIndex);

        if (!_ctx[pos]) {
            return;
        }

        DetachStatus detachStatus;

        if (_lobbySvc[pos]) {
            _ctx[pos]->detachComponent<LobbyBackendService>(&detachStatus);
            ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);
            _lobbySvc[pos].reset();
        }

        if (_svmSvc[pos]) {
            _ctx[pos]->detachComponent<SyncedVarmapService>(&detachStatus);
            ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);
            _svmSvc[pos].reset();
        }

        if (_netSvc[pos]) {
            _ctx[pos]->detachComponent<NetworkingService>(&detachStatus);
            ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);
            _netSvc[pos].reset();
        }

        _ctx[pos].reset();
    }

    void _runAllContextsFor(hg::PZInteger aStepCount) {
        for (hg::PZInteger i = 0; i < aStepCount; i += 1) {
            if (_ctx[CLI2]) {
                _ctx[CLI2]->runFor(1);
            }
            if (_ctx[CLI1]) {
                _ctx[CLI1]->runFor(1);
            }
            if (_ctx[HOST]) {
                _ctx[HOST]->runFor(1);
            }
        }
    }
};

TEST_F(DefaultLobbyBackendServiceTest, InitialStateTest) {
    EXPECT_EQ(_lobbySvc[HOST]->getLocalPlayerIndex(), 0); // Host is in slot 0 unless changed

    _lobbySvc[HOST]->setLocalName("host");
    _lobbySvc[HOST]->setLocalUniqueId("1234");
    _lobbySvc[HOST]->setLocalCustomData(0, "cdat0");

    EXPECT_EQ(_lobbySvc[HOST]->getLocalName(), "host");
    EXPECT_EQ(_lobbySvc[HOST]->getLocalUniqueId(), "1234");
    EXPECT_EQ(_lobbySvc[HOST]->getLocalCustomData(0), "cdat0");

    // Check initial state - slot 0 should be the host and the other 2 empty
    {
        const auto& info = _lobbySvc[HOST]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[HOST]->getLockedInPlayerInfo(0), info);

        EXPECT_EQ(_lobbySvc[HOST]->playerIdxToClientIdx(0), CLIENT_INDEX_LOCAL);
    }
    {
        for (hg::PZInteger slot = 1; slot < 3; slot += 1) {
            const auto& info = _lobbySvc[HOST]->getPendingPlayerInfo(slot);
            EXPECT_EQ(info.name, "");
            EXPECT_EQ(info.uniqueId, "");
            EXPECT_EQ(info.customData[0], "");

            EXPECT_EQ(_lobbySvc[HOST]->getPendingPlayerInfo(slot), info);

            EXPECT_EQ(_lobbySvc[HOST]->playerIdxToClientIdx(slot), CLIENT_INDEX_UNKNOWN);
        }
    }
}

TEST_F(DefaultLobbyBackendServiceTest, SingleClientTest) {
    _lobbySvc[HOST]->setLocalName("host");
    _lobbySvc[HOST]->setLocalUniqueId("1234");
    _lobbySvc[HOST]->setLocalCustomData(0, "cdat0");

    _initClientContext(CLI1);
    _runAllContextsFor(2);

    ASSERT_EQ(_netSvc[CLI1]->getClient().getServerConnector().getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Connected);

    ASSERT_EQ(_lobbySvc[CLI1]->getSize(), 3);

    // Host is in both [0] slots
    {
        const auto& info = _lobbySvc[HOST]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[HOST]->getLockedInPlayerInfo(0), info);
    }
    // CLI1 is pending in slot [1] but locked-in slot [1] is empty
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(1);
        EXPECT_FALSE(info.isEmpty());
        EXPECT_FALSE(info.isComplete());
    }
    {
        const auto& info = _lobbySvc[CLI1]->getLockedInPlayerInfo(1);
        EXPECT_TRUE(info.isEmpty());
        EXPECT_EQ(info.customData[0], "");
    }
    // Both [2] slots are empty
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(2);
        EXPECT_TRUE(info.isEmpty());
        EXPECT_EQ(info.customData[0], "");

        EXPECT_EQ(_lobbySvc[CLI1]->getLockedInPlayerInfo(2), info);
    }

    _lobbySvc[CLI1]->setLocalName("cli1");
    _lobbySvc[CLI1]->setLocalUniqueId("5678");
    _lobbySvc[CLI1]->setLocalCustomData(0, "CDAT0");
    _lobbySvc[CLI1]->uploadLocalInfo();

    _runAllContextsFor(2);

    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(1);
        EXPECT_EQ(info.name, "cli1");
        EXPECT_EQ(info.uniqueId, "5678");
        EXPECT_EQ(info.customData[0], "CDAT0");
    }

    ASSERT_EQ(_lobbySvc[HOST]->clientIdxToPlayerIdx(0), PLAYER_INDEX_UNKNOWN);
    ASSERT_EQ(_lobbySvc[CLI1]->getLocalPlayerIndex(), PLAYER_INDEX_UNKNOWN);

    _lobbySvc[HOST]->lockInPendingChanges();

    _runAllContextsFor(2);

    ASSERT_EQ(_lobbySvc[CLI1]->getLocalPlayerIndex(), 1);
    ASSERT_EQ(_lobbySvc[HOST]->clientIdxToPlayerIdx(0), 1);

    // Test disconnect
    _netSvc[HOST]->getServer().setTimeoutLimit(std::chrono::microseconds{1});
    _netSvc[CLI1]->getClient().disconnect(false);
    _runAllContextsFor(4);
    ASSERT_EQ(_netSvc[HOST]->getServer().getClientConnector(0).getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Disconnected);

    {
        const auto& info = _lobbySvc[HOST]->getPendingPlayerInfo(1);
        EXPECT_TRUE(info.isEmpty());
    }
    {
        const auto& info = _lobbySvc[HOST]->getLockedInPlayerInfo(1);
        EXPECT_EQ(info.name, "cli1");
        EXPECT_EQ(info.uniqueId, "5678");
        EXPECT_EQ(info.customData[0], "CDAT0");

        EXPECT_FALSE(info.isEmpty());
        EXPECT_TRUE(info.isComplete());
    }
}

TEST_F(DefaultLobbyBackendServiceTest, MultipleClientsTest) {
    /*
     * SCENARIO: server start, client 1 connect, lock in, client 2 connect, client 1 disconnect, lock in
     */

    _lobbySvc[HOST]->setLocalName("host");
    _lobbySvc[HOST]->setLocalUniqueId("1234");
    _lobbySvc[HOST]->setLocalCustomData(0, "cdat0");

    _initClientContext(CLI1);
    _runAllContextsFor(2);

    ASSERT_EQ(_netSvc[CLI1]->getClient().getServerConnector().getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Connected);

    _lobbySvc[HOST]->lockInPendingChanges();

    _runAllContextsFor(2);

    // Verify slots from CLI1 perspective
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[CLI1]->getLockedInPlayerInfo(0), info);
    }
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(1);
        EXPECT_FALSE(info.isEmpty());

        EXPECT_EQ(_lobbySvc[CLI1]->getLockedInPlayerInfo(1), info);
    }
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(2);
        EXPECT_TRUE(info.isEmpty());
        EXPECT_EQ(info.customData[0], "");

        EXPECT_EQ(_lobbySvc[CLI1]->getLockedInPlayerInfo(2), info);
    }

    _initClientContext(CLI2);
    _runAllContextsFor(2);

    ASSERT_EQ(_netSvc[CLI2]->getClient().getServerConnector().getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Connected);

    // Verify slots from CLI1 perspective
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[CLI1]->getLockedInPlayerInfo(0), info);
    }
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(1);
        EXPECT_FALSE(info.isEmpty());

        EXPECT_EQ(_lobbySvc[CLI1]->getLockedInPlayerInfo(1), info);
    }
    {
        const auto& info = _lobbySvc[CLI1]->getPendingPlayerInfo(2);
        EXPECT_FALSE(info.isEmpty());
    }
    {
        const auto& info = _lobbySvc[CLI1]->getLockedInPlayerInfo(2);
        EXPECT_TRUE(info.isEmpty());
    }

    // Verify slots from CLI2 perspective
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[CLI2]->getLockedInPlayerInfo(0), info);
    }
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(1);
        EXPECT_FALSE(info.isEmpty());

        EXPECT_EQ(_lobbySvc[CLI2]->getLockedInPlayerInfo(1), info);
    }
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(2);
        EXPECT_FALSE(info.isEmpty());
    }
    {
        const auto& info = _lobbySvc[CLI2]->getLockedInPlayerInfo(2);
        EXPECT_TRUE(info.isEmpty());
    }

    _netSvc[HOST]->getServer().setTimeoutLimit(std::chrono::microseconds{1});
    _netSvc[CLI1]->getClient().disconnect(false);
    _runAllContextsFor(4);
    ASSERT_EQ(_netSvc[HOST]->getServer().getClientConnector(0).getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Disconnected);
    ASSERT_EQ(_netSvc[HOST]->getServer().getClientConnector(1).getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Connected);

    // Verify slots from CLI2 perspective
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[CLI2]->getLockedInPlayerInfo(0), info);
    }
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(1);
        EXPECT_TRUE(info.isEmpty());
    }
    {
        const auto& info = _lobbySvc[CLI2]->getLockedInPlayerInfo(1);
        EXPECT_FALSE(info.isEmpty());
    }
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(2);
        EXPECT_FALSE(info.isEmpty());
    }
    {
        const auto& info = _lobbySvc[CLI2]->getLockedInPlayerInfo(2);
        EXPECT_TRUE(info.isEmpty());
    }

    _lobbySvc[HOST]->lockInPendingChanges();
    _runAllContextsFor(2);

    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(0);
        EXPECT_EQ(info.name, "host");
        EXPECT_EQ(info.uniqueId, "1234");
        EXPECT_EQ(info.customData[0], "cdat0");

        EXPECT_EQ(_lobbySvc[CLI2]->getLockedInPlayerInfo(0), info);
    }
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(1);
        EXPECT_TRUE(info.isEmpty());

        EXPECT_EQ(_lobbySvc[CLI2]->getLockedInPlayerInfo(1), info);
    }
    {
        const auto& info = _lobbySvc[CLI2]->getPendingPlayerInfo(2);
        EXPECT_FALSE(info.isEmpty());

        EXPECT_EQ(_lobbySvc[CLI2]->getLockedInPlayerInfo(2), info);
    }
}

TEST_F(DefaultLobbyBackendServiceTest, SwapSlotsTest) {
    // TODO
}

// clang-format on

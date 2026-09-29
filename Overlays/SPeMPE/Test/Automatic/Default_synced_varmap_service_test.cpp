// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off


#include <Hobgoblin/Common.hpp>
#include <SPeMPE/SPeMPE.hpp>

#include <gtest/gtest.h>

#include <memory>

using namespace jbatnozic::spempe;
using namespace hg::qao;

class DefaultSyncedVarmapServiceTest : public ::testing::Test {
public:
    void SetUp() override {
        hg::RN_IndexHandlers();

        GameContext::RuntimeConfig rc{};
        _ctx1 = std::make_unique<GameContext>(rc);
        _ctx1->setToMode(GameContext::Mode::Server);
        _ctx2 = std::make_unique<GameContext>(rc);
        _ctx2->setToMode(GameContext::Mode::Client);

        // Add networking services
        _netSvc1 = QAO_Create<DefaultNetworkingService>(_ctx1->getQAORuntime().nonOwning(),
                                                        PRIORITY_NETMGR,
                                                        0);
        _netSvc1->setToServerMode(hg::RN_Protocol::UDP, "pass", 2, 512, hg::RN_NetworkingStack::Default);

        _netSvc2 = QAO_Create<DefaultNetworkingService>(_ctx2->getQAORuntime().nonOwning(),
                                                        PRIORITY_NETMGR,
                                                        0);
        _netSvc2->setToClientMode(hg::RN_Protocol::UDP, "pass", 512, hg::RN_NetworkingStack::Default);

        {
            auto& server = _netSvc1->getServer();
            auto& client = _netSvc2->getClient();

            server.start(0);
            client.connectLocal(server);
        }

        _ctx1->attachComponent(*_netSvc1);
        _ctx2->attachComponent(*_netSvc2);

        // Add varmap services
        _svmSvc1 = QAO_Create<DefaultSyncedVarmapService>(_ctx1->getQAORuntime().nonOwning(), PRIORITY_SVMMGR);
        _svmSvc1->setToMode(SyncedVarmapService::Mode::Host);

        _svmSvc2 = QAO_Create<DefaultSyncedVarmapService>(_ctx2->getQAORuntime().nonOwning(), PRIORITY_SVMMGR);
        _svmSvc2->setToMode(SyncedVarmapService::Mode::Client);

        _ctx1->attachComponent(*_svmSvc1);
        _ctx2->attachComponent(*_svmSvc2);

        // Run both contexts a little to propagate the connection
        _ctx2->runFor(1);
        _ctx1->runFor(1);
        _ctx2->runFor(1);
        _ctx1->runFor(1);

        ASSERT_EQ(_netSvc2->getClient().getServerConnector().getStatus(),
                  jbatnozic::hobgoblin::RN_ConnectorStatus::Connected);
        ASSERT_EQ(_netSvc2->getLocalClientIndex(), 0);
    }

    void TearDown() override {
        _netSvc2->getClient().disconnect(false);
        _netSvc1->getServer().stop();

        DetachStatus detachStatus;
        _ctx1->detachComponent<NetworkingService>(&detachStatus);
        ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);
        _ctx1->detachComponent<SyncedVarmapService>(&detachStatus);
        ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);
        _ctx2->detachComponent<NetworkingService>(&detachStatus);
        ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);
        _ctx2->detachComponent<SyncedVarmapService>(&detachStatus);
        ASSERT_EQ(detachStatus, DetachStatus::NOT_OWNED_BY_CONTEXT);

        _svmSvc1.reset();
        _netSvc1.reset();
        _svmSvc2.reset();
        _netSvc2.reset();
        
        _ctx1.reset();
        _ctx2.reset();
    }

protected:
    constexpr static int PRIORITY_SVMMGR = 11;
    constexpr static int PRIORITY_NETMGR = 10;

    constexpr static auto ALLOWED = DefaultSyncedVarmapService::ALLOWED;

    std::unique_ptr<GameContext> _ctx1;
    std::unique_ptr<GameContext> _ctx2;

    QAO_Handle<DefaultNetworkingService> _netSvc1;
    QAO_Handle<DefaultNetworkingService> _netSvc2;

    QAO_Handle<DefaultSyncedVarmapService> _svmSvc1;
    QAO_Handle<DefaultSyncedVarmapService> _svmSvc2;
};

TEST_F(DefaultSyncedVarmapServiceTest, BasicFunctionalityTest) {
    // Check that there are no values:
    ASSERT_FALSE(_svmSvc1->getInt64("valInt64_h").has_value());
    ASSERT_FALSE(_svmSvc1->getDouble("valDouble_h").has_value());
    ASSERT_FALSE(_svmSvc1->getString("valString_h").has_value());

    ASSERT_FALSE(_svmSvc1->getInt64("valInt64_c").has_value());
    ASSERT_FALSE(_svmSvc1->getDouble("valDouble_c").has_value());
    ASSERT_FALSE(_svmSvc1->getString("valString_c").has_value());

    ASSERT_FALSE(_svmSvc2->getInt64("valInt64_h").has_value());
    ASSERT_FALSE(_svmSvc2->getDouble("valDouble_h").has_value());
    ASSERT_FALSE(_svmSvc2->getString("valString_h").has_value());

    ASSERT_FALSE(_svmSvc2->getInt64("valInt64_c").has_value());
    ASSERT_FALSE(_svmSvc2->getDouble("valDouble_c").has_value());
    ASSERT_FALSE(_svmSvc2->getString("valString_c").has_value());

    // Authorize client & set some values from host side:
    _svmSvc1->int64SetClientWritePermission("valInt64_c", 1, ALLOWED);
    _svmSvc1->doubleSetClientWritePermission("valDouble_c", 1, ALLOWED);
    _svmSvc1->stringSetClientWritePermission("valString_c", 1, ALLOWED);

    _svmSvc1->setInt64("valInt64_h", 5);
    _svmSvc1->setDouble("valDouble_h", 5.0);
    _svmSvc1->setString("valString_h", "5s");

    // Then set some values from client side:
    _svmSvc2->requestToSetInt64("valInt64_c", 6);
    _svmSvc2->requestToSetDouble("valDouble_c", 6.0);
    _svmSvc2->requestToSetString("valString_c", "6s");

    // Run both contexts a bit to propagate the values
    _ctx2->runFor(1);
    _ctx1->runFor(1);
    _ctx2->runFor(1);
    _ctx1->runFor(1);

    // Check that the values are correct now:
    ASSERT_EQ(_svmSvc1->getInt64("valInt64_h").value(), 5);
    ASSERT_EQ(_svmSvc1->getDouble("valDouble_h").value(), 5.0);
    ASSERT_EQ(_svmSvc1->getString("valString_h").value(), "5s");

    ASSERT_EQ(_svmSvc1->getInt64("valInt64_c").value(), 6);
    ASSERT_EQ(_svmSvc1->getDouble("valDouble_c").value(), 6.0);
    ASSERT_EQ(_svmSvc1->getString("valString_c").value(), "6s");

    ASSERT_EQ(_svmSvc2->getInt64("valInt64_h").value(), 5);
    ASSERT_EQ(_svmSvc2->getDouble("valDouble_h").value(), 5.0);
    ASSERT_EQ(_svmSvc2->getString("valString_h").value(), "5s");

    ASSERT_EQ(_svmSvc2->getInt64("valInt64_c").value(), 6);
    ASSERT_EQ(_svmSvc2->getDouble("valDouble_c").value(), 6.0);
    ASSERT_EQ(_svmSvc2->getString("valString_c").value(), "6s");
}

TEST_F(DefaultSyncedVarmapServiceTest, ClientTriesToSetValueAndPermission_ThrowsException) {
    EXPECT_THROW(_svmSvc2->setInt64("valInt64_c", 6), hg::TracedLogicError);
    EXPECT_THROW(_svmSvc2->setDouble("valDouble_c", 6.0), hg::TracedLogicError);
    EXPECT_THROW(_svmSvc2->setString("valString_c", "6s"), hg::TracedLogicError);

    EXPECT_THROW(_svmSvc2->int64SetClientWritePermission("valInt64_c", 1, ALLOWED), hg::TracedLogicError);
    EXPECT_THROW(_svmSvc2->doubleSetClientWritePermission("valDouble_c", 1, ALLOWED), hg::TracedLogicError);
    EXPECT_THROW(_svmSvc2->stringSetClientWritePermission("valString_c", 1, ALLOWED), hg::TracedLogicError);
}

TEST_F(DefaultSyncedVarmapServiceTest, HostTriesToRequestSet_ThrowsException) {
    EXPECT_THROW(_svmSvc1->requestToSetInt64("valInt64_h", 5), hg::TracedLogicError);
    EXPECT_THROW(_svmSvc1->requestToSetDouble("valDouble_h", 5.0), hg::TracedLogicError);
    EXPECT_THROW(_svmSvc1->requestToSetString("valString_h", "5s"), hg::TracedLogicError);
}

TEST_F(DefaultSyncedVarmapServiceTest, ClientRequestsToSetUnauthorizedValue_Disconnects) {
    _svmSvc2->requestToSetInt64("valInt64_c", 6);

    // Run both contexts a bit to propagate the values
    _ctx2->runFor(1);
    _ctx1->runFor(1);
    _ctx2->runFor(1);
    _ctx1->runFor(1);

    // Check that the client has been disconnected
    ASSERT_EQ(_netSvc1->getServer().getClientConnector(0).getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Disconnected);

    ASSERT_EQ(_netSvc2->getClient().getServerConnector().getStatus(),
              jbatnozic::hobgoblin::RN_ConnectorStatus::Disconnected);
}

// clang-format on

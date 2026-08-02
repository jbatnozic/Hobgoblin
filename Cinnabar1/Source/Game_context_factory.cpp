// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Game_context_factory.hpp>

#include <Graphics_system_provider.hpp>
#include <Main_game_flow_manager.hpp>

namespace cinnabar {

namespace {
// clang-format off
const spe::GameContext::RuntimeConfig RUNTIME_CONFIG = {
    .tickRate              = spe::TickRate{60},
    .maxConsecutiveUpdates = 2
};

const spe::WindowManagerInterface::WindowConfig WINDOW_CONFIG = {
    .size  = {1280, 720},
    .title = "Cinnamon1",
    .style = uwga::WindowStyle::DEFAULT
};

spe::WindowManagerInterface::MainRenderTextureConfig MRT_CONFIG = {
    .size           = {1920, 1080},
    .smooth         = true,
    .batchingConfig = {
        .strategy = uwga::BatchingConfig::Strategy::FAST_N_LOOSE
    },
    .clearingColor  = uwga::COLOR_BLACK
};

spe::WindowManagerInterface::TimingConfig TIMING_CONFIG = {
    spe::FrameRate{120},
    spe::PREVENT_BUSY_WAIT_ON,
    spe::VSYNC_OFF
};

constexpr int DEFAULT_STATE_BUFFERING_LENGTH = 3;
// clang-format on

std::unique_ptr<spe::GameContext> CreateBasicGameContext() {
    auto ctx = std::make_unique<spe::GameContext>(RUNTIME_CONFIG);

    // clang-format off
    QAO_Create<spe::EventLoopTimingReporter>(
        ctx->getQAORuntime(),
        0,
        spe::EventLoopTimingReporter::Config{
            .cycleLength = RUNTIME_CONFIG.tickRate.getValue() * 10 // Every 10 seconds
        }
    );
    // clang-format on

    {
        auto uwgaSystem             = uwga::CreateGraphicsSystem("SFML");
        auto graphicsSystemProvider = std::make_unique<GraphicsSystemProvider>(std::move(uwgaSystem));
        ctx->attachAndOwnComponent(std::move(graphicsSystemProvider));
    }

    return ctx;
}

void AttachWindowManager(spe::GameContext& aContext, GameContextMode aMode) {
    auto graphicsSystemPtr = aContext.getComponent<GraphicsSystemProvider>().getSystemPtr();

    auto winMgr =
        QAO_Create<spe::DefaultWindowManager>(aContext.getQAORuntime().nonOwning(), PRIORITY_WINDOWMGR);

    auto windowConfig = WINDOW_CONFIG;

    switch (aMode) {
    case GameContextMode::SERVER:
        winMgr->setToHeadlessMode(TIMING_CONFIG);
        break;

    case GameContextMode::CLIENT:
    case GameContextMode::DEV:
        if (aMode == GameContextMode::DEV) {
            windowConfig.title.append(" (Developer Mode)");
        }
        winMgr->setToNormalMode(graphicsSystemPtr, windowConfig, MRT_CONFIG, TIMING_CONFIG);
        winMgr->setStopIfCloseClicked(true);
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    aContext.attachAndOwnComponent(std::move(winMgr));
}

void AttachNetworkingManager(spe::GameContext& aContext, GameContextMode aMode) {
    auto netMgr = QAO_Create<spe::DefaultNetworkingManager>(aContext.getQAORuntime().nonOwning(),
                                                            PRIORITY_NETWORKMGR,
                                                            DEFAULT_STATE_BUFFERING_LENGTH);

    switch (aMode) {
    case GameContextMode::SERVER:
    case GameContextMode::DEV:
        {
            netMgr->setToServerMode(RN_Protocol::UDP,
                                    "dreamscape-escape",
                                    1,
                                    1024,
                                    RN_NetworkingStack::Default);
            netMgr->setPacemakerPulsePeriod(300);
            auto& server = netMgr->getServer();
            server.setTimeoutLimit(std::chrono::seconds{5});
            // server.setRetransmitPredicate(&MyRetransmitPredicate);
            server.start(8888);
        }
        break;

    case GameContextMode::CLIENT:
        {
            netMgr->setToClientMode(RN_Protocol::UDP,
                                    "dreamscape-escape",
                                    1024,
                                    RN_NetworkingStack::Default);
            auto& client = netMgr->getClient();
            client.setTimeoutLimit(std::chrono::seconds{5});
            // client.setRetransmitPredicate(&MyRetransmitPredicate);
            // client.connect(aLocalPort, aRemoteIp, aRemotePort);
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    aContext.attachAndOwnComponent(std::move(netMgr));
}

// MARK: DEV

std::unique_ptr<spe::GameContext> CreateDevGameContext() {
    auto ctx = CreateBasicGameContext();
    ctx->setToMode(spe::GameContext::Mode::GameMaster);

    auto graphicsSystemPtr = ctx->getComponent<GraphicsSystemProvider>().getSystemPtr();

    // Set EXECON level
    {
        constexpr auto CALLER_ID = "ctx_create";
        ctx->getGameState().setUpdateExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDrawExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDisplayExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
    }

    AttachWindowManager(*ctx, GameContextMode::DEV);
    AttachNetworkingManager(*ctx, GameContextMode::DEV);

    // Add MainGameFlowManager
    {
        auto mgfMgr = QAO_Create<MainGameFlowManager>(ctx->getQAORuntime().nonOwning());
        ctx->attachAndOwnComponent(std::move(mgfMgr));
    }

    return ctx;
}
} // namespace

std::unique_ptr<spe::GameContext> CreateGameContext(GameContextMode aMode) {
    switch (aMode) {
    case GameContextMode::SERVER:
        HG_NOT_IMPLEMENTED(); // TODO
        break;

    case GameContextMode::CLIENT:
        HG_NOT_IMPLEMENTED(); // TODO
        break;

    case GameContextMode::DEV:
        return CreateDevGameContext();
        break;

    default:
        HG_UNREACHABLE("Invalid value for GameContextMode ({}).", (int)aMode);
    }
}

} // namespace cinnabar

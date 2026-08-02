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
    .size           = {1920 / 2, 1080 / 2},
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
        auto uwgaSystem = uwga::CreateGraphicsSystem("SFML");
        auto graphicsSystemProvider = std::make_unique<GraphicsSystemProvider>(std::move(uwgaSystem));
        ctx->attachAndOwnComponent(std::move(graphicsSystemProvider));
    }

    return ctx;
}

// MARK: DEV

std::unique_ptr<spe::GameContext> CreateDevGameContext() {
    auto ctx = CreateBasicGameContext();

    auto graphicsSystemPtr = ctx->getComponent<GraphicsSystemProvider>().getSystemPtr();

    // Set EXECON level
    {
        constexpr auto CALLER_ID = "ctx_create";
        ctx->getGameState().setUpdateExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDrawExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDisplayExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
    }
    // Add WindowManager
    {
        auto winMgr =
            QAO_Create<spe::DefaultWindowManager>(ctx->getQAORuntime().nonOwning(), PRIORITY_WINDOWMGR);

        auto windowConfig = WINDOW_CONFIG;
        windowConfig.title.append(" (Developer Mode)");

        winMgr->setToNormalMode(graphicsSystemPtr,
                                windowConfig,
                                MRT_CONFIG,
                                TIMING_CONFIG);

        winMgr->setStopIfCloseClicked(true);

        ctx->attachAndOwnComponent(std::move(winMgr));
    }
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

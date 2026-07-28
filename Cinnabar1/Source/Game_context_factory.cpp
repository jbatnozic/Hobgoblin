// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Game_context_factory.hpp>

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
// clang-format on

std::unique_ptr<spe::GameContext> CreateBasicGameContext() {
    return std::make_unique<spe::GameContext>(RUNTIME_CONFIG);
}

// MARK: DEV

std::unique_ptr<spe::GameContext> CreateDevGameContext() {
    auto ctx = CreateBasicGameContext();
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

        winMgr->setToNormalMode(uwga::CreateGraphicsSystem("SFML"),
                                windowConfig,
                                MRT_CONFIG,
                                TIMING_CONFIG);

        ctx->attachAndOwnComponent(std::move(winMgr));
    }
    // Add ???
    {
        // TODO
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

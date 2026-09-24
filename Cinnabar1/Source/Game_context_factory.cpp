// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Game_context_factory.hpp>

#include <Graphics_system_provider.hpp>
#include <Lobby_frontend_manager_default.hpp>
#include <Main_game_flow_manager_default.hpp>
#include <Player_controls.hpp>

#include <Hobgoblin/RmlUi.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <string>

namespace cinnabar {

namespace {
// clang-format off
const spe::GameContext::RuntimeConfig RUNTIME_CONFIG = {
    .tickRate              = spe::TickRate{60},
    .maxConsecutiveUpdates = 2
};

const spe::WindowManager::WindowConfig WINDOW_CONFIG = {
    .size  = {1280, 720},
    .title = "Cinnabar1",
    .style = uwga::WindowStyle::DEFAULT
};

spe::WindowManager::MainRenderTextureConfig MRT_CONFIG = {
    .size           = {1920, 1080},
    .smooth         = true,
    .batchingConfig = {
        .strategy = uwga::BatchingConfig::Strategy::FAST_N_LOOSE
    },
    .clearingColor  = uwga::COLOR_BLACK
};

spe::WindowManager::TimingConfig TIMING_CONFIG = {
    spe::FrameRate{120},
    spe::PREVENT_BUSY_WAIT_ON,
    spe::VSYNC_OFF
};

constexpr int DEFAULT_STATE_BUFFERING_LENGTH = 3;

//! The size of the lobby (number of player slots) when hosting.
//! For now there's just a single slot for the local host.
constexpr hg::PZInteger DEFAULT_LOBBY_SIZE = 1;
// clang-format on

//! Loads the GUI fonts needed by the RmlUi documents (such as the lobby).
//! Must be called after the window manager has been switched to normal mode.
void LoadGuiFonts() {
    struct FontFace {
        Rml::String filename;
        bool        fallbackFace;
    };
    const FontFace fontFaces[] = {
        {   "LatoLatin-Regular.ttf", false},
        {    "LatoLatin-Italic.ttf", false},
        {      "LatoLatin-Bold.ttf", false},
        {"LatoLatin-BoldItalic.ttf", false},
    };
    for (const auto& face : fontFaces) {
        Rml::LoadFontFace("assets/fonts/" + face.filename, face.fallbackFace);
    }
}

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

// MARK: Manager factories

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

        // Set up RmlUi GUI (fonts + debugger) now that the window is in normal mode.
        LoadGuiFonts();
        Rml::Debugger::Initialise(&(winMgr->getGUIContext()));
        Rml::Debugger::SetVisible(true);
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

void AttachInputSyncManager(spe::GameContext& aContext, GameContextMode aMode) {
    auto insMgr = QAO_Create<spe::DefaultInputSyncManager>(aContext.getQAORuntime().nonOwning(),
                                                           PRIORITY_INPUTMGR);

    switch (aMode) {
    case GameContextMode::SERVER:
    case GameContextMode::DEV:
        // Host mode. With 0 remote clients the manager just echoes the local player's input.
        insMgr->setToHostMode(0, DEFAULT_STATE_BUFFERING_LENGTH);
        break;

    case GameContextMode::CLIENT:
        insMgr->setToClientMode();
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    // Define the 4 directional controls (identically on the host and on clients).
    SetUpPlayerControlsDefinitions(*insMgr);

    aContext.attachAndOwnComponent(std::move(insMgr));
}

void AttachSyncedVarmapManager(spe::GameContext& aContext, GameContextMode aMode) {
    auto svmMgr = QAO_Create<spe::DefaultSyncedVarmapManager>(aContext.getQAORuntime().nonOwning(),
                                                              PRIORITY_VARMAPMGR);

    switch (aMode) {
    case GameContextMode::SERVER:
    case GameContextMode::DEV:
        svmMgr->setToMode(spe::SyncedVarmapManager::Mode::Host);
        break;

    case GameContextMode::CLIENT:
        svmMgr->setToMode(spe::SyncedVarmapManager::Mode::Client);
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    aContext.attachAndOwnComponent(std::move(svmMgr));
}

void AttachLobbyBackendManager(spe::GameContext& aContext, GameContextMode aMode) {
    // NOTE: Requires the Synced varmap manager and Networking manager to be attached already.
    auto lobbyMgr = QAO_Create<spe::DefaultLobbyBackendManager>(aContext.getQAORuntime().nonOwning(),
                                                                PRIORITY_LOBBYBACKMGR);

    switch (aMode) {
    case GameContextMode::SERVER:
    case GameContextMode::DEV:
        lobbyMgr->setToHostMode(DEFAULT_LOBBY_SIZE);
        break;

    case GameContextMode::CLIENT:
        lobbyMgr->setToClientMode(DEFAULT_LOBBY_SIZE);
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    aContext.attachAndOwnComponent(std::move(lobbyMgr));
}

void AttachLobbyFrontendManager(spe::GameContext& aContext, GameContextMode aMode) {
    // NOTE: Requires the Lobby backend manager and (in windowed modes) the Window manager.
    auto lobbyFrontendMgr = QAO_Create<DefaultLobbyFrontendManager>(aContext.getQAORuntime().nonOwning(),
                                                                    PRIORITY_LOBBYFRONTMGR);

    switch (aMode) {
    case GameContextMode::SERVER:
        lobbyFrontendMgr->setToHeadlessHostMode();
        break;

    case GameContextMode::CLIENT:
    case GameContextMode::DEV:
        {
            // DEV mode is a windowed host, so it uses the (windowed) client-mode frontend
            // to actually display the lobby GUI.
            const auto name = "player_" + std::to_string(hg::util::GetRandomNumber<int>(10'000, 99'999));
            const auto uniqueId = "id_" + std::to_string(hg::util::GetRandomNumber<int>(10'000, 99'999));
            lobbyFrontendMgr->setToClientMode(name, uniqueId);
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    aContext.attachAndOwnComponent(std::move(lobbyFrontendMgr));
}

void AttachAuthorizationManager(spe::GameContext& aContext, GameContextMode aMode) {
    auto authMgr = QAO_Create<spe::DefaultAuthorizationManager>(aContext.getQAORuntime().nonOwning(),
                                                                PRIORITY_AUTHMGR);

    switch (aMode) {
    case GameContextMode::SERVER:
    case GameContextMode::DEV:
        authMgr->setToHostMode();
        break;

    case GameContextMode::CLIENT:
        authMgr->setToClientMode();
        break;

    default:
        HG_UNREACHABLE("Invalid value for enum GameContextMode ({}).", (int)aMode);
        break;
    }

    aContext.attachAndOwnComponent(std::move(authMgr));
}

// MARK: SERVER context

std::unique_ptr<spe::GameContext> CreateServerGameContext() {
    constexpr auto GCMODE = GameContextMode::SERVER;

    auto ctx = CreateBasicGameContext();
    ctx->setToMode(spe::GameContext::Mode::Server);

    // Set EXECON level
    {
        constexpr auto CALLER_ID = "ctx_create";
        ctx->getGameState().setUpdateExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        // Headless server doesn't draw anything
        ctx->getGameState().setDrawExeconLevel(QAO_ExeCon::META_EXECUTE_NONE, CALLER_ID);
        // Need WindowManager (ESSENTIAL) still needs to run its Display event even in Headless mode
        ctx->getGameState().setDisplayExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
    }

    AttachWindowManager(*ctx, GCMODE);
    AttachNetworkingManager(*ctx, GCMODE);

    // NOTE: attachment order matters here, because some managers look up others during setup:
    //   - the Lobby backend manager needs the Synced varmap and Networking managers,
    //   - the Lobby frontend manager needs the Lobby backend and Window managers,
    //   - the Authorization manager needs the Lobby backend, Synced varmap and Networking managers.
    AttachInputSyncManager(*ctx, GCMODE);
    AttachSyncedVarmapManager(*ctx, GCMODE);
    AttachLobbyBackendManager(*ctx, GCMODE);
    AttachLobbyFrontendManager(*ctx, GCMODE);
    AttachAuthorizationManager(*ctx, GCMODE);

    // Add DefaultMainGameFlowManager
    {
        auto mgfMgr = QAO_Create<DefaultMainGameFlowManager>(ctx->getQAORuntime().nonOwning());
        ctx->attachAndOwnComponent(std::move(mgfMgr));
    }

    return ctx;
}

// MARK: CLIENT context

std::unique_ptr<spe::GameContext> CreateClientGameContext() {
    constexpr auto GCMODE = GameContextMode::CLIENT;

    auto ctx = CreateBasicGameContext();
    ctx->setToMode(spe::GameContext::Mode::Client);

    // Set EXECON level
    {
        constexpr auto CALLER_ID = "ctx_create";
        ctx->getGameState().setUpdateExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDrawExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDisplayExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
    }

    AttachWindowManager(*ctx, GCMODE);
    AttachNetworkingManager(*ctx, GCMODE);

    // NOTE: attachment order matters here, because some managers look up others during setup:
    //   - the Lobby backend manager needs the Synced varmap and Networking managers,
    //   - the Lobby frontend manager needs the Lobby backend and Window managers,
    //   - the Authorization manager needs the Lobby backend, Synced varmap and Networking managers.
    AttachInputSyncManager(*ctx, GCMODE);
    AttachSyncedVarmapManager(*ctx, GCMODE);
    AttachLobbyBackendManager(*ctx, GCMODE);
    AttachLobbyFrontendManager(*ctx, GCMODE);
    AttachAuthorizationManager(*ctx, GCMODE);

    // Add DefaultMainGameFlowManager
    {
        auto mgfMgr = QAO_Create<DefaultMainGameFlowManager>(ctx->getQAORuntime().nonOwning());
        ctx->attachAndOwnComponent(std::move(mgfMgr));
    }

    return ctx;
}

// MARK: DEV context

std::unique_ptr<spe::GameContext> CreateDevGameContext() {
    constexpr auto GCMODE = GameContextMode::DEV;

    auto ctx = CreateBasicGameContext();
    ctx->setToMode(spe::GameContext::Mode::GameMaster);

    // Set EXECON level
    {
        constexpr auto CALLER_ID = "ctx_create";
        ctx->getGameState().setUpdateExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDrawExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
        ctx->getGameState().setDisplayExeconLevel(QAO_ExeCon::ESSENTIAL, CALLER_ID);
    }

    AttachWindowManager(*ctx, GCMODE);
    AttachNetworkingManager(*ctx, GCMODE);

    // NOTE: attachment order matters here, because some managers look up others during setup:
    //   - the Lobby backend manager needs the Synced varmap and Networking managers,
    //   - the Lobby frontend manager needs the Lobby backend and Window managers,
    //   - the Authorization manager needs the Lobby backend, Synced varmap and Networking managers.
    AttachInputSyncManager(*ctx, GCMODE);
    AttachSyncedVarmapManager(*ctx, GCMODE);
    AttachLobbyBackendManager(*ctx, GCMODE);
    AttachLobbyFrontendManager(*ctx, GCMODE);
    AttachAuthorizationManager(*ctx, GCMODE);

    // Add DefaultMainGameFlowManager
    {
        auto mgfMgr = QAO_Create<DefaultMainGameFlowManager>(ctx->getQAORuntime().nonOwning());
        ctx->attachAndOwnComponent(std::move(mgfMgr));
    }

    return ctx;
}

} // namespace

std::unique_ptr<spe::GameContext> CreateGameContext(GameContextMode aMode) {
    switch (aMode) {
    case GameContextMode::SERVER:
        return CreateServerGameContext();
        break;

    case GameContextMode::CLIENT:
        return CreateClientGameContext();
        break;

    case GameContextMode::DEV:
        return CreateDevGameContext();
        break;

    default:
        HG_UNREACHABLE("Invalid value for GameContextMode ({}).", (int)aMode);
    }
}

} // namespace cinnabar

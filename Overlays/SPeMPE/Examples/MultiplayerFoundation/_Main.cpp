// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Engine.h"
#include "Lobby_frontend_service_default.hpp"
#include "Main_gameplay_service_default.hpp"

#include "Player_character_alternating.hpp"
#include "Player_character_autodiff.hpp"
#include "Player_character_autodiff_alternating.hpp"
#include "Player_character_basic.hpp"

#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <cstdint>

static constexpr auto LOG_ID = "MultiplayerFoundation";

///////////////////////////////////////////////////////////////////////////
// GAME CONFIG                                                           //
///////////////////////////////////////////////////////////////////////////

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800
#define TICK_RATE     60
#define FRAME_RATE    120

bool MyRetransmitPredicate(hg::PZInteger             aCyclesSinceLastTransmit,
                           std::chrono::microseconds aTimeSinceLastSend,
                           std::chrono::microseconds aCurrentLatency) {
    // Default behaviour:
    return RN_DefaultRetransmitPredicate(aCyclesSinceLastTransmit, aTimeSinceLastSend, aCurrentLatency);
    // Aggressive retransmission:
    // return 1;
}

enum class GameMode {
    Server,
    Client
};

std::unique_ptr<spe::GameContext> MakeGameContext(GameMode      aGameMode,
                                                  std::uint16_t aLocalPort,
                                                  std::uint16_t aRemotePort,
                                                  std::string   aRemoteIp,
                                                  hg::PZInteger aPlayerCount) {
    auto context =
        std::make_unique<spe::GameContext>(spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode((aGameMode == GameMode::Server) ? spe::GameContext::Mode::Server
                                                       : spe::GameContext::Mode::Client);

    // Create and attach a Window service
    auto winSvc =
        QAO_Create<spe::DefaultWindowService>(context->getQAORuntime().nonOwning(), PRIORITY_WINDOWMGR);
    spe::WindowService::TimingConfig timingConfig{
#ifdef _MSC_VER
        spe::FrameRate{FRAME_RATE},
        spe::PREVENT_BUSY_WAIT_ON,
        spe::VSYNC_OFF
#else
        FRAME_RATE,
        ((aGameMode == GameMode::Server) ? spe::PREVENT_BUSY_WAIT_ON : spe::PREVENT_BUSY_WAIT_OFF),
        spe::VSYNC_OFF
#endif
    };
    if (aGameMode == GameMode::Server) {
        winSvc->setToHeadlessMode(timingConfig);
    } else {
        winSvc->setToNormalMode(
            hg::uwga::CreateGraphicsSystem("SFML"),
            spe::WindowService::WindowConfig{
                .size  = {WINDOW_WIDTH, WINDOW_HEIGHT},
                .title = "SPeMPE Multiplayer Foundation",
                .style = hg::uwga::WindowStyle::DEFAULT
        },
            spe::WindowService::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
            timingConfig);

        struct FontFace {
            Rml::String filename;
            bool        fallback_face;
        };
        FontFace font_faces[] = {
            {   "LatoLatin-Regular.ttf", false},
            {    "LatoLatin-Italic.ttf", false},
            {      "LatoLatin-Bold.ttf", false},
            {"LatoLatin-BoldItalic.ttf", false},
        };
        for (const FontFace& face : font_faces) {
            Rml::LoadFontFace("assets/fonts/" + face.filename, face.fallback_face);
        }

        Rml::Debugger::Initialise(&(winSvc->getGUIContext()));
        Rml::Debugger::SetVisible(true);
    }

    context->attachAndOwnComponent(std::move(winSvc));

    // Create and attach a Networking service
    auto netSvc = QAO_Create<spe::DefaultNetworkingService>(context->getQAORuntime().nonOwning(),
                                                            PRIORITY_NETWORKMGR,
                                                            STATE_BUFFERING_LENGTH);
    if (aGameMode == GameMode::Server) {
        netSvc->setToServerMode(
            RN_Protocol::UDP,
            "minimal-multiplayer",
            aPlayerCount -
                1, // -1 because player 0 is the host itself (even if it doesn't participate in the game)
            1024,
            RN_NetworkingStack::Default);
        netSvc->setPacemakerPulsePeriod(120);
        auto& server = netSvc->getServer();
        server.setTimeoutLimit(std::chrono::seconds{5});
        server.setRetransmitPredicate(&MyRetransmitPredicate);
        server.start(aLocalPort);

        std::printf("Server started on port %d for up to %d clients.\n",
                    (int)server.getLocalPort(),
                    aPlayerCount - 1);
    } else {
        netSvc->setToClientMode(RN_Protocol::UDP,
                                "minimal-multiplayer",
                                1024,
                                RN_NetworkingStack::Default);
        auto& client = netSvc->getClient();
        client.setTimeoutLimit(std::chrono::seconds{5});
        client.setRetransmitPredicate(&MyRetransmitPredicate);
        client.connect(aLocalPort, aRemoteIp, aRemotePort);

        std::printf("Client started on port %d (connecting to %s:%d)\n",
                    (int)client.getLocalPort(),
                    aRemoteIp.c_str(),
                    (int)aRemotePort);
    }
    netSvc->setTelemetryCycleLimit(120);
    context->attachAndOwnComponent(std::move(netSvc));

    // Create and attack an Input sync service
    auto insSvc = QAO_Create<spe::DefaultInputSyncService>(context->getQAORuntime().nonOwning(),
                                                           PRIORITY_INPUTMGR);

    if (aGameMode == GameMode::Server) {
        insSvc->setToHostMode(aPlayerCount - 1, STATE_BUFFERING_LENGTH);
    } else {
        insSvc->setToClientMode();
    }

    /* Either way, define the inputs in the same way */
    {
        spe::InputSyncServiceWrapper wrapper{*insSvc};
        wrapper.defineSignal<bool>("left", false);
        wrapper.defineSignal<bool>("right", false);
        wrapper.defineSignal<bool>("up", false);
        wrapper.defineSignal<bool>("down", false);
        wrapper.defineSimpleEvent("jump");
    }

    context->attachAndOwnComponent(std::move(insSvc));

    // Create and attach a varmap service
    auto svmSvc = QAO_Create<spe::DefaultSyncedVarmapService>(context->getQAORuntime().nonOwning(),
                                                              PRIORITY_VARMAPMGR);
    if (aGameMode == GameMode::Server) {
        svmSvc->setToMode(spe::SyncedVarmapService::Mode::Host);
        for (hg::PZInteger i = 0; i < aPlayerCount; i += 1) {
            svmSvc->int64SetClientWritePermission("val" + std::to_string(i), i, true);
        }
    } else {
        svmSvc->setToMode(spe::SyncedVarmapService::Mode::Client);
    }

    context->attachAndOwnComponent(std::move(svmSvc));

    // Create and attach a lobby backend service
    auto lobbySvc = QAO_Create<spe::DefaultLobbyBackendService>(context->getQAORuntime().nonOwning(),
                                                                PRIORITY_LOBBYBACKMGR);

    if (aGameMode == GameMode::Server) {
        lobbySvc->setToHostMode(aPlayerCount);
    } else {
        lobbySvc->setToClientMode(1);
    }

    context->attachAndOwnComponent(std::move(lobbySvc));

    // Create and attach a lobby frontend service
    auto lobbyFrontendSvc = QAO_Create<DefaultLobbyFrontendService>(context->getQAORuntime().nonOwning(),
                                                                    PRIORITY_LOBBYFRONTMGR);

    if (aGameMode == GameMode::Server) {
        lobbyFrontendSvc->setToHeadlessHostMode();
    } else {
        const auto nameInLobby =
            "player_" + std::to_string(hg::util::GetRandomNumber<int>(10'000, 99'999));
        const auto uniqueId = "id_" + std::to_string(hg::util::GetRandomNumber<int>(10'000, 99'999));
        lobbyFrontendSvc->setToClientMode(nameInLobby, uniqueId);
    }

    context->attachAndOwnComponent(std::move(lobbyFrontendSvc));

    // Create and attach an Auth service
    auto authSvc = QAO_Create<spe::DefaultAuthorizationService>(context->getQAORuntime().nonOwning(),
                                                                PRIORITY_AUTHMGR);

    if (aGameMode == GameMode::Server) {
        authSvc->setToHostMode();
    } else {
        authSvc->setToClientMode();
    }

    context->attachAndOwnComponent(std::move(authSvc));

    // Create and attach a Gameplay service
    auto gpSvc = QAO_Create<DefaultMainGameplayService>(context->getQAORuntime().nonOwning(),
                                                        PRIORITY_GAMEPLAYMGR);
    context->attachAndOwnComponent(std::move(gpSvc));

    // Create player "characters"
    if (aGameMode == GameMode::Server) {
        for (hg::PZInteger i = 0; i < aPlayerCount; i += 1) {
            if (i == 0)
                continue; // host doesn't need a character
            {
                auto p = QAO_Create<BasicPlayerCharacter>(context->getQAORuntime());
                p->init(i, 20.f + i * 40.f, 40.f);
            }
            {
                auto p = QAO_Create<AutodiffPlayerCharacter>(context->getQAORuntime());
                p->init(i, 20.f + i * 40.f, 80.f);
            }
            {
                auto p = QAO_Create<AlternatingPlayerCharacter>(context->getQAORuntime());
                p->init(i, 20.f + i * 40.f, 120.f);
            }
            {
                auto p = QAO_Create<AutodiffAlternatingPlayerCharacter>(context->getQAORuntime());
                p->init(i, 20.f + i * 40.f, 160.f);
            }
        }
    }

    return context;
}

/* SERVER:
 *   mmp.exe server <local-port> <player-count>
 *
 * CLIENT:
 *   mmp.exe client <local-port> <server-ip> <server-port>
 *
 */
int main(int argc, char* argv[]) {
    // Set logging severity:
    hg::log::SetMinimalLogSeverity(hg::log::Severity::All);

    // Seed pseudorandom number generators:
    hg::util::DoWith32bitRNG([](std::mt19937& aRNG) {
        aRNG.seed(hg::util::Generate32bitSeed());
    });
    hg::util::DoWith64bitRNG([](std::mt19937_64& aRNG) {
        aRNG.seed(hg::util::Generate64bitSeed());
    });

    // Initialize QAO:
    QAO_InitializeMetadata();

    // Initialize RigelNet:
    RN_IndexHandlers();

    // Parse command line arguments:
    GameMode      gameMode;
    std::uint16_t localPort   = 0;
    std::uint16_t remotePort  = 0;
    hg::PZInteger playerCount = 1;
    std::string   remoteIp    = "";

    if (argc != 4 && argc != 5) {
        std::puts("Invalid argument count");
        return EXIT_FAILURE;
    }
    const std::string gameModeStr = argv[1];
    if (gameModeStr == "server") {
        gameMode    = GameMode::Server;
        localPort   = std::stoi(argv[2]);
        playerCount = std::stoi(argv[3]) + 1;
    } else if (gameModeStr == "client") {
        gameMode   = GameMode::Client;
        localPort  = std::stoi(argv[2]);
        remoteIp   = argv[3];
        remotePort = std::stoi(argv[4]);
    } else {
        std::puts("Game mode must be either 'server' or 'client'");
        return EXIT_FAILURE;
    }

    if (!(playerCount >= 1 && playerCount < 12)) {
        std::puts("Player count must be between 1 and 12");
        return EXIT_FAILURE;
    }

    // Start the game:
    auto context = MakeGameContext(gameMode, localPort, remotePort, std::move(remoteIp), playerCount);
    const int status = context->runFor(-1);
    HG_LOG_INFO(LOG_ID, "Program exiting with status code: {}.", status);
    return status;
}

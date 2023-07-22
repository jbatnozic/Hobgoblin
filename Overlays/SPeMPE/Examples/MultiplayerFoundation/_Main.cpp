
#include "Engine.h"
#include "Lobby_frontend_manager.hpp"
#include "Main_gameplay_manager.hpp"
#include "Player_character.hpp"
#include "Player_character_alternating.hpp"

#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>
#include <Hobgoblin/Utility/State_scheduler.hpp>

#include <cstdint>
#include <iostream>

static constexpr auto LOG_ID = "MultiplayerFoundation";

///////////////////////////////////////////////////////////////////////////
// GAME CONFIG                                                           //
///////////////////////////////////////////////////////////////////////////

#define WINDOW_WIDTH          1200
#define WINDOW_HEIGHT          800
#define FRAMERATE               60

bool MyRetransmitPredicate(hg::PZInteger aCyclesSinceLastTransmit,
                           std::chrono::microseconds aTimeSinceLastSend,
                           std::chrono::microseconds aCurrentLatency) {
    // Default behaviour:
    return RN_DefaultRetransmitPredicate(aCyclesSinceLastTransmit,
                                         aTimeSinceLastSend,
                                         aCurrentLatency);
    // Aggressive retransmission:
    // return 1;
}

enum class GameMode {
    Server, Client
};

std::unique_ptr<spe::GameContext> MakeGameContext(GameMode aGameMode,
                                                  std::uint16_t aLocalPort,
                                                  std::uint16_t aRemotePort,
                                                  std::string aRemoteIp,
                                                  hg::PZInteger aPlayerCount)
{
    auto context = std::make_unique<spe::GameContext>(
        spe::GameContext::RuntimeConfig{std::chrono::duration<double>(1.0 / FRAMERATE)});
    context->setToMode((aGameMode == GameMode::Server) ? spe::GameContext::Mode::Server
                       : spe::GameContext::Mode::Client);

    // Create and attach a Window manager
    auto winMgr = std::make_unique<spe::DefaultWindowManager>(context->getQAORuntime().nonOwning(),
                                                              PRIORITY_WINDOWMGR);
    if (aGameMode == GameMode::Server) {
        winMgr->setToHeadlessMode(spe::WindowManagerInterface::TimingConfig{FRAMERATE});
    }
    else {
        winMgr->setToNormalMode(
            spe::WindowManagerInterface::WindowConfig{
                sf::VideoMode{WINDOW_WIDTH, WINDOW_HEIGHT},
                "SPeMPE Multiplayer Foundation",
                sf::Style::Default
            },
            spe::WindowManagerInterface::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
            spe::WindowManagerInterface::TimingConfig{
                FRAMERATE,
                false,                                           /* Framerate limiter */
                (aGameMode == GameMode::Server) ? false : true , /* V-Sync */
                (aGameMode == GameMode::Server) ? true : true    /* Precise timing */
            }
        );

        struct FontFace {
            Rml::String filename;
            bool fallback_face;
        };
        FontFace font_faces[] = {
            { "LatoLatin-Regular.ttf",    false },
            { "LatoLatin-Italic.ttf",     false },
            { "LatoLatin-Bold.ttf",       false },
            { "LatoLatin-BoldItalic.ttf", false },
        };
        for (const FontFace& face : font_faces) {
            Rml::LoadFontFace("assets/fonts/" + face.filename, face.fallback_face);
        }

        Rml::Debugger::Initialise(&(winMgr->getGUIContext()));
        Rml::Debugger::SetVisible(true);
    }

    context->attachAndOwnComponent(std::move(winMgr));

    // Create and attach a Networking manager
    auto netMgr = std::make_unique<spe::DefaultNetworkingManager>(context->getQAORuntime().nonOwning(),
                                                                  PRIORITY_NETWORKMGR,
                                                                  STATE_BUFFERING_LENGTH);
    if (aGameMode == GameMode::Server) {
        netMgr->setToServerMode(
            RN_Protocol::UDP,
            "minimal-multiplayer",
            aPlayerCount - 1, // -1 because player 0 is the host itself (even if it doesn't participate in the game)
            1024,
            RN_NetworkingStack::Default
        );
        netMgr->setPacemakerPulsePeriod(120);
        auto& server = netMgr->getServer();
        server.setTimeoutLimit(std::chrono::seconds{5});
        server.setRetransmitPredicate(&MyRetransmitPredicate);
        server.start(aLocalPort);

        std::printf("Server started on port %d for up to %d clients.\n", (int)server.getLocalPort(), aPlayerCount - 1);
    }
    else {
        netMgr->setToClientMode(
            RN_Protocol::UDP,
            "minimal-multiplayer",
            1024,
            RN_NetworkingStack::Default
        );
        auto& client = netMgr->getClient();
        client.setTimeoutLimit(std::chrono::seconds{5});
        client.setRetransmitPredicate(&MyRetransmitPredicate);
        client.connect(aLocalPort, aRemoteIp, aRemotePort);

        std::printf("Client started on port %d (connecting to %s:%d)\n",
                    (int)client.getLocalPort(), aRemoteIp.c_str(), (int)aRemotePort);
    }
    netMgr->setTelemetryCycleLimit(120);
    context->attachAndOwnComponent(std::move(netMgr));

    // Create and attack an Input sync manager
    auto insMgr = std::make_unique<spe::DefaultInputSyncManager>(context->getQAORuntime().nonOwning(),
                                                                 PRIORITY_INPUTMGR);

    if (aGameMode == GameMode::Server) {
        insMgr->setToHostMode(aPlayerCount - 1, STATE_BUFFERING_LENGTH);
    }
    else {
        insMgr->setToClientMode();
    }

    /* Either way, define the inputs in the same way */
    {
        spe::InputSyncManagerWrapper wrapper{*insMgr};
        wrapper.defineSignal<bool>("left", false);
        wrapper.defineSignal<bool>("right", false);
        wrapper.defineSignal<bool>("up", false);
        wrapper.defineSignal<bool>("down", false);
        wrapper.defineSimpleEvent("jump");
    }

    context->attachAndOwnComponent(std::move(insMgr));

    // Create and attach a varmap manager
    auto svmMgr = std::make_unique<spe::DefaultSyncedVarmapManager>(context->getQAORuntime().nonOwning(),
                                                                    PRIORITY_VARMAPMGR);
    if (aGameMode == GameMode::Server) {
        svmMgr->setToMode(spe::SyncedVarmapManagerInterface::Mode::Host);
        for (hg::PZInteger i = 0; i < aPlayerCount; i += 1) {
            svmMgr->int64SetClientWritePermission("val" + std::to_string(i), i, true);
        }
    }
    else {
        svmMgr->setToMode(spe::SyncedVarmapManagerInterface::Mode::Client);
    }

    context->attachAndOwnComponent(std::move(svmMgr));

    // Create and attach a lobby backend manager
    auto lobbyMgr = std::make_unique<spe::DefaultLobbyBackendManager>(context->getQAORuntime().nonOwning(),
                                                                      PRIORITY_LOBBYBACKMGR);

    if (aGameMode == GameMode::Server) {
        lobbyMgr->setToHostMode(aPlayerCount);
    }
    else {
        lobbyMgr->setToClientMode(1);
    }

    context->attachAndOwnComponent(std::move(lobbyMgr));

    // Create and attach a lobby frontend manager
    auto lobbyFrontendMgr = std::make_unique<LobbyFrontendManager>(context->getQAORuntime().nonOwning(),
                                                                   PRIORITY_LOBBYFRONTMGR);

    if (aGameMode == GameMode::Server) {
        lobbyFrontendMgr->setToHeadlessHostMode();
    }
    else {
        const auto nameInLobby = "player_" + std::to_string(hg::util::GetRandomNumber<int>(10'000, 99'999));
        const auto uniqueId    =     "id_" + std::to_string(hg::util::GetRandomNumber<int>(10'000, 99'999));
        lobbyFrontendMgr->setToClientMode(nameInLobby, uniqueId);
    }

    context->attachAndOwnComponent(std::move(lobbyFrontendMgr));

    // Create and attach an Auth manager
    auto authMgr = std::make_unique<spe::DefaultAuthorizationManager>(context->getQAORuntime().nonOwning(),
                                                                      PRIORITY_AUTHMGR);

    if (aGameMode == GameMode::Server) {
        authMgr->setToHostMode();
    }
    else {
        authMgr->setToClientMode();
    }

    context->attachAndOwnComponent(std::move(authMgr));

    // Create and attach a Gameplay manager
    auto gpMgr = std::make_unique<MainGameplayManager>(context->getQAORuntime().nonOwning(),
                                                       PRIORITY_GAMEPLAYMGR);
    context->attachAndOwnComponent(std::move(gpMgr));

    // Create player "characters"
    if (aGameMode == GameMode::Server) {
        for (hg::PZInteger i = 0; i < aPlayerCount; i += 1) {
            if (i == 0) continue; // host doesn't need a character
            auto* p = QAO_PCreate<PlayerCharacter>(context->getQAORuntime(),
                                                   context->getComponent<MNetworking>().getRegistryId(),
                                                   spe::SYNC_ID_NEW);
            p->init(i, 20.f + i * 40.f, 40.f);
            auto* pa = QAO_PCreate<PlayerCharacterAlt>(context->getQAORuntime(),
                                                       context->getComponent<MNetworking>().getRegistryId(),
                                                       spe::SYNC_ID_NEW);
            pa->init(i, 20.f + i * 40.f, 80.f);
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
    
    // Initialize RigelNet:
    RN_IndexHandlers();

    // Parse command line arguments:
    GameMode gameMode;
    std::uint16_t localPort = 0;
    std::uint16_t remotePort = 0;
    hg::PZInteger playerCount = 1;
    std::string remoteIp = "";

    if (argc != 4 && argc != 5) {
        std::puts("Invalid argument count");
        return EXIT_FAILURE;
    }
    const std::string gameModeStr = argv[1];
    if (gameModeStr == "server") {
        gameMode = GameMode::Server;
        localPort = std::stoi(argv[2]);
        playerCount = std::stoi(argv[3]) + 1;
    }
    else if (gameModeStr == "client") {
        gameMode = GameMode::Client;
        localPort = std::stoi(argv[2]);
        remoteIp = argv[3];
        remotePort = std::stoi(argv[4]);
    }
    else {
        std::puts("Game mode must be either 'server' or 'client'");
        return EXIT_FAILURE;
    }

    if (!(playerCount >= 1 && playerCount < 12)) {
        std::puts("Player count must be between 1 and 12");
        return EXIT_FAILURE;
    }

    // Start the game:
    auto context = MakeGameContext(gameMode, localPort, remotePort,
                                   std::move(remoteIp), playerCount);
    const int status = context->runFor(-1);
    HG_LOG_INFO(LOG_ID, "Program exiting with status code: {}.", status);
    return status;
}

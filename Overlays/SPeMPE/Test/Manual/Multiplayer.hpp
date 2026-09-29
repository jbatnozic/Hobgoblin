// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include "Config.hpp"
#include "Engine.hpp"
#include "Main_gameplay_service_default.hpp"

#include <SPeMPE/Services/Networking_service_default.hpp>

#include <chrono>
#include <memory>

namespace multiplayer {

inline std::unique_ptr<spe::GameContext> CreateHostGameContext() {
    auto context =
        std::make_unique<spe::GameContext>(spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::GameMaster);

    // Create and attach a Multiplayer service
    auto netSvc = QAO_Create<spe::DefaultNetworkingService>(context->getQAORuntime().nonOwning(),
                                                            PRIORITY_NETWORKMGR,
                                                            0);
    netSvc->setToServerMode(hg::RN_Protocol::UDP, "pass", 3, 4096, hg::RN_NetworkingStack::Default);
    netSvc->setStateBufferingLength(STATE_BUFFERING_LENGTH);
    netSvc->getServer().setTimeoutLimit(std::chrono::seconds{5});
    netSvc->getServer().start(0);

    context->attachAndOwnComponent(std::move(netSvc));

    // Create and attach a Window service
    auto winSvc =
        QAO_Create<spe::DefaultWindowService>(context->getQAORuntime().nonOwning(), PRIORITY_WINDOWMGR);
    // clang-format off
    winSvc->setToNormalMode(
        hg::uwga::CreateGraphicsSystem("SFML"),
        spe::WindowService::WindowConfig{
            .size  = {WINDOW_WIDTH, WINDOW_HEIGHT},
            .title = "SPeMPE Manual Test (Multiplayer - Host)",
            .style = hg::uwga::WindowStyle::DEFAULT
    },
        spe::WindowService::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
        spe::WindowService::TimingConfig{spe::FrameRate{FRAME_RATE},
                                                  spe::PREVENT_BUSY_WAIT_ON,
                                                  spe::VSYNC_OFF});
    // clang-format on

    context->attachAndOwnComponent(std::move(winSvc));

    // Create and attach a Main gameplay service
    auto mainGameplaySvc = QAO_Create<DefaultMainGameplayService>(context->getQAORuntime().nonOwning());

    context->attachAndOwnComponent(std::move(mainGameplaySvc));

    return context;
}

inline std::unique_ptr<spe::GameContext> CreateClientGameContext(std::uint16_t aServerPort) {
    auto context =
        std::make_unique<spe::GameContext>(spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::Client);

    // Create and attach a Multiplayer service
    auto netSvc = QAO_Create<spe::DefaultNetworkingService>(context->getQAORuntime().nonOwning(),
                                                            PRIORITY_NETWORKMGR,
                                                            0);
    netSvc->setToClientMode(hg::RN_Protocol::UDP, "pass", 4096, hg::RN_NetworkingStack::Default);
    netSvc->setStateBufferingLength(STATE_BUFFERING_LENGTH);
    netSvc->getClient().setTimeoutLimit(std::chrono::seconds{5});
    netSvc->getClient().connect(0, "127.0.0.1", aServerPort);

    context->attachAndOwnComponent(std::move(netSvc));

    // Create and attach a Window service
    auto winSvc =
        QAO_Create<spe::DefaultWindowService>(context->getQAORuntime().nonOwning(), PRIORITY_WINDOWMGR);
    // clang-format off
    winSvc->setToNormalMode(
        hg::uwga::CreateGraphicsSystem("SFML"),
        spe::WindowService::WindowConfig{
            .size  = {WINDOW_WIDTH, WINDOW_HEIGHT},
            .title = "SPeMPE Manual Test (Multiplayer - Client)",
            .style = hg::uwga::WindowStyle::DEFAULT
    },
        spe::WindowService::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
        spe::WindowService::TimingConfig{spe::FrameRate{FRAME_RATE},
                                                  spe::PREVENT_BUSY_WAIT_ON,
                                                  spe::VSYNC_OFF});
    // clang-format on
    context->attachAndOwnComponent(std::move(winSvc));

    // Create and attach a Main gameplay service
    auto mainGameplaySvc = QAO_Create<DefaultMainGameplayService>(context->getQAORuntime().nonOwning());

    context->attachAndOwnComponent(std::move(mainGameplaySvc));

    return context;
}

} // namespace multiplayer

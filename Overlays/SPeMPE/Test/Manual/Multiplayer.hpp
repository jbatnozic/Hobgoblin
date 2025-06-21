// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Config.hpp"
#include "Engine.hpp"
#include "Main_gameplay_manager.hpp"

#include <SPeMPE/Managers/Networking_manager_default.hpp>

#include <chrono>
#include <memory>

namespace multiplayer {

inline
std::unique_ptr<spe::GameContext> CreateHostGameContext() {
    auto context = std::make_unique<spe::GameContext>(
        spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::GameMaster);

    // Create and attach a Multiplayer manager
    auto netMgr = QAO_Create<spe::DefaultNetworkingManager>(
        context->getQAORuntime().nonOwning(), PRIORITY_NETWORKMGR, 0);
    netMgr->setToServerMode(
        hg::RN_Protocol::UDP, "pass", 3, 4096, hg::RN_NetworkingStack::Default);
    netMgr->setStateBufferingLength(STATE_BUFFERING_LENGTH);
    netMgr->getServer().setTimeoutLimit(std::chrono::seconds{5});
    netMgr->getServer().start(0);

    context->attachAndOwnComponent(std::move(netMgr));

    // Create and attach a Window manager
    auto winMgr = QAO_Create<spe::DefaultWindowManager>(context->getQAORuntime().nonOwning(),
                                                        PRIORITY_WINDOWMGR);
    winMgr->setToNormalMode(
        spe::WindowManagerInterface::WindowConfig{
            hg::win::VideoMode{WINDOW_WIDTH, WINDOW_HEIGHT},
            "SPeMPE Manual Test (Multiplayer - Host)",
            hg::win::WindowStyle::Default
        },
        spe::WindowManagerInterface::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
        spe::WindowManagerInterface::TimingConfig{
            spe::FrameRate{FRAME_RATE},
            spe::PREVENT_BUSY_WAIT_ON,
            spe::VSYNC_OFF
        }
    );

    context->attachAndOwnComponent(std::move(winMgr));

    // Create and attach a Main gameplay manager
    auto mainGameplayMgr = QAO_Create<MainGameplayManager>(
        context->getQAORuntime().nonOwning());

    context->attachAndOwnComponent(std::move(mainGameplayMgr));

    return context;
}

inline
std::unique_ptr<spe::GameContext> CreateClientGameContext(std::uint16_t aServerPort) {
    auto context = std::make_unique<spe::GameContext>(
        spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::Client);

    // Create and attach a Multiplayer manager
    auto netMgr = QAO_Create<spe::DefaultNetworkingManager>(
        context->getQAORuntime().nonOwning(), PRIORITY_NETWORKMGR, 0);
    netMgr->setToClientMode(
        hg::RN_Protocol::UDP, "pass", 4096, hg::RN_NetworkingStack::Default);
    netMgr->setStateBufferingLength(STATE_BUFFERING_LENGTH);
    netMgr->getClient().setTimeoutLimit(std::chrono::seconds{5});
    netMgr->getClient().connect(0, "127.0.0.1", aServerPort);

    context->attachAndOwnComponent(std::move(netMgr));

    // Create and attach a Window manager
    auto winMgr = QAO_Create<spe::DefaultWindowManager>(context->getQAORuntime().nonOwning(),
                                                              PRIORITY_WINDOWMGR);
    winMgr->setToNormalMode(
        spe::WindowManagerInterface::WindowConfig{
            hg::win::VideoMode{WINDOW_WIDTH, WINDOW_HEIGHT},
            "SPeMPE Manual Test (Multiplayer - Client)",
            hg::win::WindowStyle::Default
        },
        spe::WindowManagerInterface::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
        spe::WindowManagerInterface::TimingConfig{
            spe::FrameRate{FRAME_RATE},
            spe::PREVENT_BUSY_WAIT_ON,
            spe::VSYNC_OFF
        }
    );

    context->attachAndOwnComponent(std::move(winMgr));

    // Create and attach a Main gameplay manager
    auto mainGameplayMgr = QAO_Create<MainGameplayManager>(
        context->getQAORuntime().nonOwning());

    context->attachAndOwnComponent(std::move(mainGameplayMgr));

    return context;
}

} // namespace multiplayer

// clang-format on

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Config.hpp"
#include "Engine.hpp"
#include "Main_gameplay_service_default.hpp"

#include <Hobgoblin/UWGA.hpp>

#include <SPeMPE/Services/Networking_service_default.hpp>

#include <memory>

namespace singleplayer {

inline
std::unique_ptr<spe::GameContext> CreateGameContext() {
    auto context = std::make_unique<spe::GameContext>(
        spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::GameMaster);

    // Create and attach a Networking service
    auto netSvc = QAO_Create<spe::DefaultNetworkingService>(
        context->getQAORuntime().nonOwning(), PRIORITY_NETWORKMGR, 0);
    netSvc->setToServerMode(
        hg::RN_Protocol::UDP, "pass", 1, 1024, hg::RN_NetworkingStack::Default);

    context->attachAndOwnComponent(std::move(netSvc));

    // Create and attach a Window service
    auto winSvc = QAO_Create<spe::DefaultWindowService>(context->getQAORuntime().nonOwning(),
                                                        PRIORITY_WINDOWMGR);
    winSvc->setToNormalMode(
        hg::uwga::CreateGraphicsSystem("SFML"),
        spe::WindowService::WindowConfig{
            .size = {WINDOW_WIDTH, WINDOW_HEIGHT},
            .title = "SPeMPE Manual Test (Singleplayer)",
            .style = hg::uwga::WindowStyle::DEFAULT
        },
        spe::WindowService::MainRenderTextureConfig{{WINDOW_WIDTH, WINDOW_HEIGHT}},
        spe::WindowService::TimingConfig{
            spe::FrameRate{FRAME_RATE},
            spe::PREVENT_BUSY_WAIT_ON,
            spe::VSYNC_OFF
        }
    );

    context->attachAndOwnComponent(std::move(winSvc));

    // Create and attach a Main gameplay service
    auto mainGameplaySvc = QAO_Create<DefaultMainGameplayService>(
        context->getQAORuntime().nonOwning());

    context->attachAndOwnComponent(std::move(mainGameplaySvc));

    return context;
}

} // namespace singleplayer

// clang-format on

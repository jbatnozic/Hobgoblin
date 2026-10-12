// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <cstdint>

namespace cinnabar {

enum class GameContextMode {
    CLIENT,
    SERVER,
    DEV
};

//! Default = client
//! DEV mode uses both client and server members
struct GameContextCreationParams {
    GameContextMode mode = GameContextMode::CLIENT;

    struct ClientParams {
        bool          connectImmediately = false;
        std::uint16_t localPort          = 0;
        std::uint16_t serverPort         = 0;
        std::string   serverIp           = "127.0.0.1";
    } client;

    struct ServerParams {
        hg::PZInteger playerCount = 2;
        std::uint16_t localPort   = 0;
    } server;
};

void FillInGameContextCreationParamsFromInteractiveCLI(GameContextCreationParams& aParams);

int FillInGameContextCreationParamsFromCLIArguments(GameContextCreationParams& aParams,
                                                    int                        aArgumentCount,
                                                    const char* const*         aArgumentArray);

} // namespace cinnabar

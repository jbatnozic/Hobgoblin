// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

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

    } client;

    struct ServerParams {

    } server;
};

void FillInGameContextCreationParamsFromInteractiveCLI(GameContextCreationParams& aParams);

int FillInGameContextCreationParamsFromCLIArguments(GameContextCreationParams& aParams,
                                                    int                        aArgumentCount,
                                                    const char* const*         aArgumentArray);

} // namespace cinnabar

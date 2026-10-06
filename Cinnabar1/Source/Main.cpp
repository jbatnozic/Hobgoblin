// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Engine.hpp"
#include <CBR/GameContext/Game_context_creation_params.hpp>
#include <CBR/GameContext/Game_context_factory.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <cstring>

namespace cinnabar {

int Main(int aArgumentCount, char** aArgumentArray) try {
    // Initialization that absolutely must run before anything else
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);
    RN_IndexHandlers();

    // Getting the config from the command line
    GameContextCreationParams gccParams;
    if (aArgumentCount == 2 && (strcmp(aArgumentArray[1], "--climm") == 0)) {
        FillInGameContextCreationParamsFromInteractiveCLI(gccParams);
    } else if (aArgumentCount <= 1) {
        // Leave default params
    } else {
        const int parsingResult =
            FillInGameContextCreationParamsFromCLIArguments(gccParams, aArgumentCount, aArgumentArray);
        if (parsingResult != EXIT_SUCCESS) {
            return parsingResult;
        }
    }

    const auto ctx    = CreateGameContext(GameContextMode::DEV);
    const auto status = ctx->runFor(-1);
    HG_LOG_INFO(LOG_ID, "Program exiting (status code {}).", status);
    return status;
} catch (const jbatnozic::hobgoblin::TracedException& aEx) {
    HG_LOG_FATAL(LOG_ID, "Uncaught traced exception: {}", aEx.getFullFormattedDescription());
    return EXIT_FAILURE;
} catch (const std::exception& aEx) {
    HG_LOG_FATAL(LOG_ID, "Uncaught std::exception: {}", aEx.what());
    return EXIT_FAILURE;
}

} // namespace cinnabar

int main(int argc, char** argv) {
    return cinnabar::Main(argc, argv);
} 

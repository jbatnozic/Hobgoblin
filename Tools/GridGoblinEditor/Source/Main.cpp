// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>

#include "Context_factory.hpp"

namespace hg = ::jbatnozic::hobgoblin;

constexpr auto LOG_ID = "GridGoblin.Editor";

constexpr auto RUN_WITHOUT_ITERATION_LIMIT = -1;

int main(int argc, char* argv[]) try {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);

    using namespace jbatnozic::gridgoblin::editor;

    if (argc != 5) {
        HG_LOG_INFO(LOG_ID, "Usage: ./ggedit <assets-dir> <definitions-dir> <sprites-dir> <world-cache-dir>");
        return EXIT_SUCCESS;
    }

    EditorConfig config;
    config.assetsDir      = argv[1];
    config.definitionsDir = argv[2];
    config.spritesDir     = argv[3];
    config.worldCacheDir  = argv[4];

    auto ctx = CreateEditorSPeMPEContext(config);
    return ctx->runFor(RUN_WITHOUT_ITERATION_LIMIT);
} catch (const hg::TracedException& ex) {
    HG_LOG_FATAL(LOG_ID, "{}", ex.getFormattedDescription());
    return EXIT_FAILURE;
} catch (const std::exception& ex) {
    HG_LOG_FATAL(LOG_ID, "Standard exception thrown: {}", ex.what());
    return EXIT_FAILURE;
} catch (...) {
    HG_LOG_FATAL(LOG_ID, "Non-standard exception thrown, details not available.");
    return EXIT_FAILURE;
}

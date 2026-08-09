// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Engine.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <Game_context_factory.hpp>

namespace cinnabar {

int MyMain() try {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);
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

int main(int argc, char* argv[]) {
    return cinnabar::MyMain();
} 

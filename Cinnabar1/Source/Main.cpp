// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Engine.hpp>

#include <Game_context_factory.hpp>

namespace cinnabar {

int MyMain() {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);
    const auto ctx    = CreateGameContext(GameContextMode::DEV);
    const auto status = ctx->runFor(-1);
    HG_LOG_INFO(LOG_ID, "Program exiting (status code {}).", status);
    return status;
}

} // namespace cinnabar

int main(int argc, char* argv[]) {
    return cinnabar::MyMain();
}

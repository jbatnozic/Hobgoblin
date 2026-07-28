// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Game_context_factory.hpp>

namespace cinnabar {

int MyMain() {
    auto ctx = CreateGameContext(GameContextMode::DEV);
    return ctx->runFor(-1);
}

} // namespace cinnabar

int main(int argc, char* argv[]) {
    return cinnabar::MyMain();
}

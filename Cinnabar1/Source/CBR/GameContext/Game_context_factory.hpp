// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Engine.hpp>

#include <memory>

namespace cinnabar {

enum class GameContextMode {
    SERVER,
    CLIENT,
    DEV
};

std::unique_ptr<spe::GameContext> CreateGameContext(GameContextMode aMode);

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/GameContext/Game_context_creation_params.hpp>
#include <Engine.hpp>

#include <memory>

namespace cinnabar {

std::unique_ptr<spe::GameContext> CreateGameContext(GameContextMode aMode);

} // namespace cinnabar

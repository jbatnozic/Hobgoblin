// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Engine.hpp"

namespace singleplayer {

class MainGameplayService : public spe::ContextComponent {
private:
    SPEMPE_CTXCOMP_TAG("SPMainGameplayService");
};

} // namespace singleplayer

namespace multiplayer {

class MainGameplayService : public spe::ContextComponent {
private:
    SPEMPE_CTXCOMP_TAG("MPMainGameplayService");
};

} // namespace multiplayer

// clang-format on

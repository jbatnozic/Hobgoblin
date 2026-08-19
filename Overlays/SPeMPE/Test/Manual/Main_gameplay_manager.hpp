// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Engine.hpp"

namespace singleplayer {

class MainGameplayManager : public spe::ContextComponent {
private:
    SPEMPE_CTXCOMP_TAG("SPMainGameplayManager");
};

} // namespace singleplayer

namespace multiplayer {

class MainGameplayManager : public spe::ContextComponent {
private:
    SPEMPE_CTXCOMP_TAG("MPMainGameplayManager");
};

} // namespace multiplayer

// clang-format on

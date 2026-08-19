// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#pragma once

#include "Engine.h"

//! TODO(add description)
class MainGameplayManager : public spe::ContextComponent {
private:
    SPEMPE_CTXCOMP_TAG("MainGameplayManager");
};

using MMainGameplay = MainGameplayManager;

// clang-format on

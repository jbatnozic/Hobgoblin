// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

namespace cinnabar {

class MainGameFlowManagerInterface : public spe::ContextComponent {
public:
    SPEMPE_CTXCOMP_TAG("cinnabar::MainGameFlowManager");
};

} // namespace cinnabar

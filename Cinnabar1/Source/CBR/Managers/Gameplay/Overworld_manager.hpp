// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Alvin/Space.hpp>

namespace cinnabar {

class OverworldManager : public spe::ContextComponent {
public:
    virtual hg::alvin::Space& getAlvinSpace() = 0;

    virtual const hg::alvin::Space& getAlvinSpace() const = 0;

private:
    SPEMPE_CTXCOMP_TAG("cinnabar::OverworldManager");
};

using MOverworld = OverworldManager;

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Alvin/Space.hpp>

namespace cinnabar {

class OverworldManagerInterface : public spe::ContextComponent {
public:
    SPEMPE_CTXCOMP_TAG("cinnabar::OverworldManager");

    virtual hg::alvin::Space& getAlvinSpace() = 0;

    virtual const hg::alvin::Space& getAlvinSpace() const = 0;
};

using MOverworld = OverworldManagerInterface;

} // namespace cinnabar

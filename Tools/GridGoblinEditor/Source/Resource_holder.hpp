// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Graphics.hpp>
#include <SPeMPE/SPeMPE.hpp>

#include "Definitions.hpp"

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace hg  = ::jbatnozic::hobgoblin;
namespace spe = ::jbatnozic::spempe;

class ResourceHolder : public spe::ContextComponent {
public:

    virtual const hg::gr::SpriteLoader& getSpriteLoader() const = 0;

    virtual const AllDefinitions& getAllDefinitions() const = 0;

private:
    SPEMPE_CTXCOMP_TAG("jbatnozic::gridgoblin::editor::ResourceHolder");
};

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
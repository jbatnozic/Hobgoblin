// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/GridGoblin.hpp>
#include <SPeMPE/SPeMPE.hpp>

#include "Context_factory.hpp"

#include <optional>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace hg  = ::jbatnozic::hobgoblin;
namespace spe = ::jbatnozic::spempe;

class EditorWorldDriver : public spe::NonstateObject {
public:
    EditorWorldDriver(hg::QAO_RuntimeRef aRuntimeRef, int aExecutionPriority);

    void init(const EditorConfig& aConfig);

    World& getWorld();

private:
    std::optional<World> _world;
    std::optional<TopDownRenderer> _worldRenderer;

    void _eventDraw1() override;
};

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Editor_world_driver.hpp"

#include <Hobgoblin/HGExcept.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

EditorWorldDriver::EditorWorldDriver(hg::QAO_RuntimeRef aRuntimeRef, int aExecutionPriority)
    : spe::NonstateObject{aRuntimeRef, SPEMPE_TYPEID_SELF, aExecutionPriority, "EditorWorldDriver"} {}

void EditorWorldDriver::init(const EditorConfig& aConfig) {}

World& EditorWorldDriver::getWorld() {
    HG_VALIDATE_PRECONDITION(_world.has_value());
    return *_world;
}

void EditorWorldDriver::_eventDraw1() {
    if (_world.has_value() && _worldRenderer.has_value()) {
        auto& winMgr = ccomp<spe::WindowManagerInterface>();
        auto& view   = winMgr.getView();
        _worldRenderer->startPrepareToRender(view, {}, PositionInWorld{view.getCenter()}, 0, nullptr);
        _worldRenderer->endPrepareToRender();
        _worldRenderer->render(winMgr.getCanvas());
    }
}

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic

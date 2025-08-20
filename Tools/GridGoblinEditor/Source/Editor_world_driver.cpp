// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Editor_world_driver.hpp"

#include "Editor_ui_driver.hpp"
#include "Resource_holder.hpp"

#include <Hobgoblin/HGExcept.hpp>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

EditorWorldDriver::EditorWorldDriver(hg::QAO_RuntimeRef aRuntimeRef, int aExecutionPriority)
    : spe::NonstateObject{aRuntimeRef, SPEMPE_TYPEID_SELF, aExecutionPriority, "EditorWorldDriver"} {}

void EditorWorldDriver::init(const EditorConfig& aConfig) {
    const auto& resources = ccomp<ResourceHolder>();

    const auto& worldDefinition = resources.getAllDefinitions().worldDefinition;

    const WorldConfig worldConfig{.chunkCountX                 = worldDefinition.chunkCountX,
                                  .chunkCountY                 = worldDefinition.chunkCountY,
                                  .cellsPerChunkX              = worldDefinition.cellsPerChunkX,
                                  .cellsPerChunkY              = worldDefinition.cellsPerChunkY,
                                  .cellResolution              = worldDefinition.cellResolution,
                                  .wallHeight                  = 0.f,
                                  .maxCellOpenness             = 0,
                                  .maxLoadedNonessentialChunks = 64,
                                  .chunkDirectoryPath          = aConfig.worldCacheDir};

    const TopDownRendererConfig rendererConfig{};

    _world.emplace(worldConfig);
    _worldRenderer.emplace(*_world, resources.getSpriteLoader(), rendererConfig);
}

World& EditorWorldDriver::getWorld() {
    HG_VALIDATE_PRECONDITION(_world.has_value());
    return *_world;
}

void EditorWorldDriver::_eventUpdate1() {
    if (_world.has_value() && _worldRenderer.has_value()) {
        auto& winMgr = ccomp<spe::WindowManagerInterface>();
        auto  input  = winMgr.getInput();

        if (input.checkPressed(hg::in::MB_LEFT, spe::WindowFrameInputView::Mode::Edge)) {
            auto* uiDriver = static_cast<EditorUiDriver*>(getRuntime()->find("EditorUiDriver"));
            HG_HARD_ASSERT(uiDriver != nullptr);

            const auto mousePos = input.getViewRelativeMousePos();
            // TODO: check that pos is in bounds

            const auto sel = uiDriver->getSelectedCellModel();

            _world->edit(*_world->getPermissionToEdit(), [this, &mousePos, &sel](World::Editor& aEditor) {
                const auto cellId = _world->posToCellUnchecked(mousePos);
                aEditor.setFloorAtUnchecked(cellId, sel.getFloorOpt());
                aEditor.setWallAtUnchecked(cellId, sel.getWallOpt());
                aEditor.setUserDataAtUnchecked(cellId, sel.getUserData());
            });
        }
    }
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

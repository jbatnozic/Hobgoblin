// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Positional/Position_in_view.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Rendered_object.hpp>
#include <GridGoblin/Rendering/Renderer.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/Private/Reduction_predicates.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/UWGA/Sprite_loader.hpp>

#include <vector>

namespace jbatnozic {
namespace gridgoblin {

class DimetricRenderer : public Renderer {
public:
    DimetricRenderer(const hg::uwga::SpriteLoader& aSpriteLoader);

    void reset(RenderContext& aRenderCtx) override;

    void prepareToRender(RenderContext& aRenderCtx) override;

    void render(
        const RenderContext&          aRenderCtx,
        hg::uwga::Canvas&             aCanvas,
        const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) const override;

private:
    // ===== Dependencies =====

    const hg::uwga::SpriteLoader& _spriteLoader;

    RenderContext* _renderCtx = nullptr;

    // ===== Cycle counter =====

    std::int64_t _renderCycleCounter = 0;

    // ===== View data =====

    PositionInWorld _viewTopLeft;

    // ===== Cell info =====

    struct CellGraphicsInfo {
        cell::FloorSprite     floorSprite;
        cell::WallSprite      wallSprite;
        cell::RendererAuxData rendererAuxData;
    };

    struct CellInfo : CellGraphicsInfo {
        cell::SpatialInfo spatialInfo;
        hg::PZInteger     gridX;
        hg::PZInteger     gridY;
    };

    // ===== Cell adapters =====

    class CellToRenderedObjectAdapter : public RenderedObject {
    public:
        CellToRenderedObjectAdapter(DimetricRenderer& aRenderer,
                                    CellGraphicsInfo  aCellGraphicsInfo,
                                    const BoundsInfo& aBoundsInfo);

        void render(hg::uwga::Canvas& aCanvas, PositionInView aPosInView) const override;

    private:
        DimetricRenderer& _renderer;
        CellGraphicsInfo  _cellGraphicsInfo;

        // TODO: Render parameters: color etc.
    };

    friend class CellToRenderedObjectAdapter;

    std::vector<CellToRenderedObjectAdapter> _cellAdapters;

    // ===== Sprite cache =====

    mutable std::unordered_map<SpriteId, hg::uwga::Sprite> _spriteCache;

    // ===== Methods =====

    hg::uwga::Sprite& _getSprite(SpriteId aSpriteId) const;

    template <class taCallable>
    void _diagonalTraverse(const World&       aWorld,
                           PositionInWorld    aViewTopLeft,
                           hg::math::Vector2d aViewSize,
                           taCallable&&       aFunc);

    void _reduceCellsBelowIfCellIsVisible(hg::math::Vector2pz       aCell,
                                          PositionInView            aCellPosInView,
                                          const VisibilityProvider& aVisProv);

    void _prepareCells();

    //! bits 0..9   Reduction counter
    //! bit  10     Render cycle flag
    //! bit  11     Cell touched
    //! bit  12     Should reduce
    //! bits 13..31 -- Unused --
    using AuxDataBits = std::uint32_t;

    void _updateRendererAuxDataOfCell(CellInfo& aCellInfo, World::Editor aWorldEditor);

    void _updateFadeValueOfCellRendererMask(CellInfo&                   aCellInfo,
                                            detail::RecommendedDrawMode aDrawMode,
                                            World::Editor               aWorldEditor);
};

} // namespace gridgoblin
} // namespace jbatnozic

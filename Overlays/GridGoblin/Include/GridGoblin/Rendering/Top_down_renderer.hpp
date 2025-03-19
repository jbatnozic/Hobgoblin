// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Rendering/Rendered_object.hpp>
#include <GridGoblin/Rendering/Renderer.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/Spatial/Position_in_view.hpp>
#include <GridGoblin/Spatial/Position_in_world.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/Graphics.hpp>

#include <vector>

namespace jbatnozic {
namespace gridgoblin {

struct TopDownRendererConfig {
    // Nothing for now...
};

//! Renders the world in a simple 2D top-down perspective (but it can be also used for sidescrollers).
//! \note Doesn't ever reduce walls.
class TopDownRenderer : public Renderer {
public:
    TopDownRenderer(const World&                 aWorld,
                    const hg::gr::SpriteLoader&  aSpriteLoader,
                    const TopDownRendererConfig& aConfig = {});

    void startPrepareToRender(const hg::gr::View&       aView,
                              const OverdrawAmounts&    aOverdrawAmounts,
                              PositionInWorld           aPointOfView,
                              std::int32_t              aRenderFlags,
                              const VisibilityProvider* aVisProv) override;

    void addObject(const RenderedObject& aObject) override;

    void endPrepareToRender() override;

    void render(hg::gr::Canvas& aCanvas) override;

private:
    // ===== Dependencies =====

    const World&                _world;
    const hg::gr::SpriteLoader& _spriteLoader;

    // ===== View data =====

    struct ViewData {
        PositionInView     center;
        hg::math::Vector2f size;
        OverdrawAmounts    overdraw;
    };

    ViewData _viewData;

    // ===== Cell adapters =====

    class CellToRenderedObjectAdapter : public RenderedObject {
    public:
        CellToRenderedObjectAdapter(TopDownRenderer&   aRenderer,
                                    const CellModel&   aCell,
                                    const SpatialInfo& aSpatialInfo);

        void render(hg::gr::Canvas& aCanvas, PositionInView aPosInView) const override;

    private:
        TopDownRenderer& _renderer;
        const CellModel& _cell;
    };

    friend class CellToRenderedObjectAdapter;

    std::vector<CellToRenderedObjectAdapter> _cellAdapters;

    // ===== Rendered objects =====

    std::vector<const RenderedObject*> _objectsToRender;

    // ===== Sprite cache =====

    mutable std::unordered_map<SpriteId, hg::gr::Sprite> _spriteCache;

    // ===== Methods =====

    hg::gr::Sprite& _getSprite(SpriteId aSpriteId) const;

    void _prepareCells(std::int32_t aRenderFlags, const VisibilityProvider* aVisProv);
};

} // namespace gridgoblin
} // namespace jbatnozic

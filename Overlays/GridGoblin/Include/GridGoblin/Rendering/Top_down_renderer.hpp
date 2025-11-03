// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Positional/Position_in_view.hpp>
#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Rendered_object.hpp>
#include <GridGoblin/Rendering/Renderer.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Sprite.hpp>
#include <Hobgoblin/UWGA/Sprite_loader.hpp>

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
    TopDownRenderer(const World&                  aWorld,
                    const hg::uwga::SpriteLoader& aSpriteLoader,
                    const TopDownRendererConfig&  aConfig = {});

    void startPrepareToRender(const RenderParameters&   aRenderParams,
                              const VisibilityProvider* aVisProv = nullptr) override;

    void addObject(const RenderedObject& aObject) override;

    void endPrepareToRender() override;

    void render(hg::uwga::Canvas&             aCanvas,
                const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) override;

private:
    // ===== Dependencies =====

    const World&                  _world;
    const hg::uwga::SpriteLoader& _spriteLoader;

    // ===== Render parameters =====

    RenderParameters _renderParams;

    // ===== Cell adapters =====

    class CellToRenderedObjectAdapter : public RenderedObject {
    public:
        CellToRenderedObjectAdapter(TopDownRenderer&  aRenderer,
                                    const BoundsInfo& aBoundsInfo,
                                    cell::FloorSprite aFloorSprite);

        CellToRenderedObjectAdapter(TopDownRenderer&  aRenderer,
                                    const BoundsInfo& aBoundsInfo,
                                    cell::WallSprite  aWallSprite);

        void render(hg::uwga::Canvas& aCanvas, PositionInView aPosInView) const override;

    private:
        TopDownRenderer& _renderer;
        union {
            cell::FloorSprite _floorSprite;
            cell::WallSprite  _wallSprite;
        };
    };

    friend class CellToRenderedObjectAdapter;

    std::vector<CellToRenderedObjectAdapter> _cellAdapters;

    // ===== Rendered objects =====

    std::vector<const RenderedObject*> _objectsToRender;

    // ===== Sprite cache =====

    mutable std::unordered_map<SpriteId, hg::uwga::Sprite> _spriteCache;

    // ===== Methods =====

    hg::uwga::Sprite& _getSprite(SpriteId aSpriteId) const;

    void _prepareCells(const VisibilityProvider* aVisProv);
};

} // namespace gridgoblin
} // namespace jbatnozic

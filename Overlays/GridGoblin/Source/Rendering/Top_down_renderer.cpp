// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Positional/Position_conversions.hpp>
#include <GridGoblin/Rendering/Drawing_order.hpp>
#include <GridGoblin/Rendering/Top_down_renderer.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>

#include <algorithm>

namespace jbatnozic {
namespace gridgoblin {

// MARK: Public

TopDownRenderer::TopDownRenderer(const hg::uwga::SpriteLoader& aSpriteLoader)
    : _spriteLoader{aSpriteLoader} {}

void TopDownRenderer::reset(RenderContext& aRenderCtx) {
    _cellAdapters.clear();

    _prepareCells(aRenderCtx);
}

void TopDownRenderer::prepareToRender(RenderContext& aRenderCtx) {
    auto& objs = aRenderCtx.ephemeral.renderedObjects;
    std::sort(objs.begin(),
              objs.end(),
              [](const RenderedObject* aLhs, const RenderedObject* aRhs) -> bool {
                  // Implements: Does `aLhs` come before `aRhs`?

                  HG_ASSERT(aLhs != nullptr && aRhs != nullptr);

                  return (aLhs->getBoundsInfo().getLayer() < aRhs->getBoundsInfo().getLayer());
              });
}

void TopDownRenderer::render(const RenderContext&          aRenderCtx,
                             hg::uwga::Canvas&             aCanvas,
                             const hg::uwga::RenderStates& aRenderStates) const //
{
    (void)aRenderStates; // TODO: RenderStates unused
    const auto& objs = aRenderCtx.ephemeral.renderedObjects;
    for (const auto& object : objs) {
        const auto& boundsInfo = object->getBoundsInfo();
        const auto  posInView  = topdown::ToPositionInView(boundsInfo.getCenter());
        object->render(aCanvas, posInView);
    }
}

// MARK: Private

hg::uwga::Sprite& TopDownRenderer::_getSprite(SpriteId aSpriteId) const {
    aSpriteId = ((aSpriteId & SPRITEID_IDENTIFIER_MASK) | SPRITEID_PROJECTION_TOPDOWN);

    const auto iter = _spriteCache.find(aSpriteId);
    if (iter != _spriteCache.end()) {
        return iter->second;
    }

    const auto blueprint = _spriteLoader.getBlueprint(aSpriteId);
    const auto newIter   = _spriteCache.emplace(std::make_pair(aSpriteId, blueprint.spr()));
    return newIter.first->second;
}

void TopDownRenderer::_prepareCells(RenderContext& aRenderCtx) {
    HG_HARD_ASSERT(aRenderCtx.impls.renderer == this);
    HG_HARD_ASSERT(aRenderCtx.world != nullptr);

    const auto& viewCenter = aRenderCtx.dynamic.viewCenter;
    const auto& viewSize   = aRenderCtx.dynamic.viewSize;
    const auto& world      = *aRenderCtx.world;
    const auto  cr         = world.getCellResolution();

    const auto topLeft = topdown::ToPositionInWorld(
        PositionInView{viewCenter->x - (viewSize.x * 0.5), viewCenter->y - (viewSize.y * 0.5)});

    const auto bottomRight = topdown::ToPositionInWorld(
        PositionInView{viewCenter->x + (viewSize.x * 0.5), viewCenter->y + (viewSize.y * 0.5)});

    const int startX = std::max(0, static_cast<int>(topLeft->x / cr));
    const int startY = std::max(0, static_cast<int>(topLeft->y / cr));

    const int endX = std::min(world.getCellCountX() - 1, static_cast<int>(bottomRight->x / cr));
    const int endY = std::min(world.getCellCountY() - 1, static_cast<int>(bottomRight->y / cr));

    cell::FloorSprite floorSprite;
    cell::WallSprite  wallSprite;
    for (int y = startY; y <= endY; y += 1) {
        for (int x = startX; x <= endX; x += 1) {
            const auto cellIsLoaded = world.getCellDataAtUnchecked(x, y, &floorSprite, &wallSprite);
            if (!cellIsLoaded) {
                continue;
            }

            if (floorSprite.id != SPRITEID_NONE) {
                _cellAdapters.emplace_back(
                    *this,
                    BoundsInfo::fromTopLeftAndSize({x * cr, y * cr}, {cr, cr}, Layer::FLOOR),
                    cell::FloorSprite{.id = floorSprite.id});
            }
            if (wallSprite.id != SPRITEID_NONE) {
                _cellAdapters.emplace_back(
                    *this,
                    BoundsInfo::fromTopLeftAndSize({x * cr, y * cr}, {cr, cr}, Layer::WALL),
                    cell::WallSprite{.id = wallSprite.id, .id_reduced = SPRITEID_NONE});
            }
        }
    }

    for (const auto& adapter : _cellAdapters) {
        aRenderCtx.ephemeral.renderedObjects.push_back(&adapter);
    }
}

// MARK: Cell adapter impl

TopDownRenderer::CellToRenderedObjectAdapter::CellToRenderedObjectAdapter(TopDownRenderer&  aRenderer,
                                                                          const BoundsInfo& aBoundsInfo,
                                                                          cell::FloorSprite aFloorSprite)
    : RenderedObject{aBoundsInfo}
    , _renderer{aRenderer}
    , _floorSprite{aFloorSprite} {}

TopDownRenderer::CellToRenderedObjectAdapter::CellToRenderedObjectAdapter(TopDownRenderer&  aRenderer,
                                                                          const BoundsInfo& aBoundsInfo,
                                                                          cell::WallSprite  aWallSprite)
    : RenderedObject{aBoundsInfo}
    , _renderer{aRenderer}
    , _wallSprite{aWallSprite} {}

void TopDownRenderer::CellToRenderedObjectAdapter::render(hg::uwga::Canvas& aCanvas,
                                                          PositionInView    aScreenPosition) const {
    // Select sprite ID based on layer
    SpriteId spriteId;
    switch (_boundsInfo.getLayer()) {
    case Layer::FLOOR:
        spriteId = _floorSprite.id;
        break;
    case Layer::WALL:
        spriteId = _wallSprite.id;
        break;

    default:
        HG_UNREACHABLE("Invalid value for Layer ({}).", (int)_boundsInfo.getLayer());
        break;
    }

    // Draw
    auto& sprite = _renderer._getSprite(spriteId);

    sprite.setColor(hg::uwga::COLOR_WHITE);
    sprite.setAnchor(*aScreenPosition);

    {
        float xscale = 1.f;
        float yscale = 1.f;

        if ((spriteId & SPRITEID_HFLIP) != 0) {
            xscale = -1.f;
        }
        if ((spriteId & SPRITEID_VFLIP) != 0) {
            yscale = -1.f;
        }

        sprite.setScale(xscale, yscale);
    }

    aCanvas.draw(sprite);
}

} // namespace gridgoblin
} // namespace jbatnozic

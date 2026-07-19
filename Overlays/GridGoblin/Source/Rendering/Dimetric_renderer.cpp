// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Positional/Position_conversions.hpp>
#include <GridGoblin/Private/Reduction_predicates.hpp>
#include <GridGoblin/Rendering/Dimetric_renderer.hpp>
#include <GridGoblin/Rendering/Drawing_order.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>

#include <algorithm>
#include <optional>

namespace jbatnozic {
namespace gridgoblin {

// MARK: Cell renderer mask bits

#define RM_REDUCTION_COUNTER_MASK static_cast<std::uint32_t>(0x03FF)
#define RM_RENDER_CYCLE_FLAG      static_cast<std::uint32_t>(0x0400)
#define RM_CELL_TOUCHED           static_cast<std::uint32_t>(0x0800)
#define RM_SHOULD_REDUCE          static_cast<std::uint32_t>(0x1000)

// MARK: Templates

template <class taCallable>
void DimetricRenderer::_diagonalTraverse(const World&       aWorld,
                                         PositionInWorld    aViewTopLeft,
                                         hg::math::Vector2d aViewSize,
                                         taCallable&&       aFunc) {
    const double cellRes = aWorld.getCellResolution();

    const int cellsPerRow    = static_cast<int>((aViewSize.x) / (2.0 * cellRes)) + 1;
    const int cellsPerColumn = static_cast<int>((aViewSize.y) * 2.0 / cellRes);

    int startX = static_cast<int>(trunc(aViewTopLeft->x / cellRes));
    int startY = static_cast<int>(trunc(aViewTopLeft->y / cellRes));

    for (int row = 0; row < cellsPerColumn; row += 1) {
        for (int col = 0; col < cellsPerRow; col += 1) {
            const int x = startX + col;
            const int y = startY + col;

            if (x >= 0 && x < aWorld.getCellCountX() && y >= 0 && y < aWorld.getCellCountY()) {
                std::optional<CellInfo> cellInfo = CellInfo{.gridX = x, .gridY = y};

                if (!aWorld.getCellDataAtUnchecked(x,
                                                   y,
                                                   &cellInfo->floorSprite,
                                                   &cellInfo->wallSprite,
                                                   &cellInfo->rendererAuxData,
                                                   &cellInfo->spatialInfo)) {
                    cellInfo.reset();
                }

                const auto posInWorld =
                    PositionInWorld{((float)x + 0.5f) * cellRes, ((float)y + 0.5f) * cellRes};
                const auto posInView = dimetric::ToPositionInView(posInWorld);

                aFunc(cellInfo, posInView);
            }
        }

        if (row % 2 == 0) {
            startX -= 1;
        } else {
            startY += 1;
        }
    }
}

// MARK: Public

DimetricRenderer::DimetricRenderer(const hg::uwga::SpriteLoader& aSpriteLoader)
    : _spriteLoader{aSpriteLoader} //
{
    // HG_VALIDATE_ARGUMENT(aConfig.wallReductionConfig.lowerBound <
    //                      aConfig.wallReductionConfig.upperBound);
}

void DimetricRenderer::reset(RenderContext& aRenderCtx) {
    // HG_VALIDATE_ARGUMENT(!!(aRenderFlags & REDUCE_WALLS_BASED_ON_VISIBILITY) == !!aVisProv);

    _renderCtx = nullptr;

    _cellAdapters.clear();
}

void DimetricRenderer::prepareToRender(RenderContext& aRenderCtx) {
    _renderCtx = &aRenderCtx;

    _viewTopLeft = dimetric::ToPositionInWorld(
        PositionInView{aRenderCtx.dynamic.viewCenter->x - (aRenderCtx.dynamic.viewSize.x * 0.5),
                       aRenderCtx.dynamic.viewCenter->y - (aRenderCtx.dynamic.viewSize.y * 0.5)});

    _prepareCells();

    _renderCycleCounter += 1;

    auto& objs = aRenderCtx.ephemeral.renderedObjects;
    std::sort(objs.begin(),
              objs.end(),
              [](const RenderedObject* aLhs, const RenderedObject* aRhs) -> bool {
                  // Implements: Does `aLhs` come before `aRhs`?

                  HG_ASSERT(aLhs != nullptr && aRhs != nullptr);

                  const auto order =
                      dimetric::CheckDrawingOrder(aLhs->getBoundsInfo(), aRhs->getBoundsInfo());

                  switch (order) {
                  case dimetric::DRAW_LHS_FIRST:
                      return true;

                  case dimetric::DOES_NOT_MATTER:
                      return (aLhs < aRhs);

                  case dimetric::DRAW_RHS_FIRST:
                      return false;

                  default:
                      HG_UNREACHABLE("Invalid value for drawing order ({}).", (int)order);
                      return {};
                  }
              });
}

void DimetricRenderer::render(const RenderContext&          aRenderCtx,
                              hg::uwga::Canvas&             aCanvas,
                              const hg::uwga::RenderStates& aRenderStates) const //
{
    (void)aRenderStates; // TODO: aRenderStates unused
    const auto& objs = aRenderCtx.ephemeral.renderedObjects;
    for (const auto& object : objs) {
        const auto& boundsInfo = object->getBoundsInfo();
        auto        posInView  = dimetric::ToPositionInView(boundsInfo.getCenter());
        object->render(aCanvas, posInView);
    }
}

// MARK: Private

hg::uwga::Sprite& DimetricRenderer::_getSprite(SpriteId aSpriteId) const {
    aSpriteId = ((aSpriteId & SPRITEID_IDENTIFIER_MASK) | SPRITEID_PROJECTION_DIMETRIC);

    const auto iter = _spriteCache.find(aSpriteId);
    if (iter != _spriteCache.end()) {
        return iter->second;
    }

    const auto blueprint = _spriteLoader.getBlueprint(aSpriteId);
    const auto newIter   = _spriteCache.emplace(std::make_pair(aSpriteId, blueprint.spr()));
    return newIter.first->second;
}

void DimetricRenderer::_reduceCellsBelowIfCellIsVisible(hg::math::Vector2pz       aCell,
                                                        PositionInView            aCellPosInView,
                                                        const VisibilityProvider& aVisProv) {
#if 0
    const auto cr = _world.getCellResolution();

    static constexpr double  PADDING      = 1.f;
    const hg::math::Vector2d positions[4] = {
        {(aCell.x + 0) * cr + PADDING, (aCell.y + 0) * cr + PADDING},
        {(aCell.x + 1) * cr - PADDING, (aCell.y + 0) * cr + PADDING},
        {(aCell.x + 1) * cr - PADDING, (aCell.y + 1) * cr - PADDING},
        {(aCell.x + 0) * cr + PADDING, (aCell.y + 1) * cr - PADDING}
    };

    const bool cellIsVisible =
        std::any_of(std::begin(positions), std::end(positions), [&aVisProv](const auto& aPos) {
            return aVisProv.testVisibilityAt(PositionInWorld{aPos}).value_or(false);
        });

    const auto markCell = [this](hg::math::Vector2pz aCell) {
        auto* cell = _world.getCellAt(aCell);
        if (cell) {
            auto& ext  = GetMutableExtensionData(*cell);
            auto  mask = ext.getRendererMask();
            if ((_renderCycleCounter & 0x01) == 0) {
                mask |= (RM_RENDER_CYCLE_FLAG | RM_CELL_TOUCHED | RM_SHOULD_REDUCE);
            } else {
                mask &= ~(RM_RENDER_CYCLE_FLAG | RM_CELL_TOUCHED);
                mask |= RM_SHOULD_REDUCE;
            }
            ext.setRendererMask(mask);
        }
    };

    if (!cellIsVisible) {
        return;
    }

    const auto screenPov = dimetric::ToPositionInView(_renderParams.pointOfView);
    const auto limit     = static_cast<int>(_world.getWallHeight() / cr);
    for (int i = 0; i < limit; i += 1) {
        if (screenPov->x > aCellPosInView->x) {
            const int x = aCell.x - 1 - i;
            const int y = aCell.y + 0 + i;

            if (x >= 0 && x < _world.getCellCountX() && y >= 0 && y < _world.getCellCountY()) {
                markCell({x, y});
            }
        }
        {
            const int x = aCell.x - 1 - i;
            const int y = aCell.y + 1 + i;

            if (x >= 0 && x < _world.getCellCountX() && y >= 0 && y < _world.getCellCountY()) {
                markCell({x, y});
            }
        }
        if (screenPov->x < aCellPosInView->x) {
            const int x = aCell.x - 0 - i;
            const int y = aCell.y + 1 + i;

            if (x >= 0 && x < _world.getCellCountX() && y >= 0 && y < _world.getCellCountY()) {
                markCell({x, y});
            }
        }
    } // end_for
#endif
}

void DimetricRenderer::_prepareCells() {
    auto& world = *(_renderCtx->world);

    world.edit(*world.getPermissionToEdit(), [this, &world](World::Editor aEditor) {
        _diagonalTraverse(
            world,
            _viewTopLeft,
            _renderCtx->dynamic.viewSize,
            [this, &world, aEditor](std::optional<CellInfo> aCellInfo,
                                    PositionInView          aPosInView) //
            {
                if (!aCellInfo.has_value()) {
                    return; // The cell's chunk isn't currently loaded
                }

                _updateRendererAuxDataOfCell(*aCellInfo, aEditor);

                if (const auto* visProv = _renderCtx->impls.visibilityProvider;
                    visProv != nullptr && (aCellInfo->wallSprite.id == SPRITEID_NONE)) //
                {
                    _reduceCellsBelowIfCellIsVisible({aCellInfo->gridX, aCellInfo->gridY},
                                                     aPosInView,
                                                     *visProv);
                }

                const auto cr = world.getCellResolution();

                const auto drawMode = (detail::GetReductionPredicate(aCellInfo->spatialInfo))(
                    cr,
                    {aCellInfo->gridX * cr, aCellInfo->gridY * cr},
                    _renderCtx->dynamic.pointOfView);

                _updateFadeValueOfCellRendererMask(*aCellInfo, drawMode, aEditor);

                if (drawMode == detail::RecommendedDrawMode::NOT_DRAWN) {
                    return;
                }

                if (aCellInfo->wallSprite.id != SPRITEID_NONE) {
                    _cellAdapters.emplace_back(
                        *this,
                        *aCellInfo,
                        BoundsInfo::fromTopLeftAndSize({aCellInfo->gridX * cr, aCellInfo->gridY * cr},
                                                       {cr, cr},
                                                       Layer::WALL));
                } else if (aCellInfo->floorSprite.id != SPRITEID_NONE) {
                    _cellAdapters.emplace_back(
                        *this,
                        *aCellInfo,
                        BoundsInfo::fromTopLeftAndSize({aCellInfo->gridX * cr, aCellInfo->gridY * cr},
                                                       {cr, cr},
                                                       Layer::FLOOR));
                }
            });
    });

    for (const auto& adapter : _cellAdapters) {
        _renderCtx->ephemeral.renderedObjects.push_back(&adapter);
    }
}

void DimetricRenderer::_updateRendererAuxDataOfCell(CellInfo& aCellInfo, World::Editor aWorldEditor) {
    auto bits = aCellInfo.rendererAuxData.storage;

    if ((_renderCycleCounter & 0x01) == 0) {
        static constexpr auto EXPECTED           = RM_RENDER_CYCLE_FLAG | RM_CELL_TOUCHED;
        const bool            cellAlreadyTouched = ((bits & EXPECTED) == EXPECTED);
        if (!cellAlreadyTouched) {
            bits &= ~RM_SHOULD_REDUCE;
            bits |= EXPECTED;
        }
    } else {
        static constexpr auto EXPECTED           = RM_RENDER_CYCLE_FLAG | RM_CELL_TOUCHED;
        const bool            cellAlreadyTouched = ((bits & EXPECTED) == 0);
        if (!cellAlreadyTouched) {
            bits &= ~RM_SHOULD_REDUCE;
            bits &= ~EXPECTED;
        }
    }

    aWorldEditor.setCellDataAtUnchecked(aCellInfo.gridX, aCellInfo.gridY, &aCellInfo.rendererAuxData);
}

void DimetricRenderer::_updateFadeValueOfCellRendererMask(CellInfo&                   aCellInfo,
                                                          detail::RecommendedDrawMode aDrawMode,
                                                          World::Editor               aWorldEditor) {
    const auto cr      = _renderCtx->world->getCellResolution();
    const auto cellPos = hg::math::Vector2d{(aCellInfo.gridX + 0.5) * cr, (aCellInfo.gridY + 0.5) * cr};
    const auto rflags  = _renderCtx->dynamic.flags;

    auto& rAuxDataBits = aCellInfo.rendererAuxData.storage;

    if (hg::math::EuclideanDist(cellPos, *(_renderCtx->dynamic.pointOfView)) >
        _renderCtx->config.wallReductionConfig.reductionDistanceLimit) //
    {
        rAuxDataBits &= ~RM_SHOULD_REDUCE;
    } else if ((rflags & RenderFlags::REDUCE_WALLS_BASED_ON_POSITION) != RenderFlags::NONE) {
        switch (aDrawMode) {
        case detail::RecommendedDrawMode::NOT_DRAWN:
            rAuxDataBits &= ~RM_SHOULD_REDUCE;
            break;

        case detail::RecommendedDrawMode::REDUCED:
            rAuxDataBits |= RM_SHOULD_REDUCE;
            break;

        case detail::RecommendedDrawMode::FULL:
            if ((rflags & RenderFlags::REDUCE_WALLS_BASED_ON_VISIBILITY) == RenderFlags::NONE) {
                rAuxDataBits &= ~RM_SHOULD_REDUCE;
            }
            break;

        default:
            HG_UNREACHABLE("Unexpected value for DrawingData::DrawMode ({}).", (int)aDrawMode);
            break;
        }
    } else if ((rflags & RenderFlags::REDUCE_WALLS_BASED_ON_VISIBILITY) == RenderFlags::NONE) {
        rAuxDataBits &= ~RM_SHOULD_REDUCE;
    }

    auto reductionCounter = rAuxDataBits & RM_REDUCTION_COUNTER_MASK;
    if ((rAuxDataBits & RM_SHOULD_REDUCE) == 0) {
        if (reductionCounter < _renderCtx->config.wallReductionConfig.delta) {
            reductionCounter = 0;
        } else {
            reductionCounter -= _renderCtx->config.wallReductionConfig.delta;
        }
    } else {
        if (reductionCounter + _renderCtx->config.wallReductionConfig.delta >
            RenderContext::Config::WallReductionConfig::MAX_VALUE) //
        {
            reductionCounter = RenderContext::Config::WallReductionConfig::MAX_VALUE;
        } else {
            reductionCounter += _renderCtx->config.wallReductionConfig.delta;
        }
    }

    rAuxDataBits = (rAuxDataBits & ~RM_REDUCTION_COUNTER_MASK) | reductionCounter;

    aWorldEditor.setCellDataAtUnchecked(aCellInfo.gridX, aCellInfo.gridY, &aCellInfo.rendererAuxData);
}

// MARK: Cell adapter impl

DimetricRenderer::CellToRenderedObjectAdapter::CellToRenderedObjectAdapter(
    DimetricRenderer& aRenderer,
    CellGraphicsInfo  aCellGraphicsInfo,
    const BoundsInfo& aBoundsInfo)
    : RenderedObject{aBoundsInfo}
    , _renderer{aRenderer}
    , _cellGraphicsInfo{aCellGraphicsInfo} {}

void DimetricRenderer::CellToRenderedObjectAdapter::render(hg::uwga::Canvas& aCanvas,
                                                           PositionInView    aScreenPosition) const {
    switch (_boundsInfo.getLayer()) {
    case Layer::FLOOR:
        {
            auto& sprite = _renderer._getSprite(_cellGraphicsInfo.floorSprite.id);
            sprite.setAnchor(*aScreenPosition);
            sprite.setColor(hg::uwga::COLOR_WHITE);
            aCanvas.draw(sprite);
        }
        break;

    case Layer::WALL:
        {
            std::uint8_t opacity;
            const auto   reductionCounter =
                _cellGraphicsInfo.rendererAuxData.storage & RM_REDUCTION_COUNTER_MASK;
            const auto& wrConfig = _renderer._renderCtx->config.wallReductionConfig;

            if (reductionCounter <= wrConfig.lowerBound) {
                opacity = 255;
            } else if (reductionCounter >= wrConfig.upperBound) {
                opacity = static_cast<std::uint8_t>(255 * (1.f - wrConfig.maxReduction));
            } else {
                const auto stepCount        = wrConfig.upperBound - wrConfig.lowerBound;
                const auto stepsTaken       = reductionCounter - wrConfig.lowerBound;
                const auto reductionPerStep = wrConfig.maxReduction / (float)stepCount;

                opacity = static_cast<std::uint8_t>(
                    hg::math::Clamp(255.f * (1.f - (float)stepsTaken * reductionPerStep), 0.f, 255.f));
            }

            if (opacity < 255) {
                auto& reducedSprite = _renderer._getSprite(_cellGraphicsInfo.wallSprite.id_reduced);

                reducedSprite.setAnchor(*aScreenPosition);
                reducedSprite.setColor(hg::uwga::COLOR_WHITE);
                aCanvas.draw(reducedSprite);
            }

            if (opacity > 0) {
                auto& fullSprite = _renderer._getSprite(_cellGraphicsInfo.wallSprite.id);

                fullSprite.setAnchor(*aScreenPosition);
                fullSprite.setColor(hg::uwga::COLOR_WHITE.withAlpha(opacity));
                aCanvas.draw(fullSprite);
            }
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for Layer ({}).", (int)_boundsInfo.getLayer());
        break;
    }
}

} // namespace gridgoblin
} // namespace jbatnozic

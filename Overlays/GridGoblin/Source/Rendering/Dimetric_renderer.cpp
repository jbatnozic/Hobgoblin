// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Rendering/Dimetric_renderer.hpp>
#include <GridGoblin/Rendering/Drawing_order.hpp>
#include <GridGoblin/Spatial/Position_conversions.hpp>

#include "../Detail_access.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>

#include <algorithm>

namespace jbatnozic {
namespace gridgoblin {

#define vector_cast hg::math::VectorCast

// MARK: Cell renderer mask bits

#define RM_REDUCTION_COUNTER_MASK 0x03FF
#define RM_RENDER_CYCLE_FLAG      0x0400
#define RM_CELL_TOUCHED           0x0800
#define RM_SHOULD_REDUCE          0x1000

// MARK: Templates

template <class taCallable>
void DimetricRenderer::_diagonalTraverse(const World&                                 aWorld,
                                         const DimetricRenderer::RenderParametersExt& aRenderParams,
                                         taCallable&&                                 aFunc) {
    const double cellRes = aWorld.getCellResolution();

    const int cellsPerRow    = static_cast<int>((aRenderParams.viewSize.x) / (2.0 * cellRes)) + 1;
    const int cellsPerColumn = static_cast<int>((aRenderParams.viewSize.y) * 2.0 / cellRes);

    int startX = static_cast<int>(trunc(aRenderParams.topLeft->x / cellRes));
    int startY = static_cast<int>(trunc(aRenderParams.topLeft->y / cellRes));

    for (int row = 0; row < cellsPerColumn; row += 1) {
        for (int col = 0; col < cellsPerRow; col += 1) {
            const int x = startX + col;
            const int y = startY + col;

            if (x >= 0 && x < aWorld.getCellCountX() && y >= 0 && y < aWorld.getCellCountY()) {
                const auto* cell = aWorld.getCellAtUnchecked(x, y);

                const auto posInWorld =
                    PositionInWorld{((float)x + 0.5f) * cellRes, ((float)y + 0.5f) * cellRes};
                const auto posInView = dimetric::ToPositionInView(posInWorld);

                aFunc(CellInfo{cell, x, y}, posInView);
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

DimetricRenderer::DimetricRenderer(const World&                  aWorld,
                                   const hg::gr::SpriteLoader&   aSpriteLoader,
                                   const DimetricRendererConfig& aConfig)
    : _world{aWorld}
    , _spriteLoader{aSpriteLoader}
    , _config{aConfig} //
{
    HG_VALIDATE_ARGUMENT(aConfig.wallReductionConfig.lowerBound <
                         aConfig.wallReductionConfig.upperBound);
}

void DimetricRenderer::startPrepareToRender(const RenderParameters&   aRenderParams,
                                            std::int32_t              aRenderFlags,
                                            const VisibilityProvider* aVisProv) {
    HG_VALIDATE_ARGUMENT(!!(aRenderFlags & REDUCE_WALLS_BASED_ON_VISIBILITY) == !!aVisProv);

    static_cast<RenderParameters&>(_renderParams) = aRenderParams;

    _renderParams.topLeft = dimetric::ToPositionInWorld(
        PositionInView{_renderParams.viewCenter->x - (_renderParams.viewSize.x * 0.5),
                       _renderParams.viewCenter->y - (_renderParams.viewSize.y * 0.5)});

    _objectsToRender.clear();
    _cellAdapters.clear();

    _prepareCells(aRenderFlags, aVisProv);

    _renderCycleCounter += 1;
}

void DimetricRenderer::addObject(const RenderedObject& aObject) {
    HG_NOT_IMPLEMENTED("TODO");
}

void DimetricRenderer::endPrepareToRender() {
    std::sort(_objectsToRender.begin(),
              _objectsToRender.end(),
              [](const RenderedObject* aLhs, const RenderedObject* aRhs) -> bool {
                  // Implements: Does `aLhs` come before `aRhs`?

                  HG_ASSERT(aLhs != nullptr && aRhs != nullptr);

                  const auto order =
                      dimetric::CheckDrawingOrder(aLhs->getSpatialInfo(), aRhs->getSpatialInfo());

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

void DimetricRenderer::render(hg::gr::Canvas& aCanvas) {
    for (const auto& object : _objectsToRender) {
        const auto& spatialInfo = object->getSpatialInfo();
        auto        posInView   = dimetric::ToPositionInView(spatialInfo.getCenter());
        posInView->x += _renderParams.xOffset;
        posInView->y += _renderParams.yOffset;
        object->render(aCanvas, posInView);
    }
}

// MARK: Private

hg::gr::Sprite& DimetricRenderer::_getSprite(SpriteId aSpriteId) const {
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
}

void DimetricRenderer::_prepareCells(std::int32_t aRenderFlags, const VisibilityProvider* aVisProv) {
    const auto cr = _world.getCellResolution();

    _diagonalTraverse(
        _world,
        _renderParams,
        [this, cr, aRenderFlags, aVisProv](const CellInfo& aCellInfo, PositionInView aPosInView) {
            if (aCellInfo.cell == nullptr) {
                return;
            }

            std::uint16_t mask;
            mask = _updateFlagsOfCellRendererMask(*aCellInfo.cell);

            const auto flags = aCellInfo.cell->getFlags();

            if (aVisProv != nullptr && (flags & CellModel::WALL_INITIALIZED) == 0) {
                _reduceCellsBelowIfCellIsVisible({aCellInfo.gridX, aCellInfo.gridY},
                                                 aPosInView,
                                                 *aVisProv);
            }

            const auto drawingData = GetExtensionData(*aCellInfo.cell)
                                         .getDrawingData(cr,
                                                         {aCellInfo.gridX * cr, aCellInfo.gridY * cr},
                                                         _renderParams.pointOfView);

            mask = _updateFadeValueOfCellRendererMask(aCellInfo, drawingData, aRenderFlags);

            if (drawingData.suggestedDrawMode == detail::DrawingData::NONE) {
                return;
            }

            if (flags & CellModel::WALL_INITIALIZED) {
                _cellAdapters.emplace_back(
                    *this,
                    *aCellInfo.cell,
                    SpatialInfo::fromTopLeftAndSize({aCellInfo.gridX * cr, aCellInfo.gridY * cr},
                                                    {cr, cr},
                                                    Layer::WALL),
                    mask);
            } else if (flags & CellModel::FLOOR_INITIALIZED) {
                _cellAdapters.emplace_back(
                    *this,
                    *aCellInfo.cell,
                    SpatialInfo::fromTopLeftAndSize({aCellInfo.gridX * cr, aCellInfo.gridY * cr},
                                                    {cr, cr},
                                                    Layer::FLOOR),
                    mask);
            }
        });

    for (const auto& adapter : _cellAdapters) {
        _objectsToRender.push_back(&adapter);
    }
}

std::uint16_t DimetricRenderer::_updateFlagsOfCellRendererMask(const CellModel& aCell) {
    auto& ext  = GetMutableExtensionData(aCell);
    auto  mask = ext.getRendererMask();

    if ((_renderCycleCounter & 0x01) == 0) {
        static constexpr auto EXPECTED           = RM_RENDER_CYCLE_FLAG | RM_CELL_TOUCHED;
        const bool            cellAlreadyTouched = ((mask & EXPECTED) == EXPECTED);
        if (!cellAlreadyTouched) {
            mask &= ~RM_SHOULD_REDUCE;
            mask |= EXPECTED;
        }
    } else {
        static constexpr auto EXPECTED           = RM_RENDER_CYCLE_FLAG | RM_CELL_TOUCHED;
        const bool            cellAlreadyTouched = ((mask & EXPECTED) == 0);
        if (!cellAlreadyTouched) {
            mask &= ~RM_SHOULD_REDUCE;
            mask &= ~EXPECTED;
        }
    }

    ext.setRendererMask(mask);
    return mask;
}

std::uint16_t DimetricRenderer::_updateFadeValueOfCellRendererMask(
    const CellInfo&            aCellInfo,
    const detail::DrawingData& aDrawingData,
    std::int32_t               aRenderFlags) //
{
    auto& ext  = GetMutableExtensionData(*aCellInfo.cell);
    auto  mask = ext.getRendererMask();

    const auto cr      = _world.getCellResolution();
    const auto cellPos = hg::math::Vector2d{(aCellInfo.gridX + 0.5) * cr, (aCellInfo.gridY + 0.5) * cr};

    if (hg::math::EuclideanDist(cellPos, *_renderParams.pointOfView) >
        _config.wallReductionConfig.reductionDistanceLimit) //
    {
        mask &= ~RM_SHOULD_REDUCE;
    } else if ((aRenderFlags & REDUCE_WALLS_BASED_ON_POSITION) != 0) {
        switch (aDrawingData.suggestedDrawMode) {
        case detail::DrawingData::NONE:
            mask &= ~RM_SHOULD_REDUCE;
            break;

        case detail::DrawingData::REDUCED:
            mask |= RM_SHOULD_REDUCE;
            break;

        case detail::DrawingData::FULL:
            if ((aRenderFlags & REDUCE_WALLS_BASED_ON_VISIBILITY) == 0) {
                mask &= ~RM_SHOULD_REDUCE;
            }
            break;

        default:
            HG_UNREACHABLE("Unexpected value for DrawingData::DrawMode ({}).",
                           (int)aDrawingData.suggestedDrawMode);
            break;
        }
    } else if ((aRenderFlags & REDUCE_WALLS_BASED_ON_VISIBILITY) == 0) {
        mask &= ~RM_SHOULD_REDUCE;
    }

    auto reductionCounter = mask & RM_REDUCTION_COUNTER_MASK;
    if ((mask & RM_SHOULD_REDUCE) == 0) {
        if (reductionCounter < _config.wallReductionConfig.delta) {
            reductionCounter = 0;
        } else {
            reductionCounter -= _config.wallReductionConfig.delta;
        }
    } else {
        if (reductionCounter + _config.wallReductionConfig.delta > WallReductionConfig::MAX_VALUE) {
            reductionCounter = WallReductionConfig::MAX_VALUE;
        } else {
            reductionCounter += _config.wallReductionConfig.delta;
        }
    }

    mask = static_cast<std::uint16_t>((mask & ~RM_REDUCTION_COUNTER_MASK) | reductionCounter);

    ext.setRendererMask(mask);
    return mask;
}

// MARK: Cell adapter impl

DimetricRenderer::CellToRenderedObjectAdapter::CellToRenderedObjectAdapter(
    DimetricRenderer&  aRenderer,
    const CellModel&   aCell,
    const SpatialInfo& aSpatialInfo,
    std::uint16_t      aRendererMask)
    : RenderedObject{aSpatialInfo}
    , _renderer{aRenderer}
    , _cell{aCell}
    , _rendererMask{aRendererMask} {}

void DimetricRenderer::CellToRenderedObjectAdapter::render(hg::gr::Canvas& aCanvas,
                                                           PositionInView  aScreenPosition) const {
    switch (_spatialInfo.getLayer()) {
    case Layer::FLOOR:
        {
            auto& sprite = _renderer._getSprite(_cell.getFloor().spriteId);
            sprite.setPosition(vector_cast<float>(*aScreenPosition));
            sprite.setColor(hg::gr::COLOR_WHITE);
            aCanvas.draw(sprite);
        }
        break;

    case Layer::WALL:
        {
            std::uint8_t opacity;
            const auto   reductionCounter = _rendererMask & RM_REDUCTION_COUNTER_MASK;
            const auto&  wrConfig         = _renderer._config.wallReductionConfig;

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
                auto& reducedSprite = _renderer._getSprite(_cell.getWall().spriteId_reduced);

                reducedSprite.setPosition(vector_cast<float>(*aScreenPosition));
                reducedSprite.setColor(hg::gr::COLOR_WHITE);
                aCanvas.draw(reducedSprite);
            }

            if (opacity > 0) {
                auto& fullSprite = _renderer._getSprite(_cell.getWall().spriteId);

                fullSprite.setPosition(vector_cast<float>(*aScreenPosition));
                fullSprite.setColor(hg::gr::COLOR_WHITE.withAlpha(opacity));
                aCanvas.draw(fullSprite);
            }
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for Layer ({}).", (int)_spatialInfo.getLayer());
        break;
    }
}

} // namespace gridgoblin
} // namespace jbatnozic

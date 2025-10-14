// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Rendering/Visibility_calculator.hpp>

#include <GridGoblin/Model/Shape_vertices.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Logging.hpp>

#include <array>
#include <cmath>

namespace jbatnozic {
namespace gridgoblin {

namespace {
using CellFlags = std::uint8_t;

using hg::math::AngleD;
using hg::math::Clamp;
using hg::math::Sign;
using hg::math::Sqr;
using hg::math::Vector2d;
using hg::math::Vector2pz;

std::size_t GetUnobstructedVertices(cell::SpatialInfo        aCellSpatialInfo,
                                    Vector2pz                aCellCoords,
                                    CellFlags                aEdgesOfInterest,
                                    bool                     aAllEdgesOverride,
                                    double                   aCellResolution,
                                    std::array<Vector2d, 8>& aVertices) {
    // We divide by aCellResolution because later we will multiply by it and
    // then the offset ends up always being 0.25 pixels.
    const double offset = 0.25 / aCellResolution;

    const auto cnt =
        GetVisibilityVertices(aCellSpatialInfo, aEdgesOfInterest, aAllEdgesOverride, offset, aVertices);

    const auto tlCorner =
        hg::math::Vector2d{aCellCoords.x * aCellResolution, aCellCoords.y * aCellResolution};

    for (std::size_t i = 0; i < cnt; i += 1) {
        aVertices[i] = tlCorner + (aVertices[i] * aCellResolution);
    }

    return cnt;
}

void ProjectRay(/*  in */ Vector2d  aOrigin,
                /*  in */ Vector2d  aPassesThrough,
                /*  in */ double    aLength,
                /* out */ Vector2d& aRayEnd,
                /* out */ AngleD&   aRayAngle) {
    aPassesThrough -= aOrigin;
    aRayAngle = AngleD::fromVector(aPassesThrough.x, aPassesThrough.y);
    aRayEnd   = aOrigin + aRayAngle.asNormalizedVector() * aLength;
}
} // namespace

VisibilityCalculator::VisibilityCalculator(const World&                      aWorld,
                                           const VisibilityCalculatorConfig& aConfig)
    : _world{aWorld}
    , _cr{_world.getCellResolution()}
    , _xLimit{std::nextafter(_world.getCellCountX() * _cr, 0.0)}
    , _yLimit{std::nextafter(_world.getCellCountY() * _cr, 0.0)}
    , _minRingsBeforeRaycasting{aConfig.minRingsBeforeRaycasting}
    , _minTrianglesBeforeRaycasting{aConfig.minTrianglesBeforeRaycasting}
    , _rayCount{aConfig.rayCount}
    , _rayPointsPerCell{aConfig.rayPointsPerCell} //
{
    _rays.resize(hg::pztos(_rayCount));
}

std::optional<bool> VisibilityCalculator::testVisibilityAt(PositionInWorld aPos) const {
    if (_viewBbox.overlaps(*aPos)) {
        return _isPointVisible(aPos, 0, true);
    }
    return std::nullopt;
}

void VisibilityCalculator::calc(PositionInWorld aViewCenter,
                                Vector2d        aViewSize,
                                PositionInWorld aLineOfSightOrigin) {
    _resetData();
    _setInitialCalculationContext(aViewCenter, aViewSize, aLineOfSightOrigin);

    _calcOngoing = true;

    hg::PZInteger currentRingIndex = 0;
    while ((_lineOfSightOriginCell.x - currentRingIndex >= _viewTopLeftCell.x) ||
           (_lineOfSightOriginCell.x + currentRingIndex <= _viewBottomRightCell.x) ||
           (_lineOfSightOriginCell.y - currentRingIndex >= _viewTopLeftCell.y) ||
           (_lineOfSightOriginCell.y + currentRingIndex <= _viewBottomRightCell.y)) //
    {
        _processRing(currentRingIndex);

        _stats.highDetailRingCount = currentRingIndex;
        currentRingIndex += 1;

        if ((_minRingsBeforeRaycasting != 0 && currentRingIndex > _minRingsBeforeRaycasting) &&
            _triangles.size() >= hg::pztos(_minTrianglesBeforeRaycasting)) //
        {
            _processRays();
            _rayCheckingEnabled = true;
            break;
        }
    }

    _calcOngoing = false;

    _stats.triangleCount = hg::stopz(_triangles.size());
}

auto VisibilityCalculator::getStats() const -> const CalculationStats& {
    return _stats;
}

// MARK: Private

void VisibilityCalculator::_resetData() {
    for (auto& r : _rays) {
        r = INFINITY;
    }

    _triangles.clear();

    _stats = {.highDetailRingCount = 0, .triangleCount = 0, .triangleCheckCount = 0};
}

void VisibilityCalculator::_setInitialCalculationContext(PositionInWorld    aViewCenter,
                                                         hg::math::Vector2d aViewSize,
                                                         PositionInWorld    aLineOfSightOrigin) {
    _processedRingsBbox = {0.0, 0.0, 0.0, 0.0};

    _viewBbox = {Clamp(aViewCenter->x - aViewSize.x * 0.5f, 0.0, _xLimit),
                 Clamp(aViewCenter->y - aViewSize.y * 0.5f, 0.0, _yLimit),
                 aViewSize.x,
                 aViewSize.y};

    if (_viewBbox.getRight() >= _xLimit) {
        _viewBbox.w = _xLimit - _viewBbox.x;
    }
    if (_viewBbox.getBottom() >= _yLimit) {
        _viewBbox.h = _yLimit - _viewBbox.y;
    }

    _viewTopLeftCell     = _world.posToCell(_viewBbox.getLeft(), _viewBbox.getTop());
    _viewBottomRightCell = _world.posToCell(_viewBbox.getRight(), _viewBbox.getBottom());

    _lineOfSightOrigin     = *aLineOfSightOrigin;
    _lineOfSightOriginCell = {hg::ToPz(_lineOfSightOrigin.x / _cr),
                              hg::ToPz(_lineOfSightOrigin.y / _cr)};

    _triangleSideLength = std::sqrt(Sqr(_viewBbox.w) + Sqr(_viewBbox.h));

    _rayRadius = _minRingsBeforeRaycasting * _cr -
                 std::max(std::abs(_lineOfSightOrigin.x - (_lineOfSightOriginCell.x + 0.5) * _cr),
                          std::abs(_lineOfSightOrigin.y - (_lineOfSightOriginCell.y + 0.5) * _cr));

    // equal to _triangleSideLength / (_cr / _rayPointsPerCell)
    _maxPointsPerRay = hg::ToPz(_triangleSideLength * _rayPointsPerCell / _cr);

    _rayCheckingEnabled = false;
}

std::uint8_t VisibilityCalculator::_calcEdgesOfInterest(Vector2pz aCell) const {
    static constexpr CellFlags ALL_EDGES =
        cell::OBSTRUCTED_FULLY_BY_NORTH_NEIGHBOR | cell::OBSTRUCTED_FULLY_BY_WEST_NEIGHBOR |
        cell::OBSTRUCTED_FULLY_BY_EAST_NEIGHBOR | cell::OBSTRUCTED_FULLY_BY_SOUTH_NEIGHBOR;

    CellFlags edgesOfInterest = ALL_EDGES;

    switch (Sign(aCell.x - _lineOfSightOriginCell.x)) {
    case -1:
        edgesOfInterest ^= cell::OBSTRUCTED_FULLY_BY_EAST_NEIGHBOR;
        break;
    case +1:
        edgesOfInterest ^= cell::OBSTRUCTED_FULLY_BY_WEST_NEIGHBOR;
        break;
    }
    switch (Sign(aCell.y - _lineOfSightOriginCell.y)) {
    case -1:
        edgesOfInterest ^= cell::OBSTRUCTED_FULLY_BY_SOUTH_NEIGHBOR;
        break;
    case 1:
        edgesOfInterest ^= cell::OBSTRUCTED_FULLY_BY_NORTH_NEIGHBOR;
        break;
    }

    return edgesOfInterest;
}

bool VisibilityCalculator::_areAnyVerticesVisible(const std::array<Vector2d, 8>& aVertices,
                                                  std::size_t                    aVertCount,
                                                  std::uint8_t aEdgesOfInterest) const //
{
    for (std::size_t i = 0; i < aVertCount; i += 1) {
        if (i > 0 && aVertices[i] == aVertices[i - 1]) {
            continue;
        }
        if (_isPointVisible(PositionInWorld{aVertices[i]}, aEdgesOfInterest, false)) {
            return true;
        }
        if (i % 2 == 1 && _isLineVisible(PositionInWorld{aVertices[i]},
                                         PositionInWorld{aVertices[i - 1]},
                                         aEdgesOfInterest,
                                         2)) { // TODO: 2 = magic constant
            return true;
        }
    }

    return false;
}

void VisibilityCalculator::_processRing(hg::PZInteger aRingIndex) {
    if (aRingIndex == 0) {
        return; // TODO: special treatment? -- only when non-square shapes are supported
    }

    if (aRingIndex > _minRingsBeforeRaycasting) {
        _rayRadius += _cr;
    }

    const auto xStart = _lineOfSightOriginCell.x - aRingIndex;
    const auto xEnd   = _lineOfSightOriginCell.x + aRingIndex;

#define CELL_Y_IS_IN_BOUNDS(_y_) ((_y_) >= _viewTopLeftCell.y && (_y_) <= _viewBottomRightCell.y)
#define CELL_X_IS_IN_BOUNDS(_x_) ((_x_) >= _viewTopLeftCell.x && (_x_) <= _viewBottomRightCell.x)

    // Top row
    {
        const auto y = _lineOfSightOriginCell.y - aRingIndex;
        if (CELL_Y_IS_IN_BOUNDS(y)) {
            for (int x = xStart; x <= xEnd; x += 1) {
                if (CELL_X_IS_IN_BOUNDS(x)) {
                    _processCell({x, y}, aRingIndex);
                }
            }
        }
    }

    // Middle rows
    for (int y = _lineOfSightOriginCell.y - aRingIndex + 1;
         y <= _lineOfSightOriginCell.y + aRingIndex - 1;
         y += 1) {
        if (CELL_Y_IS_IN_BOUNDS(y)) {
            if (CELL_X_IS_IN_BOUNDS(xStart)) {
                _processCell({xStart, y}, aRingIndex);
            }
            if (CELL_X_IS_IN_BOUNDS(xEnd)) {
                _processCell({xEnd, y}, aRingIndex);
            }
        }
    }

    // Bottom row
    {
        const auto y = _lineOfSightOriginCell.y + aRingIndex;
        if (CELL_Y_IS_IN_BOUNDS(y)) {
            for (int x = xStart; x <= xEnd; x += 1) {
                if (CELL_X_IS_IN_BOUNDS(x)) {
                    _processCell({x, y}, aRingIndex);
                }
            }
        }
    }

#undef X_IS_IN_BOUNDS
#undef Y_IS_IN_BOUNDS

    _processedRingsBbox = {(_lineOfSightOriginCell.x - aRingIndex) * _cr,
                           (_lineOfSightOriginCell.y - aRingIndex) * _cr,
                           (aRingIndex * 2 + 1) * _cr,
                           (aRingIndex * 2 + 1) * _cr};
}

void VisibilityCalculator::_processCell(Vector2pz aCell, hg::PZInteger aRingIndex) {
    const auto spatialInfo = _world.getSpatialInfoAtUnchecked(aCell);
    if (HG_UNLIKELY_CONDITION(!spatialInfo.has_value())) {
        HG_UNLIKELY_BRANCH;
        return;
    }

    if (!spatialInfo->hasNonEmptyWallShape()) {
        return;
    }

    const auto edgesOfInterest  = _calcEdgesOfInterest(aCell);
    const bool allEdgesOverride = (aRingIndex <= 1);

    std::array<Vector2d, 8> vertices;
    const auto              vertCnt =
        GetUnobstructedVertices(*spatialInfo, aCell, edgesOfInterest, allEdgesOverride, _cr, vertices);

    if (vertCnt == 0) {
        return;
    }

    if (!_areAnyVerticesVisible(vertices, vertCnt, edgesOfInterest)) {
        return;
    }

    for (std::size_t i = 0; i < vertCnt; i += 2) {
        Vector2d ray1End, ray2End;
        AngleD   a1, a2;

        ProjectRay(_lineOfSightOrigin, vertices[i + 0], _triangleSideLength, ray1End, a1);
        ProjectRay(_lineOfSightOrigin, vertices[i + 1], _triangleSideLength, ray2End, a2);

        _triangles.push_back({vertices[i], vertices[i + 1], ray1End, edgesOfInterest});
        _triangles.push_back({ray1End, vertices[i + 1], ray2End, edgesOfInterest});

        _setRaysFromTriangles(a1, a2);
    }
}

void VisibilityCalculator::_setRaysFromTriangles(AngleD aAngle1, AngleD aAngle2) {
    if (aAngle2 < aAngle1) {
        std::swap(aAngle1, aAngle2);
    }

    const auto angleToRayIndex = [this](AngleD aAngle) -> hg::PZInteger {
        return hg::ToPz(std::round(_rayCount * (aAngle / AngleD::fullCircle())));
    };

    if (HG_UNLIKELY_CONDITION(aAngle1 <= AngleD::halfCircle() * 0.5 &&
                              aAngle2 >= AngleD::halfCircle() * 1.5)) {
        HG_UNLIKELY_BRANCH;
        // This branch deals with the unlikely case that the triangle crosses angle 0
        // (direction to the right)
        const auto start = std::max(angleToRayIndex(aAngle2), 0);
        const auto end   = std::min(angleToRayIndex(aAngle1), _rayCount - 1);
        for (hg::PZInteger i = start; i < _rayCount || i % _rayCount <= end; i += 1) {
            _rays[hg::pztos(i % _rayCount)] = _rayRadius;
        }
    } else {
        HG_LIKELY_BRANCH;
        const auto start = std::max(angleToRayIndex(aAngle1), 0);
        const auto end   = std::min(angleToRayIndex(aAngle2), _rayCount - 1);
        for (hg::PZInteger i = start; i <= end; i += 1) {
            _rays[hg::pztos(i)] = _rayRadius;
        }
    }
}

void VisibilityCalculator::_processRays() {
    for (std::size_t i = 0; i < _rays.size(); i += 1) {
        if (_rays[i] != INFINITY) {
            continue;
        }
        _castRay(hg::stopz(i));
    }
}

void VisibilityCalculator::_castRay(hg::PZInteger aRayIndex) {
    const auto direction         = AngleD::fullCircle() * aRayIndex / _rayCount;
    const auto incrementDistance = _world.getCellResolution() / _rayPointsPerCell;
    const auto incrementVector   = direction.asNormalizedVector() * incrementDistance;

    Vector2pz     prevCoords   = {};
    hg::PZInteger prevOpenness = 3; // cell with an openness of 3+ surely has no neighbouring walls

    Vector2d point = _lineOfSightOrigin + direction.asNormalizedVector() * _rayRadius;
    for (hg::PZInteger t = 0; t < _maxPointsPerRay; t += 1) {
        point += incrementVector;
        if (HG_UNLIKELY_CONDITION(point.x < 0.0 || point.y < 0.0 || point.x >= _xLimit ||
                                  point.y >= _yLimit)) {
            HG_UNLIKELY_BRANCH;
            _rays[hg::pztos(aRayIndex)] = _rayRadius + (t + 1) * incrementDistance;
            break;
        }

        const auto coords      = _world.posToCellUnchecked(point);
        const auto spatialInfo = _world.getSpatialInfoAtUnchecked(coords);

        if (HG_UNLIKELY_CONDITION(!spatialInfo.has_value())) {
            HG_UNLIKELY_BRANCH;
            _rays[hg::pztos(aRayIndex)] = _rayRadius + (t + 1) * incrementDistance;
            break;
        }

        const auto openness = spatialInfo->openness;

        if (openness < 3 && prevOpenness < 3 && coords.x != prevCoords.x && coords.y != prevCoords.y) {
            const auto diagonalNeighbour1SpatialInfo =
                _world.getSpatialInfoAtUnchecked({coords.x, prevCoords.y});
            const auto diagonalNeighbour2SpatialInfo =
                _world.getSpatialInfoAtUnchecked({prevCoords.x, coords.y});

            // clang-format off
            if ((!diagonalNeighbour1SpatialInfo || diagonalNeighbour1SpatialInfo->hasNonEmptyWallShape()) &&
                (!diagonalNeighbour2SpatialInfo || diagonalNeighbour2SpatialInfo->hasNonEmptyWallShape())) {
                _rays[hg::pztos(aRayIndex)] = _rayRadius + t * incrementDistance;
                return;
            }
            // clang-format on
        }
        prevCoords   = coords;
        prevOpenness = openness;

        if (spatialInfo->hasNonEmptyWallShape()) {
            _rays[hg::pztos(aRayIndex)] = _rayRadius + (t + 1) * incrementDistance;
            return;
        }
    }
}

bool VisibilityCalculator::_isPointVisible(PositionInWorld aPosInWorld,
                                           std::uint8_t    aFlags,
                                           bool            aLazy) const {
    if (_rayCheckingEnabled && !_processedRingsBbox.overlaps(*aPosInWorld)) {
        const double   dist  = hg::math::EuclideanDist(*aPosInWorld, _lineOfSightOrigin);
        const Vector2d diff  = {aPosInWorld->x - _lineOfSightOrigin.x,
                                aPosInWorld->y - _lineOfSightOrigin.y};
        const auto     angle = AngleD::fromVector(diff.x, diff.y);
        const auto     idx =
            static_cast<std::size_t>(std::round(_rayCount * (angle / AngleD::fullCircle())));
        if (dist > _rays[idx]) {
            return false;
        }
    }

    if (aLazy) {
        const hg::math::Vector2i cell = {static_cast<int>(aPosInWorld->x / _cr),
                                         static_cast<int>(aPosInWorld->y / _cr)};
        aFlags                        = _calcEdgesOfInterest(cell);
    }

    const auto limit = _triangles.size();
    for (std::int64_t i = 0; i < limit; i += 1) {
        // Explanation for the bit magic:
        //   Triangles have the following edge flags, depending on their position relative to the
        //   central cell:
        //
        //         |   |
        //      TL |TLR| TR
        //   ------|---|------
        //     LTB | * | RTB
        //   ------|---|------
        //      BL |BLR| BR
        //         |   |
        //
        //   For two triangles to potentially intersect, they have to have at least two flags in
        //   common. So we bitwise-and the two triangles' masks and check if at least 2 bits are
        //   set - or, precisely, we check the opposite - if only 1 or 0 bits are set. Such numbers
        //   have an interesting property - if you bitwise-and them with themselves minus 1, they
        //   produce 0 (0 because it has no set bits; ..0001000.. - 1 => ..0000111..; and so on).
        //
        //   Note: I tried std::popcnt, it was too slow.
        const auto mask = _triangles[static_cast<std::size_t>(i)].flags & aFlags;
        if (HG_LIKELY_CONDITION((mask & (mask - 1)) == 0)) {
            HG_LIKELY_BRANCH;
            continue;
        }
        if (_calcOngoing) {
            _stats.triangleCheckCount += 1;
        }
        if (hg::math::IsPointInsideTriangle(*aPosInWorld, _triangles[static_cast<std::size_t>(i)])) {
            return false;
        }
    }

    return true;
}

bool VisibilityCalculator::_isLineVisible(PositionInWorld aP1,
                                          PositionInWorld aP2,
                                          std::uint8_t    aFlags,
                                          hg::PZInteger   aLevels) const {
    if (aLevels <= 0) {
        return false;
    }

    const auto p = (*aP1 + *aP2) / 2.0;

    if (_isPointVisible(PositionInWorld{p}, aFlags, false)) {
        return true;
    }

    if (aLevels <= 1) {
        return false;
    } else {
        return (_isLineVisible(aP1, PositionInWorld{p}, aFlags, aLevels - 1) ||
                _isLineVisible(PositionInWorld{p}, aP2, aFlags, aLevels - 1));
    }
}

} // namespace gridgoblin
} // namespace jbatnozic

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/Math.hpp>

#include <vector>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = jbatnozic::hobgoblin;

struct VisibilityCalculatorConfig {
    //! Minimal number of rings around point of view that have to be resolved in high detail
    //! before the calculator can fall back to lower detail mode (ray casting).
    //!
    //! \note set this to 0 to force resolving everything in high detail.
    hg::PZInteger minRingsBeforeRaycasting = 15;

    //! Minimal number of triangles that has to be resolved before the calculator can fall
    //! back to lower detail mode (ray casting).
    hg::PZInteger minTrianglesBeforeRaycasting = 100;

    //! Should the calculator fall back to lower detail mode - ray casting - this is the total number
    //! of rays that will be evenly distributed in a circle around the point of view.
    hg::PZInteger rayCount = 360;

    //! TODO(description)
    hg::PZInteger rayPointsPerCell = 6;
};

//! Implements the `VisibilityProvider` using math/geometry formulas fully calculated on the CPU.
class VisibilityCalculator : public VisibilityProvider {
public:
    VisibilityCalculator(const World& aWorld, const VisibilityCalculatorConfig& aConfig = {});

    void calc(PositionInWorld aViewCenter, hg::math::Vector2d aViewSize, PositionInWorld aPointOfView);

    struct CalculationStats {
        //! Number of rings that were resolved in high detail.
        hg::PZInteger highDetailRingCount;

        //! Number of triangles that were resolved.
        hg::PZInteger triangleCount;

        //! Number of checks that were performed to see if a point is inside a triangle.
        hg::PZInteger triangleCheckCount;
    };

    //! Get some stats from the latest calc() call.
    const CalculationStats& getStats() const;

    std::optional<bool> testVisibilityAt(PositionInWorld aPos) const override;

    // TODO: render()

private:
    // ===== Dependencies =====

    const World& _world;

    // ===== Configuration =====

    const double _cr;     //!< Cell resolution (known from _world).
    const double _xLimit; //!< Maximum for X values (known from _world).
    const double _yLimit; //!< Maximum for Y values (known from _world).

    hg::PZInteger _minRingsBeforeRaycasting;
    hg::PZInteger _minTrianglesBeforeRaycasting;
    hg::PZInteger _rayCount;
    hg::PZInteger _rayPointsPerCell;

    // ===== Calculation context =====

    hg::math::Rectangle<double> _processedRingsBbox;

    hg::math::Rectangle<double> _viewBbox;
    hg::math::Vector2pz         _viewTopLeftCell;
    hg::math::Vector2pz         _viewBottomRightCell;

    hg::math::Vector2d  _lineOfSightOrigin;
    hg::math::Vector2pz _lineOfSightOriginCell;

    double        _triangleSideLength;
    double        _rayRadius;
    hg::PZInteger _maxPointsPerRay;
    bool          _rayCheckingEnabled;

    // ===== Data structures =====

    struct Triangle : public hg::math::TriangleD {
        Triangle(hg::math::Vector2d aA,
                 hg::math::Vector2d aB,
                 hg::math::Vector2d aC,
                 std::uint16_t      aFlags)
            : hg::math::TriangleD{aA, aB, aC}
            , flags{aFlags} {}

        std::uint16_t flags;
    };

    std::vector<Triangle> _triangles;

    std::vector<double> _rays;

    // ===== Statistics =====

    bool _calcOngoing = false;

    mutable CalculationStats _stats;

    // ===== Methods =====

    void _resetData();

    void _setInitialCalculationContext(PositionInWorld    aViewCenter,
                                       hg::math::Vector2d aViewSize,
                                       PositionInWorld    aLineOfSightOrigin);

    std::uint8_t _calcEdgesOfInterest(hg::math::Vector2pz aCell) const;

    bool _areAnyVerticesVisible(const std::array<hg::math::Vector2d, 8>& aVertices,
                                std::size_t                              aVertCount,
                                std::uint16_t                            aEdgesOfInterest) const;

    void _processRing(hg::PZInteger aRingIndex);

    void _processCell(hg::math::Vector2pz aCell, hg::PZInteger aRingIndex);

    void _setRaysFromTriangles(hg::math::AngleD aAngle1, hg::math::AngleD aAngle2);

    void _processRays();

    void _castRay(hg::PZInteger aRayIndex);

    bool _isPointVisible(PositionInWorld aPosInWorld, std::uint16_t aFlags) const;

    bool _isLineVisible(PositionInWorld aP1,
                        PositionInWorld aP2,
                        std::uint16_t   aFlags,
                        hg::PZInteger   aLevels) const;
};

} // namespace gridgoblin
} // namespace jbatnozic

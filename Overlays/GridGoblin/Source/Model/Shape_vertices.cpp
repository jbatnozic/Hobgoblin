// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Model/Shape.hpp>
#include <GridGoblin/Model/Shape_vertices.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <array>
#include <cstdint>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

template <Shape taShape>
std::size_t GetVisibilityVertices(cell::SpatialInfo                  aCellSpatialInfo,
                                  std::uint8_t                       aEdgesOfInterest,
                                  bool                               aAllEdgesOverride,
                                  double                             aPaddingOffset,
                                  std::array<hg::math::Vector2d, 8>& aVertices) {
    return 0; // Dummy
}

#define GG_EAST(_flags_)  (((_flags_) & cell::OBSTRUCTED_FULLY_BY_EAST_NEIGHBOR) == 0)
#define GG_NORTH(_flags_) (((_flags_) & cell::OBSTRUCTED_FULLY_BY_NORTH_NEIGHBOR) == 0)
#define GG_WEST(_flags_)  (((_flags_) & cell::OBSTRUCTED_FULLY_BY_WEST_NEIGHBOR) == 0)
#define GG_SOUTH(_flags_) (((_flags_) & cell::OBSTRUCTED_FULLY_BY_SOUTH_NEIGHBOR) == 0)

// MARK: FULL_SQUARE

template <>
std::size_t GetVisibilityVertices<Shape::FULL_SQUARE>(cell::SpatialInfo aCellSpatialInfo,
                                                      std::uint8_t      aEdgesOfInterest,
                                                      bool              aAllEdgesOverride,
                                                      double            aPaddingOffset,
                                                      std::array<hg::math::Vector2d, 8>& aVertices) {
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::FULL_SQUARE | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices<Shape::FULL_SQUARE>(aCellSpatialInfo,
                                                     aEdgesOfInterest,
                                                     aAllEdgesOverride,
                                                     aPaddingOffset,
                                                     aVertices);
}

template <>
std::size_t GetVisibilityVertices<Shape::FULL_SQUARE | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices<Shape::FULL_SQUARE>(aCellSpatialInfo,
                                                     aEdgesOfInterest,
                                                     aAllEdgesOverride,
                                                     aPaddingOffset,
                                                     aVertices);
}

template <>
std::size_t GetVisibilityVertices<Shape::FULL_SQUARE | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices<Shape::FULL_SQUARE>(aCellSpatialInfo,
                                                     aEdgesOfInterest,
                                                     aAllEdgesOverride,
                                                     aPaddingOffset,
                                                     aVertices);
}

// MARK: LARGE_TRIANGLE

template <>
std::size_t GetVisibilityVertices<Shape::LARGE_TRIANGLE>(cell::SpatialInfo aCellSpatialInfo,
                                                         std::uint8_t      aEdgesOfInterest,
                                                         bool              aAllEdgesOverride,
                                                         double            aPaddingOffset,
                                                         std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::LARGE_TRIANGLE | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::LARGE_TRIANGLE | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::LARGE_TRIANGLE | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags) || GG_WEST(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

// MARK: SMALL_TRIANGLE_HOR

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_HOR>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_WEST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 0.5 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 0.5 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_HOR | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 0.5 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_EAST(flags) || GG_WEST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 0.5 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_HOR | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_WEST(flags) || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.5 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.5 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_HOR | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.5 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags) || GG_EAST(flags) || GG_WEST(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.5 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

// MARK: TALL_SMALL_TRIANGLE_HOR

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_HOR>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 0.5 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 0.5 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_HOR | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 0.5 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 0.5 - aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_HOR | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.5 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_EAST(flags) || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.5 + aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_HOR | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags) || GG_WEST(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.5 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.5 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

// MARK: HALF_SQUARE_HOR

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_HOR>(cell::SpatialInfo aCellSpatialInfo,
                                                          std::uint8_t      aEdgesOfInterest,
                                                          bool              aAllEdgesOverride,
                                                          double            aPaddingOffset,
                                                          std::array<hg::math::Vector2d, 8>& aVertices) {
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 0.5 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 0.5 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_EAST(flags) || GG_WEST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 0.5};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 0.5};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_HOR | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices<Shape::HALF_SQUARE_HOR>(aCellSpatialInfo,
                                                         aEdgesOfInterest,
                                                         aAllEdgesOverride,
                                                         aPaddingOffset,
                                                         aVertices);
}

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_HOR | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.5 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_EAST(flags) || GG_WEST(flags) || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.5};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.5};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.5 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_HOR | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices< Shape::HALF_SQUARE_HOR | Shape::VFLIP >(aCellSpatialInfo,
                                                                          aEdgesOfInterest,
                                                                          aAllEdgesOverride,
                                                                          aPaddingOffset,
                                                                          aVertices);
}

// MARK: SMALL_TRIANGLE_VER

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_VER>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_NORTH(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {0.5 + aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {0.5 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_VER | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.5 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags) || GG_NORTH(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.5 - aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_VER | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_NORTH(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.5 + aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {0.5 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::SMALL_TRIANGLE_VER | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags) || GG_SOUTH(flags) || GG_WEST(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.5 - aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.5 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

// MARK: TALL_SMALL_TRIANGLE_VER

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_VER>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.5 - aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {0.5 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_VER | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags) || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.5 + aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.5 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_VER | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags) || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {0.5 - aPaddingOffset, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {0.5 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::TALL_SMALL_TRIANGLE_VER | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.5 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags) || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.5 + aPaddingOffset, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

// MARK: HALF_SQUARE_VER

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_VER>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_NORTH(flags) || GG_SOUTH(flags) || GG_EAST(flags)) {
        aVertices[cnt + 0] = {0.5, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {0.5, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {0.5 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.0 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.0, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.0, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.0 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {0.5 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_VER | Shape::HFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    std::size_t cnt = 0;

    const auto flags = aCellSpatialInfo.obFlags | aEdgesOfInterest;

    if (aAllEdgesOverride || GG_EAST(flags)) {
        aVertices[cnt + 0] = {1.0, 1.0 + aPaddingOffset};
        aVertices[cnt + 1] = {1.0, 0.0 - aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags)) {
        aVertices[cnt + 0] = {1.0 + aPaddingOffset, 0.0};
        aVertices[cnt + 1] = {0.5 - aPaddingOffset, 0.0};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_NORTH(flags) || GG_SOUTH(flags) || GG_WEST(flags)) {
        aVertices[cnt + 0] = {0.5, 0.0 - aPaddingOffset};
        aVertices[cnt + 1] = {0.5, 1.0 + aPaddingOffset};
        cnt += 2;
    }
    if (aAllEdgesOverride || GG_SOUTH(flags)) {
        aVertices[cnt + 0] = {0.5 - aPaddingOffset, 1.0};
        aVertices[cnt + 1] = {1.0 + aPaddingOffset, 1.0};
        cnt += 2;
    }

    return cnt;
}

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_VER | Shape::VFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices<Shape::HALF_SQUARE_VER>(aCellSpatialInfo,
                                                         aEdgesOfInterest,
                                                         aAllEdgesOverride,
                                                         aPaddingOffset,
                                                         aVertices);
}

template <>
std::size_t GetVisibilityVertices<Shape::HALF_SQUARE_VER | Shape::HVFLIP>(
    cell::SpatialInfo                  aCellSpatialInfo,
    std::uint8_t                       aEdgesOfInterest,
    bool                               aAllEdgesOverride,
    double                             aPaddingOffset,
    std::array<hg::math::Vector2d, 8>& aVertices) //
{
    return GetVisibilityVertices< Shape::HALF_SQUARE_VER | Shape::HFLIP >(aCellSpatialInfo,
                                                                          aEdgesOfInterest,
                                                                          aAllEdgesOverride,
                                                                          aPaddingOffset,
                                                                          aVertices);
}

// MARK: TABLE

namespace {
using GetVisibilityVerticesFunc = decltype(&GetVisibilityVertices<Shape::FULL_SQUARE>);

std::array<GetVisibilityVerticesFunc, MAX_SHAPE_NUMBER + 1> MakeVisibilityVerticesFuncTable() {
    std::array<GetVisibilityVerticesFunc, MAX_SHAPE_NUMBER + 1> result;

#define ADD_ENTRY(_shape_) result[static_cast<std::size_t>(_shape_)] = &GetVisibilityVertices<_shape_>

    ADD_ENTRY(Shape::FULL_SQUARE);
    ADD_ENTRY(Shape::FULL_SQUARE | Shape::HFLIP);
    ADD_ENTRY(Shape::FULL_SQUARE | Shape::VFLIP);
    ADD_ENTRY(Shape::FULL_SQUARE | Shape::HVFLIP);
    ADD_ENTRY(Shape::LARGE_TRIANGLE);
    ADD_ENTRY(Shape::LARGE_TRIANGLE | Shape::HFLIP);
    ADD_ENTRY(Shape::LARGE_TRIANGLE | Shape::VFLIP);
    ADD_ENTRY(Shape::LARGE_TRIANGLE | Shape::HVFLIP);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_HOR);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_HOR | Shape::HFLIP);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_HOR | Shape::VFLIP);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_HOR | Shape::HVFLIP);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_HOR);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_HOR | Shape::HFLIP);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_HOR | Shape::VFLIP);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_HOR | Shape::HVFLIP);
    ADD_ENTRY(Shape::HALF_SQUARE_HOR);
    ADD_ENTRY(Shape::HALF_SQUARE_HOR | Shape::HFLIP);
    ADD_ENTRY(Shape::HALF_SQUARE_HOR | Shape::VFLIP);
    ADD_ENTRY(Shape::HALF_SQUARE_HOR | Shape::HVFLIP);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_VER);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_VER | Shape::HFLIP);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_VER | Shape::VFLIP);
    ADD_ENTRY(Shape::SMALL_TRIANGLE_VER | Shape::HVFLIP);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_VER);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_VER | Shape::HFLIP);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_VER | Shape::VFLIP);
    ADD_ENTRY(Shape::TALL_SMALL_TRIANGLE_VER | Shape::HVFLIP);
    ADD_ENTRY(Shape::HALF_SQUARE_VER);
    ADD_ENTRY(Shape::HALF_SQUARE_VER | Shape::HFLIP);
    ADD_ENTRY(Shape::HALF_SQUARE_VER | Shape::VFLIP);
    ADD_ENTRY(Shape::HALF_SQUARE_VER | Shape::HVFLIP);

#undef ADD_ENTRY

    return result;
}

const auto VISIBILITY_VERTICES_FUNC_TABLE = MakeVisibilityVerticesFuncTable();
} // namespace

std::size_t GetVisibilityVertices(cell::SpatialInfo                  aCellSpatialInfo,
                                  std::uint8_t                       aEdgesOfInterest,
                                  bool                               aAllEdgesOverride,
                                  double                             aPaddingOffset,
                                  std::array<hg::math::Vector2d, 8>& aVertices) //
{
    HG_ASSERT(aCellSpatialInfo.hasNonEmptyWallShape());

    const auto funcIdx = static_cast<std::size_t>(aCellSpatialInfo.wallShape);
    HG_ASSERT(funcIdx < VISIBILITY_VERTICES_FUNC_TABLE.size());

    const auto func = VISIBILITY_VERTICES_FUNC_TABLE[funcIdx];
    return func(aCellSpatialInfo, aEdgesOfInterest, aAllEdgesOverride, aPaddingOffset, aVertices);
}

} // namespace gridgoblin
} // namespace jbatnozic

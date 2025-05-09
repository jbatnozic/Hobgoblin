// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Rendering/Drawing_order.hpp>
#include <GridGoblin/Spatial/Spatial_info.hpp>

#include <gtest/gtest.h>

#include <array>

namespace jbatnozic {
namespace gridgoblin {

TEST(GridGoblinSpatialInfoDrawingOrderTest, FourSeparateRectangles) {
    std::array<SpatialInfo, 4> infos;

    infos[0].setTopLeft({6.5f, 1.f});
    infos[0].setSize({1.5f, 1.5f});

    infos[1].setTopLeft({2.f, 3.f});
    infos[1].setSize({4.f, 2.f});

    infos[2].setTopLeft({4.5f, 7.f});
    infos[2].setSize({2.f, 3.f});

    infos[3].setTopLeft({1.f, 6.f});
    infos[3].setSize({3.f, 3.f});

    for (unsigned i = 0; i < 4; i += 1) {
        for (unsigned t = i + 1; t < 4; t += 1) {
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[i], infos[t]), dimetric::DRAW_LHS_FIRST);
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[t], infos[i]), dimetric::DRAW_RHS_FIRST);
        }
    }
}

TEST(GridGoblinSpatialInfoDrawingOrderTest, TouchingRectanglesCycle) {
    std::array<SpatialInfo, 4> infos;

    infos[0].setTopLeft({2.0f, 0.f});
    infos[0].setSize({6.f, 2.f});

    infos[1].setTopLeft({6.f, 2.f});
    infos[1].setSize({2.f, 6.f});

    infos[2].setTopLeft({0.f, 0.f});
    infos[2].setSize({2.f, 6.f});

    infos[3].setTopLeft({0.f, 6.f});
    infos[3].setSize({6.f, 2.f});

    for (unsigned i = 0; i < 4; i += 1) {
        for (unsigned t = i + 1; t < 4; t += 1) {
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[i], infos[t]), dimetric::DRAW_LHS_FIRST);
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[t], infos[i]), dimetric::DRAW_RHS_FIRST);
        }
    }
}

// TODO(doesn't matter - test)

TEST(GridGoblinSpatialInfoDrawingOrderTest, CircleOrbitingAnotherCircle) {
    // SpatialInfos are technically rectangles, not circles, but we pretend they are
    // circles with R=2, and we set the spatial info params according to the bounding
    // boxes of those circles.
    // Pivot orbits the axis with a distance of 4 between their centres.

    const float R       = 2.f; // circle radius
    const float centreX = 8.f;
    const float centreY = 8.f;

    SpatialInfo axis;
    axis.setCenter({centreX, centreX});
    axis.setSizeMaintainingCenter({2.f * R, 2.f * R});

    SpatialInfo pivot;
    pivot.setSizeMaintainingCenter({2.f * R, 2.f * R});

    for (int i = 0; i < 360; i += 1) {
        const auto  angle = hg::math::AngleF::fromDegrees(static_cast<float>(i - 45));
        const float x     = centreX + angle.cos() * 4.f;
        const float y     = centreY - angle.sin() * 4.f;
        pivot.setCenter({x, y});

        const auto order = dimetric::CheckDrawingOrder(pivot, axis);
        if (i == 0 || i == 180) {
            // It's indeterminate what happens in this case (and also doesn't matter)
            continue;
        } else if (i <= 180) {
            EXPECT_EQ(order, dimetric::DRAW_LHS_FIRST) << "angle = " << i;
        } else {
            EXPECT_EQ(order, dimetric::DRAW_RHS_FIRST) << "angle = " << i;
        }
    }
}

} // namespace gridgoblin
} // namespace jbatnozic

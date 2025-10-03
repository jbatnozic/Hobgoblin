// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Positional/Bounds_info.hpp>
#include <GridGoblin/Rendering/Drawing_order.hpp>

#include <gtest/gtest.h>

#include <array>

namespace jbatnozic {
namespace gridgoblin {

TEST(GridGoblinBoundsInfoDrawingOrderTest, FourSeparateRectangles) {
    std::array<BoundsInfo, 4> infos;

    infos[0].setTopLeft({6.5, 1.0});
    infos[0].setSize({1.5, 1.5});

    infos[1].setTopLeft({2.0, 3.0});
    infos[1].setSize({4.0, 2.0});

    infos[2].setTopLeft({4.5, 7.0});
    infos[2].setSize({2.0, 3.0});

    infos[3].setTopLeft({1.0, 6.0});
    infos[3].setSize({3.0, 3.0});

    for (unsigned i = 0; i < 4; i += 1) {
        for (unsigned t = i + 1; t < 4; t += 1) {
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[i], infos[t]), dimetric::DRAW_LHS_FIRST);
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[t], infos[i]), dimetric::DRAW_RHS_FIRST);
        }
    }
}

TEST(GridGoblinBoundsInfoDrawingOrderTest, TouchingRectanglesCycle) {
    std::array<BoundsInfo, 4> infos;

    infos[0].setTopLeft({2.0, 0.0});
    infos[0].setSize({6.0, 2.0});

    infos[1].setTopLeft({6.0, 2.0});
    infos[1].setSize({2.0, 6.0});

    infos[2].setTopLeft({0.0, 0.0});
    infos[2].setSize({2.0, 6.0});

    infos[3].setTopLeft({0.0, 6.0});
    infos[3].setSize({6.0, 2.0});

    for (unsigned i = 0; i < 4; i += 1) {
        for (unsigned t = i + 1; t < 4; t += 1) {
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[i], infos[t]), dimetric::DRAW_LHS_FIRST);
            EXPECT_EQ(dimetric::CheckDrawingOrder(infos[t], infos[i]), dimetric::DRAW_RHS_FIRST);
        }
    }
}

// TODO(doesn't matter - test)

TEST(GridGoblinBoundsInfoDrawingOrderTest, CircleOrbitingAnotherCircle) {
    // BoundsInfos are technically rectangles, not circles, but we pretend they are
    // circles with R=2, and we set the spatial info params according to the bounding
    // boxes of those circles.
    // Pivot orbits the axis with a distance of 4 between their centres.

    const double R       = 2.0; // circle radius
    const double centreX = 8.0;
    const double centreY = 8.0;

    BoundsInfo axis;
    axis.setCenter({centreX, centreX});
    axis.setSizeMaintainingCenter({2.0 * R, 2.0 * R});

    BoundsInfo pivot;
    pivot.setSizeMaintainingCenter({2.0 * R, 2.0 * R});

    for (int i = 0; i < 360; i += 1) {
        const auto   angle = hg::math::AngleD::fromDegrees(static_cast<double>(i - 45));
        const double x     = centreX + angle.cos() * 4.0;
        const double y     = centreY - angle.sin() * 4.0;
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

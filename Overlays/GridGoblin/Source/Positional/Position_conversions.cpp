// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Positional/Position_conversions.hpp>

namespace jbatnozic {
namespace gridgoblin {

namespace dimetric {

PositionInView ToPositionInView(PositionInWorld aPosInWorld) {
    PositionInView result;
    // clang-format off
    result->x =  aPosInWorld->y + aPosInWorld->x;
    result->y = (aPosInWorld->y - aPosInWorld->x) / 2.f;
    // clang-format on
    return result;
}

PositionInWorld ToPositionInWorld(PositionInView aPosInView) {
    PositionInWorld result;
    result->x = aPosInView->x / 2.f - aPosInView->y;
    result->y = aPosInView->x / 2.f + aPosInView->y;
    return result;
}

} // namespace dimetric

namespace topdown {

PositionInView ToPositionInView(PositionInWorld aPosInWorld) {
    return PositionInView{aPosInWorld->x, aPosInWorld->y};
}

PositionInWorld ToPositionInWorld(PositionInView aPosInView) {
    return PositionInWorld{aPosInView->x, aPosInView->y};
}

} // namespace topdown

} // namespace gridgoblin
} // namespace jbatnozic

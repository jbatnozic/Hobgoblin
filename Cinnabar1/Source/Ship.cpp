// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Ship.hpp>

namespace cinnabar {

Ship::Ship(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard, QAO_ExeCon::GAMEPLAY, 5, "cinnabar::Ship"} {}

void Ship::init(double aX, double aY) {}

void Ship::_eventUpdate1() {}

void Ship::_eventDraw1() {}

} // namespace cinnabar

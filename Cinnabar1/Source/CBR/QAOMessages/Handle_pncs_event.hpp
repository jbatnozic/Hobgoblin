// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/QAO.hpp>

#include <cstdint>

namespace cinnabar {

// PNCS = Point n' Click System

struct PNCSEventData {
    bool          mbLeftDown  = false; // a.k.a. left click press
    bool          mbRightDown = false; // a.k.a. right click press
    bool          mbLeftUp    = false; // a.k.a. left click release
    bool          mbRightUp   = false; // a.k.a. right click release
    std::intptr_t userData    = 0;
};

QAO_DEFINE_MESSAGE(HandlePNCSEvent, PNCSEventData*);

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/QAO.hpp>

namespace cinnabar {

class ShipController;

QAO_DEFINE_MESSAGE(DowncastToShipController, ShipController**);

} // namespace cinnabar

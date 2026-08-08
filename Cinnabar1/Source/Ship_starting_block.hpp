// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <functional>

#include <Ship_attachable.hpp>

namespace cinnabar {

class ShipStartingBlock : public spe::StateObject, public UnibodyShipAttachable {
public:
    ShipStartingBlock(QAO_InstGuard aInstGuard);

private:
    PolyShape                    _initPolyShape();
    PhysicalProperties           _initPhysicalProperties();
    hg::alvin::CollisionDelegate _initColDelegate();

    // void _event
};

inline ShipStartingBlock::ShipStartingBlock(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       0, /* TODO */
                       QAO_STATIC_NAME("cinnabar::ShipStartingBlock")}
    // clang-format off
    , UnibodyShipAttachable{
        std::bind(&ShipStartingBlock::_initPolyShape, this),
        std::bind(&ShipStartingBlock::_initPhysicalProperties, this),
        std::bind(&ShipStartingBlock::_initColDelegate, this),
        std::bind(&ShipStartingBlock::_alvinBodyFromPhysicalPropertiesAndPolyShape, this),
        std::bind(&ShipStartingBlock::_alvinShapeFromPolyShape, this)
    }
    // clang-format on
{
    // _unibody.bindDelegate(taEntity &aEntity)
}

} // namespace cinnabar

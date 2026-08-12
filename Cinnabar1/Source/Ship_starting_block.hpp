// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Ship_attachable.hpp>
#include <Overworld_collisions.hpp>

namespace cinnabar {

class ShipStartingBlock
    : public spe::StateObject
    , public UnibodyShipAttachable
    , public ovwcol::JunkEntity {
public:
    ShipStartingBlock(QAO_InstGuard aInstGuard);

    void init(hg::math::Vector2d aPosition);

    void drawAsGhost(
        const hg::math::Vector2d& aPosition,
        hg::math::AngleF          aAngle,
        uwga::Color               aColor,
        uwga::Canvas&             aCanvas,
        const uwga::RenderStates& aRenderStates = uwga::RENDER_STATES_DEFAULT) const override {}

private:
    PolyShape                    _initPolyShape();
    PhysicalProperties           _initPhysicalProperties();
    hg::alvin::CollisionDelegate _initColDelegate();

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1() override;
    void _eventUpdate2() override;
    void _eventDraw1() override;

    void _applyPropulsion(const spe::WindowFrameInputView& aInput);
};

QAO_REGISTER_CLASS(ShipStartingBlock, cinnabar_ShipStartingBlock) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
}

} // namespace cinnabar

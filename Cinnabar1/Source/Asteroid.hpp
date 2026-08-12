// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Ship_attachable.hpp>
#include <Overworld_collisions.hpp>
#include <QAOMessages/Downcast_to_ship_attachable.hpp>
#include <QAOMessages/Handle_pncs_event.hpp>

namespace cinnabar {

class Asteroid
    : public spe::StateObject
    , public UnibodyShipAttachable
    , public ovwcol::JunkEntity {
public:
    Asteroid(QAO_InstGuard aInstGuard);

    void init(hg::math::Vector2d aPosition);

    void drawAsGhost(
        const hg::math::Vector2d& aPosition,
        hg::math::AngleF          aAngle,
        uwga::Color               aColor,
        uwga::Canvas&             aCanvas,
        const uwga::RenderStates& aRenderStates = uwga::RENDER_STATES_DEFAULT) const override {}

    // QAO Message Handlers

    void msgDowncastToShipAttachable(DowncastToShipAttachable::PayloadPtr aPtr, bool /* aConst */);

    void msgHandlePNCSEvent(HandlePNCSEvent::PayloadPtr aPayload, bool /* aConst */);

private:
    bool _leftClicked = false;

    PolyShape                    _initPolyShape();
    PhysicalProperties           _initPhysicalProperties();
    hg::alvin::CollisionDelegate _initColDelegate();

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventBeginUpdate() override;
    void _eventUpdate1() override;
    void _eventUpdate2() override;
    void _eventDraw1() override;
};

QAO_REGISTER_CLASS(Asteroid, cinnabar_Asteroid) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
    klass.setMessageHandler<C, DowncastToShipAttachable, &C::msgDowncastToShipAttachable>();
    klass.setMessageHandler<C, HandlePNCSEvent, &C::msgHandlePNCSEvent>();
}

} // namespace cinnabar

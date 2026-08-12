// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Poly_shape.hpp>
#include <Ship_attachable.hpp>
#include <QAOMessages/Handle_pncs_event.hpp>

#include <Hobgoblin/Math.hpp>

namespace cinnabar {

class AttachableGhost : public spe::StateObject {
public:
    AttachableGhost(QAO_InstGuard aInstGuard);

    void init(/* TODO: ship controller of origin, */ QAO_GenericId aAttachableId);

    void msgHandlePNCSEvent(HandlePNCSEvent::PayloadPtr aPayload, bool /* aConst */);

private:
    QAO_GenericId   _attachableId  = nullptr;
    ShipAttachable* _attachablePtr = nullptr;

    PolyShape _shape;

    bool _leftClicked = false;

    bool               _held         = false;
    hg::math::Vector2d _cursorOffset = {};

    bool               _shift          = false;
    hg::math::Vector2d _shiftCursorPos = {};

    void _eventBeginUpdate() override;
    void _eventUpdate1() override;
    void _eventDraw1() override;

    ShipAttachable* _findAttachableById(QAO_GenericId aAttachableId) const;
};

QAO_REGISTER_CLASS(AttachableGhost, cinnabar_AttachableGhost) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
    klass.setMessageHandler<C, HandlePNCSEvent, &C::msgHandlePNCSEvent>();
}

} // namespace cinnabar

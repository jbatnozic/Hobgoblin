// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Attachable_ghost.hpp>

#include <Interactivity_manager.hpp>
#include <QAOMessages/Downcast_to_ship_attachable.hpp>
#include <Ship_controller.hpp>

#include <Hobgoblin/ChipmunkPhysics.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <span>

namespace cinnabar {

AttachableGhost::AttachableGhost(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard,
                       QAO_ExeCon::GAMEPLAY,
                       PRIORITY_ENTITIES,
                       QAO_STATIC_NAME("cinnabar::AttachableGhost")} {}

void AttachableGhost::init(QAO_GenericId aAttachableId) {
    _attachablePtr = _findAttachableById(aAttachableId);
    _attachableId  = aAttachableId;

    _shape = _attachablePtr->getPolyShape();

    _held = true;

    const auto input       = ccomp<MWindow>().getInput();
    const auto mousePos    = input.getViewRelativeMousePos();
    const auto mousePosRel = mousePos - _shape.getAnchor();

    _cursorOffset = mousePosRel;
}

void AttachableGhost::msgHandlePNCSEvent(HandlePNCSEvent::PayloadPtr aPayload, bool /* aConst */) {
    HG_ASSERT(aPayload != nullptr);
    if (aPayload->mbRightDown) {
        QAO_Destroy(*this);
        return;
    }
    if (aPayload->mbLeftDown) {
        _leftClicked = true;
    }
}

void AttachableGhost::_eventBeginUpdate() {
    _leftClicked = false;

    ccomp<MInteractivity>().pushClickableObject(
        this->getId(),
        _held ? -9999 : 0,
        0,
        /* quick check */
        [this](hg::math::Vector2d aMouseWorldPos) -> bool {
            return _held || ((aMouseWorldPos - _shape.getAnchor()).lengthSquared() <=
                             _shape.getDistanceToFarthestRawVertexSquared());
        },
        /* full check */
        [this](hg::math::Vector2d aMouseWorldPos) -> bool {
            return _held || _shape.intersectsWithPointRel(aMouseWorldPos - _shape.getAnchor());
        });
}

void AttachableGhost::_eventUpdate1() {
    auto* runtime = getRuntime();
    if (runtime->find(_attachableId).isNull()) {
        auto handle = hg::MoveToUnderlying(runtime->detachObject(getId()));
        HG_ASSERT(handle.isOwning());
        handle.reset(); // Destroy self
        return;
    }

    const auto& winMgr      = ccomp<MWindow>();
    const auto  input       = winMgr.getInput();
    const auto  mousePos    = input.getViewRelativeMousePos();
    const auto  mousePosRel = mousePos - _shape.getAnchor();

    if (_leftClicked) {
        if (_shape.intersectsWithPointRel(mousePosRel)) {
            if (_held) {
                _held = false;
            } else {
                _cursorOffset = mousePosRel;
                _held         = true;
            }
        } else {
            _held = false;
        }
        _shift = false;
    }

    if (_held) {
        if (input.checkPressed(hg::in::PK_LSHIFT)) {
            if (!_shift) {
                _shiftCursorPos = mousePos;
                _shift          = true;
            }
        } else {
            if (_shift) {
                _cursorOffset = mousePosRel;
                _shift        = false;
            }
        }

        bool needRecalc = false;

        if (auto wheelScroll = input.getVerticalMouseWheelScroll(); wheelScroll != 0.f) {
            _shape.setRotation(_shape.getRotation() + hg::math::AngleF::fromDeg(wheelScroll));
            needRecalc = true;
        }

        if (_shift) {
            const auto shiftScroll = static_cast<float>(_shiftCursorPos.x - mousePos.x) * 0.25;
            if (shiftScroll > 0.1f) {
                _shape.setRotation(_shape.getRotation() + hg::math::AngleF::fromDegrees(shiftScroll));
                _shiftCursorPos = mousePos;
                needRecalc      = true;
            }
        } else {
            _shape.setAnchor(mousePos - _cursorOffset);
        }

        _shape.recalcRel();
    }
}

void AttachableGhost::_eventDraw1() {
    const auto vertCount = _shape.getVertexCount();

    // Draw the AttachableGhost itself
    uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLE_FAN, vertCount + 2, _shape.getAnchor()};

    vArr.vertices[0].position = {};
    vArr.vertices[0].color    = uwga::COLOR_GREY;

    const auto outputVerts = _shape.getOutputVertices();

    for (std::size_t i = 0; i < hg::pztos(vertCount); ++i) {
        vArr.vertices[i + 1].position = outputVerts[i].cast<float>();
        vArr.vertices[i + 1].color    = uwga::COLOR_GREY.withAlpha(125);
    }
    vArr.vertices[vertCount + 1] = vArr.vertices[1];

    auto& canvas = ccomp<MWindow>().getActiveCanvas();
    canvas.draw(vArr);

    _shape.debugDraw(hg::uwga::COLOR_AQUA, canvas);

    // Draw the construction grid on top
    if (_held) {
        if (const auto shipCtrl = getRuntime()->find("cinnabar::ShipController"); shipCtrl) {
            shipCtrl.downcastCopy<ShipController>()->drawGridOverShape(_shape, canvas);
        }
    }
}

ShipAttachable* AttachableGhost::_findAttachableById(QAO_GenericId aAttachableId) const {
    HG_ASSERT(getRuntime() != nullptr);

    auto handle = getRuntime()->find(aAttachableId);
    HG_ASSERT(!handle.isNull());

    ShipAttachable* ptr = nullptr;
    if (!QAO_SendMessage<DowncastToShipAttachable>(*handle, &ptr)) {
        HG_THROW_TRACED(hg::TracedLogicError,
                        0,
                        "Object {} did not answer to message 'DowncastToShipAttachable'.",
                        handle->getDebugDescription());
    }

    HG_ASSERT(ptr != nullptr);

    return ptr;
}

} // namespace cinnabar

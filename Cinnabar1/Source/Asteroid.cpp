// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Asteroid.hpp>

#include <Ship_controller.hpp>

#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <Hobgoblin/ChipmunkPhysics.hpp>

#include <span>

namespace cinnabar {

Asteroid::Asteroid(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard, QAO_ExeCon::GAMEPLAY, 0, QAO_STATIC_NAME("cinnabar::Asteroid")} {}

void Asteroid::init(double aX, double aY) {
    _shape.setAnchor({aX, aY});

    const auto vertCount = hg::util::GetRandomNumber<hg::PZInteger>(7, 12);
    _shape.setVertexCount(vertCount);

    for (hg::PZInteger i = 0; i < vertCount; ++i) {
        const auto vec =
            (hg::math::AngleF::fullCircle() * (float)i / (float)vertCount).asNormalizedVector() *
            hg::util::GetRandomNumber(100.f, 200.f);
        _shape.setRawVertexAt(i, vec);
    }

    const auto baricenterOffset = _shape.calculateBaricenterOffset();
    for (hg::PZInteger i = 0; i < vertCount; ++i) {
        _shape.setRawVertexAt(i, _shape.getRawVertexAt(i) - baricenterOffset);
    }

    _shape.setRotation(hg::math::AngleF::fromDegrees(hg::util::GetRandomNumber<float>(0.f, 359.f)));

    _shape.recalcRel();
}

void Asteroid::_didAttach(QAO_Runtime& aRuntime) {
    spe::StateObject::_didAttach(aRuntime);
}

void Asteroid::_eventUpdate1() {
    const auto& winMgr      = ccomp<MWindow>();
    const auto  input       = winMgr.getInput();
    const auto  mousePos    = input.getViewRelativeMousePos();
    const auto  mousePosRel = mousePos - _shape.getAnchor();

    if (input.checkPressed(hg::in::MB_LEFT, spe::WindowFrameInputView::Mode::Edge)) {
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

void Asteroid::_eventDraw1() {
    const auto vertCount = _shape.getVertexCount();

    // Draw the asteroid itself
    uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLE_FAN, vertCount + 2, _shape.getAnchor()};

    vArr.vertices[0].position = {};
    vArr.vertices[0].color    = uwga::COLOR_GREY;

    const auto outputVerts = _shape.getOutputVertices();

    for (std::size_t i = 0; i < hg::pztos(vertCount); ++i) {
        vArr.vertices[i + 1].position = outputVerts[i].cast<float>();
        vArr.vertices[i + 1].color    = uwga::COLOR_GREY;
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

} // namespace cinnabar

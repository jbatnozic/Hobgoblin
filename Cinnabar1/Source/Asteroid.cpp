// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Asteroid.hpp>

#include <Ship_controller.hpp>

#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

#include <Hobgoblin/ChipmunkPhysics.hpp>

#include <span>

namespace cinnabar {

namespace {
bool IsPointInsideShape(hg::math::Vector2d            aPos,
                        hg::math::Vector2d            aShapeCenter,
                        std::span<hg::math::Vector2d> aShapeVertices) {
    const auto vertCount = aShapeVertices.size();
    for (std::size_t i = 0; i < vertCount - 1; ++i) {
        if (hg::math::IsPointInsideTriangle<double>(
                aPos,
                {.a = aShapeCenter, .b = aShapeVertices[i], .c = aShapeVertices[i + 1]})) {
            return true;
        }
    }
    return hg::math::IsPointInsideTriangle<double>(
        aPos,
        {.a = aShapeCenter, .b = aShapeVertices[vertCount - 1], .c = aShapeVertices[0]});
};
} // namespace

Asteroid::Asteroid(QAO_InstGuard aInstGuard)
    : spe::StateObject{aInstGuard, QAO_ExeCon::GAMEPLAY, 0, QAO_STATIC_NAME("cinnabar::Asteroid")} {}

void Asteroid::init(double aX, double aY) {
    _center = {aX, aY};

    const auto vertCount = hg::util::GetRandomNumber<std::size_t>(7u, 12u);
    _verts.resize(vertCount);

    for (std::size_t i = 0; i < vertCount; ++i) {
        // clang-format off
        _verts[i] = {
            .relativeRotation = hg::math::AngleF::fullCircle() * (float)i / (float)vertCount,
            .distance = hg::util::GetRandomNumber<float>(100.f, 200.f)
        };
        // clang-format on
    }

    _rotation = hg::math::AngleF::fromDegrees(hg::util::GetRandomNumber<float>(0.f, 359.f));

    _recalculateAbsoluteVertices();
}

void Asteroid::_didAttach(QAO_Runtime& aRuntime) {
    spe::StateObject::_didAttach(aRuntime);
}

void Asteroid::_eventUpdate1() {
    const auto& winMgr   = ccomp<MWindow>();
    const auto  input    = winMgr.getInput();
    const auto  mousePos = input.getViewRelativeMousePos();

    if (input.checkPressed(hg::in::MB_LEFT, spe::WindowFrameInputView::Mode::Edge)) {
        if (IsPointInsideShape(mousePos, _center, _absoluteVerts)) {
            if (_held) {
                _held = false;
            } else {
                _cursorOffset = mousePos - _center;
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
                _cursorOffset = mousePos - _center;
                _shift = false;
            }
        }

        _rotation += hg::math::AngleF::fromDegrees(input.getVerticalMouseWheelScroll());

        if (_shift) {
            _rotation +=
                hg::math::AngleF::fromDegrees(static_cast<float>(_shiftCursorPos.x - mousePos.x)) * 0.25;
            _shiftCursorPos = mousePos;
        } else {
            _center = mousePos - _cursorOffset;
        }

        _recalculateAbsoluteVertices();
    }
}

void Asteroid::_eventDraw1() {
    // Draw the asteroid itself
    uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLE_FAN, hg::stopz(_verts.size() + 2), _center};
    uwga::VertexArray lines{uwga::PrimitiveType::LINE_STRIP, hg::stopz(_verts.size() + 1), _center};

    vArr.vertices[0].position = {};
    vArr.vertices[0].color    = uwga::COLOR_GREY;

    for (std::size_t i = 0; i < _verts.size(); ++i) {
        vArr.vertices[i + 1].position =
            (_rotation + _verts[i].relativeRotation).asNormalizedVector() * _verts[i].distance;
        vArr.vertices[i + 1].color = uwga::COLOR_GREY;

        lines.vertices[i].position = vArr.vertices[i + 1].position;
        lines.vertices[i].color    = uwga::COLOR_AQUA;
    }
    vArr.vertices[_verts.size() + 1] = vArr.vertices[1];
    lines.vertices[_verts.size()]    = lines.vertices[0];

    auto& canvas = ccomp<MWindow>().getActiveCanvas();
    canvas.draw(vArr);
    canvas.draw(lines);

    // Draw the construction grid on top
    if (_held) {
        if (const auto shipCtrl = getRuntime()->find("cinnabar::ShipController"); shipCtrl) {
            shipCtrl.downcastCopy<ShipController>()->drawGridOverShape(_center, _absoluteVerts, canvas);
        }
    }
}

void Asteroid::_recalculateAbsoluteVertices() {
    _absoluteVerts.clear();
    _absoluteVerts.reserve(_verts.size());
    for (const auto& vert : _verts) {
        _absoluteVerts.push_back(
            _center +
            ((_rotation + vert.relativeRotation).asNormalizedVector() * vert.distance).cast<double>());
    }
}

} // namespace cinnabar

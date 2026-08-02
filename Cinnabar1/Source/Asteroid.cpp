// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Asteroid.hpp>

#include <Ship.hpp>

#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Utility/Randomization.hpp>

namespace cinnabar {

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
}

void Asteroid::_didAttach(QAO_Runtime& aRuntime) {
    spe::StateObject::_didAttach(aRuntime);
}

void Asteroid::_eventUpdate1() {}

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
    lines.vertices[_verts.size()] = lines.vertices[0];

    auto& canvas = ccomp<MWindow>().getActiveCanvas();
    canvas.draw(vArr);
    canvas.draw(lines);

    // Draw the construction grid on top
    if (const auto ship = getRuntime()->find("cinnabar::Ship"); ship) {
        std::vector<hg::math::Vector2d> vertices;
        vertices.reserve(_verts.size());
        for (const auto& vert : _verts) {
            vertices.push_back(_center +
                               ((_rotation + vert.relativeRotation).asNormalizedVector() * vert.distance)
                                   .cast<double>());
        }

        ship.downcastCopy<Ship>()->drawGridOverShape(_center, vertices, canvas);
    }
}

} // namespace cinnabar

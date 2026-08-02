// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Math.hpp>

#include <vector>

namespace cinnabar {

class Asteroid : public spe::StateObject {
public:
    Asteroid(QAO_InstGuard aInstGuard);

    void init(double aX, double aY);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1() override;
    void _eventDraw1() override;

    void _recalculateAbsoluteVertices();

    hg::math::Vector2d _center = {};

    struct RadialVertex {
        hg::math::AngleF relativeRotation;
        float            distance;
    };

    std::vector<RadialVertex>       _verts         = {};
    std::vector<hg::math::Vector2d> _absoluteVerts = {};

    hg::math::AngleF _rotation = hg::math::AngleF::zero();

    bool               _held         = false;
    hg::math::Vector2d _cursorOffset = {};
    
    bool _shift = false;
    hg::math::Vector2d _shiftCursorPos = {};
};

QAO_REGISTER_CLASS(Asteroid, cinnabar_Asteroid) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
}

} // namespace cinnabar

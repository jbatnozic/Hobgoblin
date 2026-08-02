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

    hg::math::Vector2d _center = {};

    struct RadialVertex {
        hg::math::AngleF relativeRotation;
        float            distance;
    };

    std::vector<RadialVertex> _verts = {};

    hg::math::AngleF _rotation = hg::math::AngleF::zero();
};

} // namespace cinnabar

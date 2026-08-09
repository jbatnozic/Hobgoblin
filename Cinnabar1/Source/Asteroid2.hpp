// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Ship_attachable.hpp>
#include <Overworld_collisions.hpp>

namespace cinnabar {

class Asteroid2
    : public spe::StateObject
    , public UnibodyShipAttachable
    , public ovwcol::JunkEntity {
public:
    Asteroid2(QAO_InstGuard aInstGuard);

    void init(hg::math::Vector2d aPosition);

private:
    PolyShape                    _initPolyShape();
    PhysicalProperties           _initPhysicalProperties();
    hg::alvin::CollisionDelegate _initColDelegate();

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1() override;
    void _eventUpdate2() override;
    void _eventDraw1() override;
};

QAO_REGISTER_CLASS(Asteroid2, cinnabar_Asteroid2) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
}

} // namespace cinnabar

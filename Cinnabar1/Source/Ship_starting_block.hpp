// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Ship_attachable.hpp>
#include <Overworld_collisions.hpp>

namespace cinnabar {

// clang-format off
SPEMPE_DEFINE_AUTODIFF_STATE(ShipStartingBlock_VisibleState,
    SPEMPE_MEMBER(double, x, 0.0),
    SPEMPE_MEMBER(double, y, 0.0),
    SPEMPE_MEMBER(hg::math::AngleF, rotation, hg::math::AngleF::zero())
) {
    hg::math::Vector2d getPosition() const {
        return {x, y};
    }

    void setPosition(hg::math::Vector2d aPosition) {
        x = aPosition.x, y = aPosition.y;
    }
};
// clang-format on

class ShipStartingBlock
    : public spe::SynchronizedObject<ShipStartingBlock_VisibleState>
    , public UnibodyShipAttachable
    , public ovwcol::JunkEntity {
public:
    //! Factory method to create a master object.
    static QAO_Handle<ShipStartingBlock> createMaster(QAO_RuntimeRef     aRuntime,
                                                      hg::math::Vector2d aPosition);

    //! Factory method to create a dummy object.
    static QAO_Handle<ShipStartingBlock> createDummy(QAO_RuntimeRef aRuntime, spe::SyncId aSyncId);

    ~ShipStartingBlock();

    void drawAsGhost(
        const hg::math::Vector2d& aPosition,
        hg::math::AngleF          aAngle,
        uwga::Color               aColor,
        uwga::Canvas&             aCanvas,
        const uwga::RenderStates& aRenderStates = uwga::RENDER_STATES_DEFAULT) const override {}

private:
    BEFRIEND_QAO_CREATE;

    //! Private constructor for the master object.
    ShipStartingBlock(QAO_InstGuard aInstGuard);

    //! Private constructor for the dummy object.
    ShipStartingBlock(QAO_InstGuard aInstGuard, spe::SyncId aSyncId);

    PolyShape                    _initPolyShape();
    PhysicalProperties           _initPhysicalProperties();
    hg::alvin::CollisionDelegate _initColDelegate();

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventUpdate1(spe::IfMaster) override;
    void _eventUpdate2(spe::IfMaster) override;
    void _eventUpdate2(spe::IfDummy) override;
    void _eventPostUpdate(spe::IfMaster) override;
    void _eventDraw1() override;

    void _applyPropulsion(const spe::WindowFrameInputView& aInput);

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

QAO_REGISTER_CLASS(ShipStartingBlock, cinnabar_ShipStartingBlock) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::SynchronizedObjectBase>();
}

} // namespace cinnabar

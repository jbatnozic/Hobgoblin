// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Ship_attachable.hpp>
#include <Overworld_collisions.hpp>
#include <QAOMessages/Downcast_to_ship_attachable.hpp>
#include <QAOMessages/Handle_pncs_event.hpp>

namespace cinnabar {

// clang-format 
SPEMPE_DEFINE_AUTODIFF_STATE(Asteroid_VisibleState,
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

class Asteroid
    : public spe::SynchronizedObject<Asteroid_VisibleState>
    , public UnibodyShipAttachable
    , public ovwcol::JunkEntity {
public:
    //! Factory method to create a master object.
    static QAO_Handle<Asteroid> createMaster(QAO_RuntimeRef aRuntime, hg::math::Vector2d aPosition);

    //! Factory method to create a dummy object.
    static QAO_Handle<Asteroid> createDummy(QAO_RuntimeRef    aRuntime,
                                            spe::SyncId       aSyncId,
                                            hg::util::Packet& aPolyShapePacket);

    ~Asteroid();

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
    BEFRIEND_QAO_CREATE;

    //! Private constructor for the master object.
    Asteroid(QAO_InstGuard aInstGuard);

    //! Private constructor for the dummy object.
    Asteroid(QAO_InstGuard aInstGuard, spe::SyncId aSyncId, hg::util::Packet& aPolyShapePacket);

    bool _leftClicked = false;

    PolyShape                    _initPolyShape();
    PhysicalProperties           _initPhysicalProperties();
    hg::alvin::CollisionDelegate _initColDelegate();

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventBeginUpdate() override;
    // void _eventUpdate1(spe::IfMaster) override;
    void _eventUpdate2(spe::IfMaster) override;
    void _eventUpdate2(spe::IfDummy) override;
    void _eventDraw1() override;

    void _syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
    void _syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const override;
};

QAO_REGISTER_CLASS(Asteroid, cinnabar_Asteroid) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::StateObject>();
    klass.setMessageHandler<C, DowncastToShipAttachable, &C::msgDowncastToShipAttachable>();
    klass.setMessageHandler<C, HandlePNCSEvent, &C::msgHandlePNCSEvent>();
}

} // namespace cinnabar

// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_starting_block.hpp>

#include <InteriorWorld/Cell_archs.hpp>
#include <Overworld_manager.hpp>
#include <Ship/Constants.hpp>

namespace cinnabar {

#define SIZE (16.f * OVERWORLD_CELL_SIZE)

QAO_Handle<ShipStartingBlock> ShipStartingBlock::createMaster(QAO_RuntimeRef     aRuntime,
                                                             hg::math::Vector2d aPosition) {
    auto handle = QAO_Create<ShipStartingBlock>(aRuntime);
    cpBodySetPosition(handle->_unibody, cpv(aPosition.x, aPosition.y));
    return handle;
}

ShipStartingBlock::ShipStartingBlock(QAO_InstGuard aInstGuard)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES,
                   QAO_STATIC_NAME("cinnabar::ShipStartingBlock"),
                   spe::SYNC_ID_NEW}
    // clang-format off
    , UnibodyShipAttachable{
        std::bind(&ShipStartingBlock::_initPolyShape, this),
        std::bind(&ShipStartingBlock::_initPhysicalProperties, this),
        std::bind(&ShipStartingBlock::_initColDelegate, this),
        std::bind(&ShipStartingBlock::_alvinBodyFromPhysicalPropertiesAndPolyShape, this),
        std::bind(&ShipStartingBlock::_alvinShapeFromPolyShape, this)
    }
    // clang-format on
{
    _unibody.bindDelegate(*this);

    _iwSliceData = std::make_unique<InteriorWorldSliceData>();
    _iwSliceData->cells.reset(16, 16);
    _iwSliceData->cellGridOffset = hg::math::Vector2f{
        -7.5f * OVERWORLD_CELL_SIZE,
        -7.5f * OVERWORLD_CELL_SIZE,
    };
    _iwSliceData->rotationOffset = -hg::math::AngleF::halfCircle() * 0.5f;

    for (int y = 0; y < _iwSliceData->cells.getHeight(); ++y) {
        for (int x = 0; x < _iwSliceData->cells.getHeight(); ++x) {
            _iwSliceData->cells[y][x].cellKindId  = interior::cell_archetype::METALLIC_FLOOR.cellKindId;
            _iwSliceData->cells[y][x].floorSprite = interior::cell_archetype::METALLIC_FLOOR.floorSprite;
            _iwSliceData->cells[y][x].wallSprite  = interior::cell_archetype::METALLIC_FLOOR.wallSprite;
            _iwSliceData->cells[y][x].spatialInfo = interior::cell_archetype::METALLIC_FLOOR.spatialInfo;
            _iwSliceData->cells[y][x].userData.i64 = 0;
        }
    }

    _iwSliceData->cells[0][0].cellKindId = interior::cell_archetype::SOLID_VOID.cellKindId;
    _iwSliceData->cells[0][3].cellKindId = interior::cell_archetype::SOLID_VOID.cellKindId;
    _iwSliceData->cells[0][5].cellKindId = interior::cell_archetype::SOLID_VOID.cellKindId;
}

QAO_Handle<ShipStartingBlock> ShipStartingBlock::createDummy(QAO_RuntimeRef aRuntime,
                                                            spe::SyncId    aSyncId) {
    return QAO_Create<ShipStartingBlock>(aRuntime, aSyncId);
}

ShipStartingBlock::ShipStartingBlock(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES,
                   QAO_STATIC_NAME("cinnabar::ShipStartingBlock"),
                   aSyncId}
    , UnibodyShipAttachable{} //
{
    _polyShape = _initPolyShape();
}

ShipStartingBlock::~ShipStartingBlock() {
    ShipAttachable::_detach();
}

// MARK: Private

PolyShape ShipStartingBlock::_initPolyShape() {
    PolyShape shape{4};

    // Clockwise (I think clockwise is correct)
    shape.setRawVertexAtUnchecked(0, {-SIZE / 2, -SIZE / 2});
    shape.setRawVertexAtUnchecked(1, {+SIZE / 2, -SIZE / 2});
    shape.setRawVertexAtUnchecked(2, {+SIZE / 2, +SIZE / 2});
    shape.setRawVertexAtUnchecked(3, {-SIZE / 2, +SIZE / 2});

    // Note: baricenter offset is guaranteed 0,0 because the shape is completely symmetrical

    shape.recalcRel();

    return shape;
}

ShipStartingBlock::PhysicalProperties ShipStartingBlock::_initPhysicalProperties() {
    return {.mass = 5.0};
}

hg::alvin::CollisionDelegate ShipStartingBlock::_initColDelegate() {
    return hg::alvin::CollisionDelegateBuilder{}
        .setDefaultDecision(hg::alvin::Decision::ACCEPT_COLLISION)
        .finalize();
}

void ShipStartingBlock::_didAttach(QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);

    if (isMasterObject()) {
        _getCurrentState().initMirror();
        _unibody.addToSpace(ccomp<MOverworld>().getAlvinSpace());
    }
}

void ShipStartingBlock::_eventUpdate1(spe::IfMaster) {
    const auto& winMgr = ccomp<MWindow>();
    _applyPropulsion(winMgr.getInput());
}

void ShipStartingBlock::_eventUpdate2(spe::IfMaster) {
    _syncPolyShapeWithUnibody();

    auto& self = _getCurrentState();
    self.setPosition(_polyShape.getAnchor());
    self.rotation = _polyShape.getRotation();
}

void ShipStartingBlock::_eventUpdate2(spe::IfDummy) {
    if (isDeactivated()) {
        return;
    }

    const auto& self = _getCurrentState();
    _polyShape.setAnchor(self.getPosition());
    _polyShape.setRotation(self.rotation);
}

void ShipStartingBlock::_eventPostUpdate(spe::IfMaster) {
    _getCurrentState().commit();
}

void ShipStartingBlock::_eventDraw1() {
    auto& winMgr = ccomp<MWindow>();
    _polyShape.debugDraw(hg::uwga::COLOR_GREY, winMgr.getActiveCanvas());
}

void ShipStartingBlock::_applyPropulsion(const spe::WindowFrameInputView& aInput) {
    const auto left  = aInput.checkPressed(hg::in::PK_A);
    const auto right = aInput.checkPressed(hg::in::PK_D);
    const auto up    = aInput.checkPressed(hg::in::PK_W);
    const auto down  = aInput.checkPressed(hg::in::PK_S);

    // Propulsion
    const cpVect  pos                = cpBodyGetPosition(_unibody);
    const cpFloat propulsionStrength = 4000.0;

    if (up) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, force, pos);
    }

    if (down) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), -0.5 * propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, force, pos);
    }

    if (left) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), 0.5 * propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, cpvrperp(force), pos);
    }

    if (right) {
        const cpVect force = cpvmult(cpBodyGetRotation(_unibody), 0.5 * propulsionStrength);
        cpBodyApplyForceAtWorldPoint(_unibody, cpvperp(force), pos);
    }

    // Rotation
    auto rotForce = cpv(0.0, 0.0);

    if (aInput.checkPressed(hg::in::PK_E)) {
        rotForce.y += 2000.0;
    }
    if (aInput.checkPressed(hg::in::PK_Q)) {
        rotForce.y -= 2000.0;
    }

    cpBodyApplyForceAtLocalPoint(_unibody, rotForce, cpv(16.0, 0.0));
    cpBodyApplyForceAtLocalPoint(_unibody, cpvneg(rotForce), cpvzero);
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(ShipStartingBlock, (CREATE, UPDATE, DESTROY));

void ShipStartingBlock::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(ShipStartingBlock, aSyncCtrl);
}

void ShipStartingBlock::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    aSyncCtrl.filter([](hg::PZInteger aClientIndex) -> spe::SyncFilterStatus {
        return spe::SyncFilterStatus::REGULAR_SYNC; // TODO: implement proper filtering
    });
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(ShipStartingBlock, aSyncCtrl);
}

void ShipStartingBlock::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(ShipStartingBlock, aSyncCtrl);
}

} // namespace cinnabar

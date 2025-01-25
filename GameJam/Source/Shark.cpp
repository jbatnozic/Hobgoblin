#include "Shark.hpp"

#include "Environment_manager.hpp"
#include "Main_gameplay_manager_interface.hpp"
#include "Player_controls.hpp"

#include <Hobgoblin/HGExcept.hpp>

#include "Config.hpp"
#include "Varmap_ids.hpp"
#include <Hobgoblin/Math.hpp>
#include <cmath>

namespace {
// Physics parameters for Shark
static constexpr cpFloat PHYS_MASS             = 80.0;
static constexpr cpFloat PHYS_WIDTH            = 32.0;
static constexpr cpFloat PHYS_HEIGHT           = 64.0;
static constexpr cpFloat PHYS_DAMPING          = 0.9;
static constexpr cpFloat PHYS_ROTATIONAL_FORCE = 250'000.0;
static constexpr cpFloat PHYS_PROPULSION_FORCE = 400'000.0;

#define NUM_COLORS 12
static const hg::gr::Color COLORS[NUM_COLORS] = {hg::gr::COLOR_BLACK,
                                                 hg::gr::COLOR_RED,
                                                 hg::gr::COLOR_GREEN,
                                                 hg::gr::COLOR_YELLOW,
                                                 hg::gr::COLOR_BLUE,
                                                 hg::gr::COLOR_ORANGE,
                                                 hg::gr::COLOR_PURPLE,
                                                 hg::gr::COLOR_TEAL,
                                                 hg::gr::COLOR_BROWN,
                                                 hg::gr::COLOR_FUCHSIA,
                                                 hg::gr::COLOR_GREY,
                                                 hg::gr::COLOR_WHITE};
} // namespace

Shark::Shark(QAO_RuntimeRef aRuntimeRef, spe::RegistryId aRegId, spe::SyncId aSyncId)
    : SyncObjSuper{aRuntimeRef, SPEMPE_TYPEID_SELF, PRIORITY_PLAYERAVATAR, "Shark", aRegId, aSyncId} {
    if (isMasterObject()) {
        _getCurrentState().initMirror(); // To get autodiff optimization working

        _unibody.init(
            [this]() {
                return _initColDelegate();
            },
            [this]() {
                return hg::alvin::Body::createDynamic(
                    80.0,
                    cpMomentForBox(PHYS_MASS, PHYS_WIDTH, PHYS_HEIGHT));
            },
            [this]() {
                return hg::alvin::Shape::createBox(_unibody.body, PHYS_WIDTH, PHYS_HEIGHT);
            });
        _unibody.bindDelegate(*this);
        _unibody.addToSpace(ccomp<MEnvironment>().getSpace());

        cpBodySetVelocityUpdateFunc(
            _unibody,
            [](cpBody* aBody, cpVect aGravity, cpFloat /*aDamping*/, cpFloat aDt) {
                cpBodyUpdateVelocity(aBody, aGravity, PHYS_DAMPING, aDt);
            });
    } else {
        // _renderer.emplace(ctx(), hg::gr::COLOR_RED);
        // _renderer->setMode(CharacterRenderer::Mode::STILL);
    }
}

Shark::~Shark() {
    if (isMasterObject()) {
        doSyncDestroy();
    }
}

void Shark::init(int aOwningPlayerIndex, float aX, float aY) {
    HG_HARD_ASSERT(isMasterObject());

    auto& self             = _getCurrentState();
    self.owningPlayerIndex = aOwningPlayerIndex;
    self.x                 = aX;
    self.y                 = aY;
    self.directionInRad    = 0.f;

    cpBodySetPosition(_unibody, cpv(aX, aY));
}

void Shark::_eventUpdate1(spe::IfMaster) {
    if (ctx().getGameState().isPaused) {
        return;
    }

    auto& self = _getCurrentState();
    HG_HARD_ASSERT(self.owningPlayerIndex >= 0);

    auto& lobbyBackend = ccomp<MLobbyBackend>();
    if (const auto clientIndex = lobbyBackend.playerIdxToClientIdx(self.owningPlayerIndex);
        clientIndex != spe::CLIENT_INDEX_UNKNOWN) {

        spe::InputSyncManagerWrapper wrapper{ccomp<MInput>()};

        const auto left  = wrapper.getSignalValue<ControlDirectionType>(clientIndex, CTRL_ID_LEFT);
        const auto right = wrapper.getSignalValue<ControlDirectionType>(clientIndex, CTRL_ID_RIGHT);

        const auto up   = wrapper.getSignalValue<ControlDirectionType>(clientIndex, CTRL_ID_UP);
        const auto down = wrapper.getSignalValue<ControlDirectionType>(clientIndex, CTRL_ID_DOWN);

        // space.runRaycastQuery(cpv(self.x - RAY_X_OFFSET, self.y + RAY_Y_OFFSET),
        //                       cpv(self.x + RAY_X_OFFSET, self.y + RAY_Y_OFFSET),
        //                       10.0,
        //                       cpShapeFilterNew(0, CP_ALL_CATEGORIES, CAT_TERRAIN),
        //                       [&, this](const hg::alvin::RaycastQueryInfo&) {
        //                           touchingTerrain = true;
        //                       });

        _execMovement(left, right, up, down);

        const auto pos      = cpBodyGetPosition(_unibody);
        self.x              = static_cast<float>(pos.x);
        self.y              = static_cast<float>(pos.y);
        const auto rot      = cpBodyGetRotation(_unibody);
        self.directionInRad = hg::math::AngleF::fromVector({(float)rot.x, (float)rot.y}).asRadians();
    }
}

void Shark::_eventUpdate1(spe::IfDummy) {
    if (this->isDeactivated() || ctx().getGameState().isPaused) {
        return;
    }

    auto& self = _getCurrentState();

    auto& lobbyBackend = ccomp<MLobbyBackend>();
    if (lobbyBackend.getLocalPlayerIndex() == self.owningPlayerIndex) {
        _adjustView();
    }
}

void Shark::_eventPostUpdate(spe::IfMaster) {
    _getCurrentState().commit();
}

void Shark::_eventDraw1() {
    if (this->isDeactivated()) {
        return;
    }

    auto& winMgr = ccomp<MWindow>();
    auto& canvas = winMgr.getCanvas();

    const auto& self = _getCurrentState();

    hg::gr::RectangleShape rect;
    rect.setSize({(float)PHYS_WIDTH, (float)PHYS_HEIGHT});
    rect.setOrigin({(float)PHYS_WIDTH / 2.f, (float)PHYS_HEIGHT / 2.f});
    rect.setFillColor(hg::gr::COLOR_TRANSPARENT);
    rect.setOutlineColor(hg::gr::COLOR_YELLOW);
    rect.setOutlineThickness(2.f);
    rect.setPosition(self.x, self.y);
    rect.setRotation(hg::math::AngleF::fromRadians(self.directionInRad));
    canvas.draw(rect);

    hg::gr::CircleShape cir{8.f};
    cir.setFillColor(hg::gr::COLOR_RED);
    cir.setOrigin({8.f, 32.f});
    cir.setPosition(self.x, self.y);
    cir.setRotation(hg::math::AngleF::fromRadians(self.directionInRad));
    canvas.draw(cir);
}

void Shark::_eventDraw2() {
    if (this->isDeactivated())
        return;

    // TODO: draw player name
    //  const auto& self = _getCurrentState();

    // auto& lobbyBackend = ccomp<MLobbyBackend>();
    // if (self.owningPlayerIndex > 0) {
    //     const auto&  name = lobbyBackend.getLockedInPlayerInfo(self.owningPlayerIndex).name;
    //     hg::gr::Text text{hg::gr::BuiltInFonts::getFont(hg::gr::BuiltInFonts::TITILLIUM_REGULAR),
    //                       name,
    //                       30};
    //     text.setScale({2.f, 2.f});
    //     text.setFillColor(hg::gr::COLOR_WHITE);
    //     text.setOutlineColor(hg::gr::COLOR_BLACK);
    //     text.setOutlineThickness(4.f);
    //     const auto& localBounds = text.getLocalBounds();
    //     text.setOrigin({localBounds.w / 2.f, localBounds.h / 2.f});
    //     text.setPosition({self.x, self.y - 180.f});
    //     ccomp<MWindow>().getCanvas().draw(text);
    // }
}

void Shark::_execMovement(bool aLeft, bool aRight, bool aUp, bool aDown) {
    auto& space = ccomp<MEnvironment>().getSpace();

    // Rotation
    {
        const cpFloat lr = static_cast<cpFloat>(aRight) - static_cast<cpFloat>(aLeft);
        const cpFloat ud = static_cast<cpFloat>(aDown) - static_cast<cpFloat>(aUp);

        const auto currentRotationVec = cpBodyGetRotation(_unibody);
        const auto currentRotation =
            hg::math::Angle<cpFloat>::fromVector(currentRotationVec.x, currentRotationVec.y);

        const auto targetRotation = (aLeft || aRight || aUp || aDown)
                                        ? (hg::math::Angle<cpFloat>::fromVector({lr, ud}) -
                                           hg::math::Angle<cpFloat>::fromDeg(90.0))
                                        : hg::math::Angle<cpFloat>::zero();

        const auto rotationDiff = currentRotation.shortestDistanceTo(targetRotation);

        cpVect rotationalForce;
        if (rotationDiff.asRadians() < 0.0) {
            rotationalForce = cpv(0.0, -PHYS_ROTATIONAL_FORCE);
        } else {
            rotationalForce = cpv(0.0, +PHYS_ROTATIONAL_FORCE);
        }

        rotationalForce =
            rotationalForce * (std::abs(rotationDiff.asRadians()) / hg::math::Pi<cpFloat>());

        cpBodyApplyForceAtLocalPoint(_unibody,
                                     cpvneg(rotationalForce),
                                     cpv(12.0, 0.0)); // Idk why 12 but it works
        cpBodyApplyForceAtLocalPoint(_unibody, rotationalForce, cpvzero);
    }

    // Propulsion
    if (aLeft || aRight || aUp || aDown) {
        const auto   angleRad = cpvtoangle(cpBodyGetRotation(_unibody)) - hg::math::Pi<cpFloat>() / 2;
        const cpVect force    = cpvmult(cpvforangle(angleRad), PHYS_PROPULSION_FORCE);
        cpBodyApplyForceAtWorldPoint(_unibody, force, cpBodyGetPosition(_unibody));
    }
}

hg::alvin::CollisionDelegate Shark::_initColDelegate() {
    auto builder = hg::alvin::CollisionDelegateBuilder{};
    builder.setDefaultDecision(hg::alvin::Decision::ACCEPT_COLLISION);

    builder.addInteraction<TerrainInterface>(
        hg::alvin::COLLISION_CONTACT,
        [this](TerrainInterface& aTerrain, const hg::alvin::CollisionData& aCollisionData) {
            // CP_ARBITER_GET_SHAPES(aCollisionData.arbiter, shape1, shape2);
            // NeverNull<cpShape*> otherShape = shape1;
            // if (otherShape == _unibody.shape) {
            //     otherShape = shape2;
            // }
            // const auto cellKind = aTerrain.getCellKindOfShape(otherShape);
            // if (cellKind && *cellKind == CellKind::SCALE) {
            //     HG_LOG_INFO(LOG_ID, "Character reached the scales.");
            //     ccomp<MainGameplayManagerInterface>().characterReachedTheScales(*this);
            // }
            return hg::alvin::Decision::ACCEPT_COLLISION;
        });

    builder.addInteraction<SharkInterface>(
        hg::alvin::COLLISION_CONTACT,
        [this](SharkInterface& aShark, const hg::alvin::CollisionData& aCollisionData) {
            // TODO: die
            return hg::alvin::Decision::ACCEPT_COLLISION;
        });

    return builder.finalize();
}

namespace {
hg::math::AngleF PointDirection2(hg::math::Vector2f aFrom, hg::math::Vector2f aTo) {
    return hg::math::PointDirection(aFrom.x, aFrom.y, aTo.x, aTo.y);
}
} // namespace

void Shark::_adjustView() {
    auto& self = _getCurrentState();
    auto& view = ccomp<MWindow>().getView(0);
#if 0
    auto& view = [this]() -> hg::gr::View& {
        auto gameOver = ccomp<MVarmap>().getInt64(VARMAP_ID_GAME_OVER);
        if (gameOver.has_value() && *gameOver == 1) {
            auto& winMgr = ccomp<MWindow>();
            winMgr.getView(0).setEnabled(false);
            return winMgr.getView(1);
        } else {
            auto& winMgr = ccomp<MWindow>();
            winMgr.getView(1).setEnabled(false);
            return winMgr.getView(0);
        }
    }();
    view.setEnabled(true);

    auto targetPos = sf::Vector2f{self.x, self.y};
    // if (self.direction & DIRECTION_RIGHT) {
    //     targetPos.x += CAMERA_OFFSET;
    // }
    // if (self.direction & DIRECTION_LEFT) {
    //     targetPos.x -= CAMERA_OFFSET;
    // }
    // if (self.direction & DIRECTION_DOWN) {
    //     targetPos.y += CAMERA_OFFSET;
    // }
    // if (self.direction & DIRECTION_UP) {
    //     targetPos.y -= CAMERA_OFFSET;
    // }

    {
        auto gameOver = ccomp<MVarmap>().getInt64(VARMAP_ID_GAME_OVER);
        if (gameOver.has_value() && *gameOver == 1) {
            const auto scalesPos = ccomp<MEnvironment>().getScalesGridPosition();
            targetPos            = {static_cast<float>((scalesPos.x + 2.f) * single_terrain_size),
                                    static_cast<float>(scalesPos.y * single_terrain_size)};
        }
    }

    const auto viewCenter = view.getCenter();
    const auto dist =
        hg::math::EuclideanDist<float>(viewCenter.x, viewCenter.y, targetPos.x, targetPos.y);
    const auto theta = PointDirection2(view.getCenter(), targetPos).asRadians();

    if (dist >= 1000.0 || dist < 2.f) {
        view.setCenter(targetPos);
    } else if (dist >= 2.f) {
        view.move(+std::cosf(theta) * dist * 0.045f, -std::sinf(theta) * dist * 0.045f);
    }

    {
        auto&       envMgr = ccomp<MEnvironment>();
        const float minX   = view.getSize().x / 2.f;
        const float minY   = view.getSize().y / 2.f;
        const float maxX   = envMgr.getGridSize().x * single_terrain_size - minX;
        const float maxY   = envMgr.getGridSize().y * single_terrain_size - minY;

        auto pos = view.getCenter();
        if (pos.x < minX) {
            pos.x = minX;
        } else if (pos.x >= maxX) {
            pos.x = maxX - 1.f;
        }
        if (pos.y < minY) {
            pos.y = minY;
        } else if (pos.y >= maxY) {
            pos.y = maxY - 1.f;
        }
        // view.setCenter(pos);
        view.setCenter({1000.f, 1000.f});
    }
#endif
    view.setCenter({1000.f, 1000.f});

    // Round
    const auto center = view.getCenter();
    view.setCenter(std::roundf(center.x), std::roundf(center.y));
}

SPEMPE_GENERATE_DEFAULT_SYNC_HANDLERS(Shark, (CREATE, UPDATE, DESTROY));

void Shark::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_CREATE_DEFAULT_IMPL(Shark, aSyncCtrl);
}

void Shark::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_UPDATE_DEFAULT_IMPL(Shark, aSyncCtrl);
}

void Shark::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    SPEMPE_SYNC_DESTROY_DEFAULT_IMPL(Shark, aSyncCtrl);
}

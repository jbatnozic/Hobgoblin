// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_controller.hpp>

#include <Graphics_system_provider.hpp>

#include <GridGoblin/World/World_config.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Circle_shape.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Rectangle_shape.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <cmath>
#include <vector>

namespace cinnabar {

// MARK: Config

namespace grid = ::jbatnozic::gridgoblin;

namespace {
#define GRID_RESOLUTION 48.f

// clang-format off
constexpr grid::ContentsConfig INTERIOR_WORLD_CONFIG = {
    .chunkCountX     = 1024,
    .chunkCountY     = 512,
    .cellsPerChunkX  = 16,
    .cellsPerChunkY  = 16,
    .buildingBlocks  = grid::BuildingBlockMask::ALL,
    .cellResolution  = GRID_RESOLUTION,
    .wallHeight      = GRID_RESOLUTION,
    .maxCellOpenness = 0,
    .maxLoadedNonessentialChunks = 0xFFFFFF
};

constexpr hg::math::Vector2d INTERIOR_WORLD_ORIGIN = {
    (INTERIOR_WORLD_CONFIG.chunkCountX / 2 * INTERIOR_WORLD_CONFIG.cellsPerChunkX) * GRID_RESOLUTION,
    (INTERIOR_WORLD_CONFIG.chunkCountY / 2 * INTERIOR_WORLD_CONFIG.cellsPerChunkY) * GRID_RESOLUTION
};
// clang-format on
} // namespace

// MARK: MasterData

ShipController_MasterData::ShipController_MasterData()
    : interiorWorld{INTERIOR_WORLD_CONFIG} {}

// MARK: ShipController

ShipController::ShipController(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   5,
                   QAO_STATIC_NAME("cinnabar::ShipController"),
                   aSyncId} {}

void ShipController::init(double aX, double aY) {
    _position = {aX, aY};
}

void ShipController::drawGridOverShape(hg::math::Vector2d            aShapeCenter,
                                       std::span<hg::math::Vector2d> aShapeVertices,
                                       uwga::Canvas&                 aCanvas) {
    // Recalculate all shape vertices relative to the ship
    auto                            relativeShapeCenter = (aShapeCenter - _position).cast<float>();
    std::vector<hg::math::Vector2f> relativeShapeVertices{};
    {
        relativeShapeVertices.reserve(aShapeVertices.size());
        for (const auto& vert : aShapeVertices) {
            relativeShapeVertices.push_back((vert - _position).cast<float>());
        }
    }

    // Transform all vertices into the ship's coordinate system
    {
        _masterData->transform->transformPoints(1, &relativeShapeCenter);
        for (auto& vert : relativeShapeVertices) {
            _masterData->transform->transformPoints(1, &vert);
        }
    }

    // Find the AABB of the shape in the ship's coordinate system
    hg::math::Vector2f aabbTopLeft     = relativeShapeCenter;
    hg::math::Vector2f aabbBottomRight = relativeShapeCenter;
    {
        for (const auto vert : relativeShapeVertices) {
            if (vert.x < aabbTopLeft.x) {
                aabbTopLeft.x = vert.x;
            }
            if (vert.x > aabbBottomRight.x) {
                aabbBottomRight.x = vert.x;
            }
            if (vert.y < aabbTopLeft.y) {
                aabbTopLeft.y = vert.y;
            }
            if (vert.y > aabbBottomRight.y) {
                aabbBottomRight.y = vert.y;
            }
        }
    }

    // Find grid coordinates of the AABB
    hg::math::Vector2i aabbGridTopLeft;
    hg::math::Vector2i aabbGridBottomRight;
    {
        aabbGridTopLeft     = {static_cast<int>(std::floor(aabbTopLeft.x / GRID_RESOLUTION)) - 1,
                               static_cast<int>(std::floor(aabbTopLeft.y / GRID_RESOLUTION)) - 1};
        aabbGridBottomRight = {static_cast<int>(std::floor(aabbBottomRight.x / GRID_RESOLUTION)) + 1,
                               static_cast<int>(std::floor(aabbBottomRight.y / GRID_RESOLUTION)) + 1};
    }

    // Construct lambdas for checking if a point/shape is inside of the shape
    auto isPointInsideShape = [&relativeShapeCenter,
                               &relativeShapeVertices](hg::math::Vector2f aPos) -> bool {
        const auto vertCount = relativeShapeVertices.size();
        for (std::size_t i = 0; i < vertCount - 1; ++i) {
            if (hg::math::IsPointInsideTriangle(
                    aPos,
                    hg::math::Triangle<float>{.a = relativeShapeCenter,
                                              .b = relativeShapeVertices[i],
                                              .c = relativeShapeVertices[i + 1]})) {
                return true;
            }
        }
        return hg::math::IsPointInsideTriangle(
            aPos,
            hg::math::Triangle<float>{.a = relativeShapeCenter,
                                      .b = relativeShapeVertices[vertCount - 1],
                                      .c = relativeShapeVertices[0]});
    };

    // Draw the grid
    uwga::RectangleShape rect{
        aCanvas.getSystem(),
        {GRID_RESOLUTION - 2.f, GRID_RESOLUTION - 2.f}
    };
    rect.setOrigin(-1.f, -1.f);
    rect.setRotation(-_rotation);
    rect.setOutlineThickness(2.f);
    rect.setFillColor(uwga::COLOR_TRANSPARENT);

    for (int yy = aabbGridTopLeft.y; yy <= aabbGridBottomRight.y; ++yy) {
        for (int xx = aabbGridTopLeft.x; xx <= aabbGridBottomRight.x; ++xx) {
            auto       squareTopLeft = hg::math::Vector2f{xx * GRID_RESOLUTION, yy * GRID_RESOLUTION};
            const bool isSquareInsideShape =
                isPointInsideShape(squareTopLeft) &&
                isPointInsideShape({(xx + 1) * GRID_RESOLUTION, (yy + 0) * GRID_RESOLUTION}) &&
                isPointInsideShape({(xx + 0) * GRID_RESOLUTION, (yy + 1) * GRID_RESOLUTION}) &&
                isPointInsideShape({(xx + 1) * GRID_RESOLUTION, (yy + 1) * GRID_RESOLUTION});

            if (isSquareInsideShape) {
                rect.setOutlineColor(uwga::COLOR_LIME.withAlpha(175));
            } else {
                rect.setOutlineColor(uwga::COLOR_ORANGE.withAlpha(100));
            }

            _masterData->transformInverse->transformPoints(1, &squareTopLeft);
            const auto anchor = squareTopLeft.cast<double>() + _position;
            rect.setAnchor(anchor);

            aCanvas.draw(rect);
        }
    }
}

void ShipController::_didAttach(QAO_Runtime& aRuntime) {
    SyncObjSuper::_didAttach(aRuntime);

    if (isMasterObject()) {
        auto& md            = *_masterData;
        md.transform        = ccomp<GraphicsSystemProvider>().getSystem().createTransform();
        md.transformInverse = md.transform->clone();
    }
}

void ShipController::_eventUpdate1(spe::IfMaster) {
    const auto& winMgr = ccomp<MWindow>();
    const auto  input  = winMgr.getInput();

    // Keyboard inputs

    float rotationDir = 0.f;
    if (input.checkPressed(hg::in::PK_Q)) {
        rotationDir += 1.f;
    }
    if (input.checkPressed(hg::in::PK_E)) {
        rotationDir -= 1.f;
    }
    _rotation += hg::math::AngleF::fromDegrees(rotationDir * 3.f);

    double xx = 0.0;
    if (input.checkPressed(hg::in::PK_A)) {
        xx -= 1.0;
    }
    if (input.checkPressed(hg::in::PK_D)) {
        xx += 1.0;
    }
    _position.x += xx * 6.0;

    double yy = 0.0;
    if (input.checkPressed(hg::in::PK_W)) {
        yy -= 1.0;
    }
    if (input.checkPressed(hg::in::PK_S)) {
        yy += 1.0;
    }
    _position.y += yy * 6.0;

    // Set transforms

    auto& md = *_masterData;

    md.transform->setToIdentity();
    md.transform->rotate(_rotation);
    md.transformInverse->setToInverseOf(*md.transform);

    // Mouse input

    if (input.checkPressed(hg::in::MB_LEFT)) {
        hg::math::Vector2f relativeMousePos =
            (input.getViewRelativeMousePos() - _position).cast<float>();

        md.transform->transformPoints(1, &relativeMousePos);

        // HG_LOG_INFO(LOG_ID, "Relative mouse pos = x: {}, y: {}", relativeMousePos.x,
        // relativeMousePos.y);

        _mousePosInLocalCoords = relativeMousePos;
        _drawGrid              = true;
    } else {
        _drawGrid = false;
    }
}

void ShipController::_eventDraw1() {
    auto& winMgr = ccomp<MWindow>();

    uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLES, 3};
    vArr.anchor = _position;

    for (std::size_t i = 0; i < vArr.vertices.size(); ++i) {
        const auto relativePos =
            (_rotation + hg::math::AngleF::fromDegrees(120.f * i)).asNormalizedVector() * 48.f;

        auto& vert    = vArr.vertices[i];
        vert.position = relativePos;
        vert.color    = (i == 0) ? uwga::COLOR_RED : uwga::COLOR_WHEAT;
    }

    winMgr.getActiveCanvas().draw(vArr);

    // Center circle
    {
        uwga::CircleShape circle{winMgr.getGraphicsSystem(), 4.f, 8};
        circle.setOrigin(4.f, 4.f);
        circle.setAnchor(_position);
        circle.setFillColor(uwga::COLOR_LIME);
        winMgr.getActiveCanvas().draw(circle);
    }

    // Grid square
    if (_drawGrid) {
        uwga::RectangleShape rect{
            winMgr.getGraphicsSystem(),
            {GRID_RESOLUTION, GRID_RESOLUTION}
        };
        rect.setRotation(-_rotation);

        auto flooredMousePosInLocalCoords =
            hg::math::Vector2f{std::floor(_mousePosInLocalCoords.x / GRID_RESOLUTION) * GRID_RESOLUTION,
                               std::floor(_mousePosInLocalCoords.y / GRID_RESOLUTION) * GRID_RESOLUTION};

        _masterData->transformInverse->transformPoints(1, &flooredMousePosInLocalCoords);

        const auto anchor = flooredMousePosInLocalCoords.cast<double>() + _position;
        rect.setAnchor(anchor);

        rect.setOutlineThickness(2.f);
        rect.setOutlineColor(uwga::COLOR_RED);
        rect.setFillColor(uwga::COLOR_TRANSPARENT);

        winMgr.getActiveCanvas().draw(rect);
    }
}

// MARK: Sync impl.

void ShipController::_syncCreateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    // TODO
}

void ShipController::_syncUpdateImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    // TODO
}

void ShipController::_syncDestroyImpl(spe::SyncControlDelegate& aSyncCtrl) const {
    // TODO
}

} // namespace cinnabar

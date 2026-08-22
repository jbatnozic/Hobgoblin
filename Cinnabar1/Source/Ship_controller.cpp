// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_controller.hpp>

#include <Graphics_system_provider.hpp>
#include <InteriorWorld/Cell_archs.hpp>
#include <Ship/Constants.hpp>

#include <GridGoblin/World/World_config.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Circle_shape.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Rectangle_shape.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <cmath>
#include <vector>

namespace cinnabar {

using hg::math::AngleF;
using hg::math::IsNearZero;

namespace {
#define GRID_RESOLUTION OVERWORLD_CELL_SIZE

constexpr float ONE_DEG_AS_RAD = hg::math::DegToRad(1.f);

void TransformPoints(hg::math::Vector2f&    aCentralPoint,
                     hg::math::Vector2f*    aPoints,
                     std::size_t            aPointCount,
                     const uwga::Transform& aTransform) {
    for (std::size_t i = 0; (i + 3) < aPointCount; i += 4) {
        aTransform.transformPoints(4,
                                   &aPoints[i + 0],
                                   &aPoints[i + 1],
                                   &aPoints[i + 2],
                                   &aPoints[i + 3]);
    }

    switch (aPointCount % 4) {
    case 0:
        aTransform.transformPoints(1, &aCentralPoint);
        break;

    case 1:
        aTransform.transformPoints(2, &aCentralPoint, &aPoints[aPointCount - 1]);
        break;

    case 2:
        aTransform.transformPoints(3,
                                   &aCentralPoint,
                                   &aPoints[aPointCount - 3],
                                   &aPoints[aPointCount - 2]);
        break;

    case 3:
        aTransform.transformPoints(4,
                                   &aCentralPoint,
                                   &aPoints[aPointCount - 3],
                                   &aPoints[aPointCount - 2],
                                   &aPoints[aPointCount - 1]);
        break;
    }
}
} // namespace

// MARK: MasterData

ShipController_MasterData::ShipController_MasterData()
    : interiorWorld{} {}

// MARK: ShipController

ShipController::ShipController(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES, // TODO: set in relation to attachables
                   QAO_STATIC_NAME("cinnabar::ShipController"),
                   aSyncId} {}

void ShipController::init(ShipAttachable& aInitialShipAttachable) {
    HG_VALIDATE_PRECONDITION(isMasterObject());

    _masterData->graphOfAttachables.insert(aInitialShipAttachable);
}

void ShipController::attach(ShipAttachable&    aShipAttachable,
                            hg::math::Vector2f aAnchorOffset,
                            hg::math::AngleF   aRotationOffset) {
    const auto* iwSliceData = aShipAttachable.getInteriorWorldSliceData();
    if (iwSliceData != nullptr) {
        if (IsNearZero(aRotationOffset.asRad(), ONE_DEG_AS_RAD)) {

        } else if (IsNearZero((aRotationOffset - AngleF::halfCircle() * 0.5f).asRad(),
                                        ONE_DEG_AS_RAD)) {

        } else if (IsNearZero((aRotationOffset - AngleF::halfCircle()).asRad(),
                                        ONE_DEG_AS_RAD)) {

        } else if (IsNearZero((aRotationOffset - AngleF::halfCircle() * 1.5f).asRad(),
                                        ONE_DEG_AS_RAD)) {
        } else {
            // TODO - MUST NOT HAPPEN!
        }
    } else {
        HG_NOT_IMPLEMENTED("TODO - cell generator func");
    }
}

void ShipController::drawGridOverShape(const PolyShape& aShape, uwga::Canvas& aCanvas) {
    HG_HARD_ASSERT(aShape.getState() == PolyShape::READY_RELATIVE);

    // Recalculate all shape vertices relative to the ship
    auto                            relativeShapeCenter = (aShape.getAnchor() - _position).cast<float>();
    std::vector<hg::math::Vector2f> relativeShapeVertices{};
    {
        relativeShapeVertices.reserve(hg::pztos(aShape.getVertexCount()));
        for (const auto& vert : aShape.getOutputVertices()) {
            relativeShapeVertices.push_back(relativeShapeCenter + vert.cast<float>());
        }
    }

    // Transform all vertices into the ship's coordinate system
    {
        TransformPoints(relativeShapeCenter,
                        relativeShapeVertices.data(),
                        relativeShapeVertices.size(),
                        *_masterData->transform);
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

    // Construct lambdas for checking if a point is inside of the shape
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

void ShipController::projectCellPositions(const PolyShape&        aShape,
                                          ProjectedCellPositions& aProjectedCellPositions) {
    HG_HARD_ASSERT(aShape.getState() == PolyShape::READY_RELATIVE);

    // Recalculate all shape vertices relative to the ship
    auto                            relativeShapeCenter = (aShape.getAnchor() - _position).cast<float>();
    std::vector<hg::math::Vector2f> relativeShapeVertices{};
    {
        relativeShapeVertices.reserve(hg::pztos(aShape.getVertexCount()));
        for (const auto& vert : aShape.getOutputVertices()) {
            relativeShapeVertices.push_back(relativeShapeCenter + vert.cast<float>());
        }
    }

    // Transform all vertices into the ship's coordinate system
    {
        TransformPoints(relativeShapeCenter,
                        relativeShapeVertices.data(),
                        relativeShapeVertices.size(),
                        *_masterData->transform);
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

    // Construct lambdas for checking if a point is inside of the shape
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

    // Output
    aProjectedCellPositions.topLeftPos = aabbGridTopLeft;
    aProjectedCellPositions.cells.reset(aabbGridBottomRight.x - aabbTopLeft.x + 1,
                                        aabbBottomRight.y - aabbTopLeft.y + 1);
    aProjectedCellPositions.totalBitmask = ProjectedCellPositions::EMPTY;

    const auto&                             ggwld = _masterData->interiorWorld.getUnderlying();
    jbatnozic::gridgoblin::cell::CellKindId cellKindId;
    for (int yy = aabbGridTopLeft.y; yy <= aabbGridBottomRight.y; ++yy) {
        for (int xx = aabbGridTopLeft.x; xx <= aabbGridBottomRight.x; ++xx) {
            const bool isSquareInsideShape =
                isPointInsideShape({(xx + 0) * GRID_RESOLUTION, (yy + 0) * GRID_RESOLUTION}) &&
                isPointInsideShape({(xx + 1) * GRID_RESOLUTION, (yy + 0) * GRID_RESOLUTION}) &&
                isPointInsideShape({(xx + 0) * GRID_RESOLUTION, (yy + 1) * GRID_RESOLUTION}) &&
                isPointInsideShape({(xx + 1) * GRID_RESOLUTION, (yy + 1) * GRID_RESOLUTION});

            std::int8_t cellValue = ProjectedCellPositions::EMPTY;
            if (isSquareInsideShape) {
                cellValue |= ProjectedCellPositions::INSIDE_SHAPE;
            }
            if (xx < 0 || xx >= ggwld.getCellCountX() || yy < 0 || yy >= ggwld.getCellCountY()) {
                cellValue |= ProjectedCellPositions::OUT_OF_BOUNDS;
            } else if (ggwld.getCellDataAt(xx, yy, &cellKindId) &&
                       cellKindId.value != ToU16(interior::CellArchE::SOLID_VOID)) {
                cellValue |= ProjectedCellPositions::COLLIDES_WITH_IW;
            }

            aProjectedCellPositions.totalBitmask |= cellValue;
            aProjectedCellPositions.cells[yy - aabbGridTopLeft.y][xx - aabbGridTopLeft.y] = cellValue;
        }
    }
}

// QAO Message Handlers

void ShipController::msgDowncastToShipController(DowncastToShipController::PayloadPtr aPtr,
                                                 bool /* aConst */) {
    (*aPtr) = this;
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
#if 0
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
    _rotation += AngleF::fromDegrees(rotationDir * 3.f);

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
#endif
    auto&       md      = *_masterData;
    const auto& mainAtt = md.graphOfAttachables.getNode(0)->associatedAttachable;

    _position = mainAtt.getPolyShape().getAnchor();
    _rotation = mainAtt.getPolyShape().getRotation();

    md.transform->setToIdentity();
    md.transform->rotate(_rotation);
    md.transformInverse->setToInverseOf(*md.transform);
}

void ShipController::_eventDraw1() {
    auto& winMgr = ccomp<MWindow>();

    uwga::VertexArray vArr{uwga::PrimitiveType::TRIANGLES, 3};
    vArr.anchor = _position;

    for (std::size_t i = 0; i < vArr.vertices.size(); ++i) {
        const auto relativePos =
            (_rotation + AngleF::fromDegrees(120.f * i)).asNormalizedVector() * 48.f;

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

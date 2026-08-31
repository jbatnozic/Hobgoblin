// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_controller.hpp>

#include <Attachable_ghost.hpp>
#include <Graphics_system_provider.hpp>
#include <InteriorWorld/Cell_archs.hpp>
#include <InteriorWorld/Cell_props.hpp>
#include <Ship/Constants.hpp>

#include <GridGoblin/World/World_config.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Circle_shape.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Rectangle_shape.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace cinnabar {

using hg::math::AngleF;
using hg::math::IsNearZero;

namespace {
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

// MARK: ShipController PUBLIC

ShipController::ShipController(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES, // TODO: set in relation to attachables
                   QAO_STATIC_NAME("cinnabar::ShipController"),
                   aSyncId} {}

void ShipController::init(ShipAttachable& aInitialShipAttachable) {
    HG_VALIDATE_PRECONDITION(isMasterObject());

    attach(aInitialShipAttachable, {}, {});
}

#define HOLDS_ANGLE(_variant_)     std::holds_alternative<hg::math::AngleF>(_variant_)
#define HOLDS_VECTOR2F(_variant_)  std::holds_alternative<hg::math::Vector2f>(_variant_)
#define GET_ANGLE(_variant_)       std::get<hg::math::AngleF>(_variant_)
#define GET_ORIENTATION(_variant_) std::get<RelativeIWSliceOrientation>(_variant_)
#define GET_VECTOR2I(_variant_)    std::get<hg::math::Vector2i>(_variant_)
#define GET_VECTOR2F(_variant_)    std::get<hg::math::Vector2f>(_variant_)

AttachmentEvaluation ShipController::evalAttachment(const AttachableGhost& aGhost) {
    const auto& attachable  = aGhost.getAssociatedAttachable();
    const auto* iwSliceData = attachable.getInteriorWorldSliceData();
    HG_VALIDATE_ARGUMENT(
        iwSliceData != nullptr,
        "This overload of evalAttachment() is only valid for AttachableGhosts that carry an IW slice!");

    AttachmentEvaluation result;
    auto&                status = result.status;

    // *** STEP 1: Analyze orientation ***

    const auto relativeRotation = _rotation.shortestDistanceTo(attachable.getPolyShape().getRotation());

    const auto orientationVariant = _checkIWSliceOrientation(*iwSliceData, relativeRotation);
    if (HOLDS_ANGLE(orientationVariant)) {
        status = AttachmentEvaluation::INVALID_ORIENTATION | AttachmentEvaluation::INVALID_POS;
        result.rotationHint = GET_ANGLE(orientationVariant);
        return result;
    } else {
        result.orientation = GET_ORIENTATION(orientationVariant);
    }

    // *** STEP 2: Analyze anchor offset ***

    const auto anchorDiff = _masterData->transformGlobalToShip->transformPoint(
        (attachable.getPolyShape().getAnchor() - _position).cast<float>());

    const auto tlCellMappingVariant =
        _checkIWTopLeftCellMapping(*iwSliceData, anchorDiff, result.orientation);
    if (HOLDS_VECTOR2F(tlCellMappingVariant)) {
        status = AttachmentEvaluation::INVALID_POS;
        result.anchorAdjustmentHint =
            _masterData->transformShipToGlobal->transformPoint(GET_VECTOR2F(tlCellMappingVariant));
        return result;
    } else {
        result.topLeftCellMapping = GET_VECTOR2I(tlCellMappingVariant);
    }

    // *** STEP 3: Analyze cell integration ***

    const auto cornerCellPosInIW =
        result.topLeftCellMapping +
        hg::math::Vector2pz{InteriorWorld::CELL_COUNT_X / 2, InteriorWorld::CELL_COUNT_Y / 2};

    switch (result.orientation) {
    case RelativeIWSliceOrientation::ROT_ALIGNED:
        status |= _checkSliceDataToIWIntegration_rot000(*iwSliceData, cornerCellPosInIW, result.bonds);
        break;
    case RelativeIWSliceOrientation::ROT_90DEG_CCW:
        status |= _checkSliceDataToIWIntegration_rot090(*iwSliceData, cornerCellPosInIW, result.bonds);
        break;
    case RelativeIWSliceOrientation::ROT_180DEG_CCW:
        status |= _checkSliceDataToIWIntegration_rot180(*iwSliceData, cornerCellPosInIW, result.bonds);
        break;
    case RelativeIWSliceOrientation::ROT_270DEG_CCW:
        status |= _checkSliceDataToIWIntegration_rot270(*iwSliceData, cornerCellPosInIW, result.bonds);
        break;
    default:
        HG_UNREACHABLE("Invalid slice orientation! ({})", (int)result.orientation);
    }

    return result;
}

AttachmentEvaluation ShipController::evalAttachment(
    const AttachableGhost&        aGhost,
    const ProjectedCellPositions& aProjectedCellPositions) //
{
    HG_VALIDATE_ARGUMENT(
        aGhost.getAssociatedAttachable().getInteriorWorldSliceData() == nullptr,
        "This overload of evalAttachment() is only valid for AttachableGhosts that carry no IW slice!");

    AttachmentEvaluation result;

    result.orientation = RelativeIWSliceOrientation::NOT_RELEVANT;

    return result;
}

void ShipController::attach(ShipAttachable&    aShipAttachable,
                            hg::math::Vector2f aAnchorOffset,
                            hg::math::AngleF   aRotationOffset) {
    const auto* iwSliceData = aShipAttachable.getInteriorWorldSliceData();
    if (iwSliceData != nullptr) {
        // Since the attachable already has a defined interior world slice and we use a square grid,
        // there are only four valid relative rotations: exactly 0, exactly 90, exactly 180, and exactly
        // 270 (though we check with a small delta due to floating point math). The anchor offset must
        // also be a multiple of the cell resolution.

        const auto orientation = _checkIWSliceOrientation(*iwSliceData, aRotationOffset);
        HG_HARD_ASSERT(orientation != RelativeIWSliceOrientation::INVALID);

        const auto cornerOffset = _checkIWSliceCornerOffset(*iwSliceData, aAnchorOffset, orientation);
        HG_HARD_ASSERT(cornerOffset.has_value());

        const auto cornerCellPosInIW =
            *cornerOffset +
            hg::math::Vector2pz{InteriorWorld::CELL_COUNT_X / 2, InteriorWorld::CELL_COUNT_Y / 2};

        switch (orientation) {
        case RelativeIWSliceOrientation::ROT_ALIGNED:
            _copySliceDataToInteriorWorld_rot000(*iwSliceData, cornerCellPosInIW);
            break;
        case RelativeIWSliceOrientation::ROT_90DEG_CCW:
            _copySliceDataToInteriorWorld_rot090(*iwSliceData, cornerCellPosInIW);
            break;
        case RelativeIWSliceOrientation::ROT_180DEG_CCW:
            _copySliceDataToInteriorWorld_rot180(*iwSliceData, cornerCellPosInIW);
            break;
        case RelativeIWSliceOrientation::ROT_270DEG_CCW:
            _copySliceDataToInteriorWorld_rot270(*iwSliceData, cornerCellPosInIW);
            break;
        default:
            HG_UNREACHABLE("Invalid slice orientation! ({})", (int)orientation);
        }

        _masterData->graphOfAttachables.insert(aShipAttachable);
    } else {
        HG_NOT_IMPLEMENTED("TODO - cell generator func");
    }
}

void ShipController::drawGridOverShape(const PolyShape& aShape, uwga::Canvas& aCanvas) const {
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
                        *_masterData->transformGlobalToShip);
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
        aabbGridTopLeft     = {static_cast<int>(std::floor(aabbTopLeft.x / OVERWORLD_CELL_SIZE)) - 1,
                               static_cast<int>(std::floor(aabbTopLeft.y / OVERWORLD_CELL_SIZE)) - 1};
        aabbGridBottomRight = {static_cast<int>(std::floor(aabbBottomRight.x / OVERWORLD_CELL_SIZE)) + 1,
                               static_cast<int>(std::floor(aabbBottomRight.y / OVERWORLD_CELL_SIZE)) +
                                   1};
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
        {OVERWORLD_CELL_SIZE - 2.f, OVERWORLD_CELL_SIZE - 2.f}
    };
    rect.setOrigin(-1.f, -1.f);
    rect.setRotation(-_rotation);
    rect.setOutlineThickness(2.f);
    rect.setFillColor(uwga::COLOR_TRANSPARENT);

    for (int yy = aabbGridTopLeft.y; yy <= aabbGridBottomRight.y; ++yy) {
        for (int xx = aabbGridTopLeft.x; xx <= aabbGridBottomRight.x; ++xx) {
            auto squareTopLeft = hg::math::Vector2f{xx * OVERWORLD_CELL_SIZE, yy * OVERWORLD_CELL_SIZE};
            const bool isSquareInsideShape =
                isPointInsideShape(squareTopLeft) &&
                isPointInsideShape({(xx + 1) * OVERWORLD_CELL_SIZE, (yy + 0) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(xx + 0) * OVERWORLD_CELL_SIZE, (yy + 1) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(xx + 1) * OVERWORLD_CELL_SIZE, (yy + 1) * OVERWORLD_CELL_SIZE});

            if (isSquareInsideShape) {
                rect.setOutlineColor(uwga::COLOR_LIME.withAlpha(175));
            } else {
                rect.setOutlineColor(uwga::COLOR_ORANGE.withAlpha(100));
            }

            _masterData->transformShipToGlobal->transformPoints(1, &squareTopLeft);
            const auto anchor = squareTopLeft.cast<double>() + _position;
            rect.setAnchor(anchor);

            aCanvas.draw(rect);
        }
    }
}

void ShipController::drawGridOverProjection(const ProjectedCellPositions& aProjectedCellPositions,
                                            uwga::Canvas&                 aCanvas) const {
    uwga::RectangleShape rect{
        aCanvas.getSystem(),
        {OVERWORLD_CELL_SIZE - 2.f, OVERWORLD_CELL_SIZE - 2.f}
    };
    rect.setOrigin(-1.f, -1.f);
    rect.setRotation(-_rotation);
    rect.setOutlineThickness(2.f);
    rect.setFillColor(uwga::COLOR_TRANSPARENT);

    for (int y = 0; y < aProjectedCellPositions.cells.getHeight(); ++y) {
        for (int x = 0; x < aProjectedCellPositions.cells.getWidth(); ++x) {
            const auto squareTopLeft =
                hg::math::Vector2f{(x + aProjectedCellPositions.topLeftPos.x) * OVERWORLD_CELL_SIZE,
                                   (y + aProjectedCellPositions.topLeftPos.y) * OVERWORLD_CELL_SIZE};
            const auto mask = aProjectedCellPositions.cells[y][x];

            if (mask == ProjectedCellPositions::EMPTY) {
                continue;
            }

            if ((mask &
                 (ProjectedCellPositions::COLLIDES_WITH_IW | ProjectedCellPositions::OUT_OF_BOUNDS))) {
                rect.setOutlineColor(uwga::COLOR_ORANGE.withAlpha(100));
            } else {
                rect.setOutlineColor(uwga::COLOR_LIME.withAlpha(175));
            }

            _masterData->transformShipToGlobal->transformPoints(1, &squareTopLeft);
            const auto anchor = squareTopLeft.cast<double>() + _position;
            rect.setAnchor(anchor);

            aCanvas.draw(rect);
        }
    }
}

void ShipController::drawGridOverGhost(const AttachableGhost& aAttachableGhost,
                                       uwga::Canvas&          aCanvas) const {
    drawGridOverProjection(aAttachableGhost.getProjectedCellPositions(), aCanvas);
}

void ShipController::projectCellPositions(const PolyShape&        aShape,
                                          ProjectedCellPositions& aProjectedCellPositions) {
    HG_HARD_ASSERT(aShape.getState() == PolyShape::READY_RELATIVE);

    // Recalculate all shape vertices relative to the ship's center
    auto                            relativeShapeCenter = (aShape.getAnchor() - _position).cast<float>();
    std::vector<hg::math::Vector2f> relativeShapeVertices{};
    {
        relativeShapeVertices.reserve(hg::pztos(aShape.getVertexCount()));
        for (const auto& vert : aShape.getOutputVertices()) {
            relativeShapeVertices.push_back(relativeShapeCenter + vert.cast<float>());
        }
    }

    // Transform all vertices into the ship's coordinate system (center-relative)
    {
        TransformPoints(relativeShapeCenter,
                        relativeShapeVertices.data(),
                        relativeShapeVertices.size(),
                        *_masterData->transformGlobalToShip);
    }

    // Find the AABB of the shape in the ship's coordinate system (center-relative)
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
        aabbGridTopLeft     = {static_cast<int>(std::floor(aabbTopLeft.x / OVERWORLD_CELL_SIZE)) - 1,
                               static_cast<int>(std::floor(aabbTopLeft.y / OVERWORLD_CELL_SIZE)) - 1};
        aabbGridBottomRight = {static_cast<int>(std::floor(aabbBottomRight.x / OVERWORLD_CELL_SIZE)) + 1,
                               static_cast<int>(std::floor(aabbBottomRight.y / OVERWORLD_CELL_SIZE)) +
                                   1};
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
    aProjectedCellPositions.cells.reset(aabbGridBottomRight.x - aabbGridTopLeft.x + 1,
                                        aabbGridBottomRight.y - aabbGridTopLeft.y + 1);
    aProjectedCellPositions.totalBitmask = ProjectedCellPositions::EMPTY;

    const auto&                             ggwld = _masterData->interiorWorld.getUnderlying();
    jbatnozic::gridgoblin::cell::CellKindId cellKindId;
    for (int y = aabbGridTopLeft.y; y <= aabbGridBottomRight.y; ++y) {
        for (int x = aabbGridTopLeft.x; x <= aabbGridBottomRight.x; ++x) {
            const bool isSquareInsideShape =
                isPointInsideShape({(x + 0) * OVERWORLD_CELL_SIZE, (y + 0) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(x + 1) * OVERWORLD_CELL_SIZE, (y + 0) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(x + 0) * OVERWORLD_CELL_SIZE, (y + 1) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(x + 1) * OVERWORLD_CELL_SIZE, (y + 1) * OVERWORLD_CELL_SIZE});

            std::int8_t cellValue = ProjectedCellPositions::EMPTY;
            if (isSquareInsideShape) {
                cellValue |= ProjectedCellPositions::INSIDE_SHAPE;
            }

            // Since the origin of the ship's coordinate system is in the center of the ship, but the
            // origin of the interior world's coordinate system is in its top-left corner, we must
            // offset the X and Y values in order to target the correct IW cells.
            const int iwX = x + (InteriorWorld::CELL_COUNT_X / 2);
            const int iwY = y + (InteriorWorld::CELL_COUNT_Y / 2);
            if (iwX < 0 || iwX >= ggwld.getCellCountX() || iwY < 0 || iwY >= ggwld.getCellCountY()) {
                cellValue |= ProjectedCellPositions::OUT_OF_BOUNDS;
            } else if (ggwld.getCellDataAt(iwX, iwY, &cellKindId) &&
                       cellKindId.value != ToU16(interior::CellArchE::SOLID_VOID)) {
                cellValue |= ProjectedCellPositions::COLLIDES_WITH_IW;
            }

            aProjectedCellPositions.totalBitmask |= cellValue;
            aProjectedCellPositions.cells[y - aabbGridTopLeft.y][x - aabbGridTopLeft.x] = cellValue;
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
        auto& md                 = *_masterData;
        md.transformGlobalToShip = ccomp<GraphicsSystemProvider>().getSystem().createTransform();
        md.transformShipToGlobal = md.transformGlobalToShip->clone();
    }
}

// MARK: ShipController PRIVATE

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

    md.transformGlobalToShip->setToIdentity();
    md.transformGlobalToShip->rotate(_rotation);
    md.transformShipToGlobal->setToInverseOf(*md.transformGlobalToShip);
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
            {OVERWORLD_CELL_SIZE, OVERWORLD_CELL_SIZE}
        };
        rect.setRotation(-_rotation);

        auto flooredMousePosInLocalCoords = hg::math::Vector2f{
            std::floor(_mousePosInLocalCoords.x / OVERWORLD_CELL_SIZE) * OVERWORLD_CELL_SIZE,
            std::floor(_mousePosInLocalCoords.y / OVERWORLD_CELL_SIZE) * OVERWORLD_CELL_SIZE};

        _masterData->transformShipToGlobal->transformPoints(1, &flooredMousePosInLocalCoords);

        const auto anchor = flooredMousePosInLocalCoords.cast<double>() + _position;
        rect.setAnchor(anchor);

        rect.setOutlineThickness(2.f);
        rect.setOutlineColor(uwga::COLOR_RED);
        rect.setFillColor(uwga::COLOR_TRANSPARENT);

        winMgr.getActiveCanvas().draw(rect);
    }
}

std::variant<RelativeIWSliceOrientation, hg::math::AngleF> ShipController::_checkIWSliceOrientation(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::AngleF                              aRelativeRotation) //
{
    constexpr auto delta = ONE_DEG_AS_RAD;

    // If this is 0, the attachable and the ship are axis-aligned.
    const auto lz = (aRelativeRotation + aSlice.rotationOffset).normalize();

    if (IsNearZero(lz.asRad(), delta)) {
        return RelativeIWSliceOrientation::ROT_ALIGNED;
    }
    if (IsNearZero((lz - AngleF::halfCircle() * 0.5f).asRad(), delta)) {
        return RelativeIWSliceOrientation::ROT_90DEG_CCW;
    }
    if (IsNearZero((lz - AngleF::halfCircle()).asRad(), delta)) {
        return RelativeIWSliceOrientation::ROT_180DEG_CCW;
    }
    if (IsNearZero((lz - AngleF::halfCircle() * 1.5f).asRad(), delta)) {
        return RelativeIWSliceOrientation::ROT_270DEG_CCW;
    }

    constexpr hg::math::AngleF angles[] = {hg::math::AngleF::zero(),
                                           hg::math::AngleF::halfCircle() * 0.5f,
                                           hg::math::AngleF::halfCircle(),
                                           hg::math::AngleF::halfCircle() * 1.5f};

    hg::math::AngleF hint = angles[1]; // 90deg clockwise

    for (const auto angle : angles) {
        const auto diff = lz.shortestDistanceTo(angle);
        if (std::abs(diff.asRad()) < std::abs(hint.asRad())) {
            hint = diff;
        }
    }

    return hint;
}

std::variant<hg::math::Vector2i, hg::math::Vector2f> ShipController::_checkIWTopLeftCellMapping(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2f                            aAnchorDiff,
    RelativeIWSliceOrientation                    aOrientation) //
{
    // Note: remember that aSlice.cellGridOffset gives offset from the center of the attachable
    //       to the CENTER of its top-left cell!

    hg::math::Vector2d actualCornerOffset; // Offset when orientation is taken into account
    switch (aOrientation) {
    case RelativeIWSliceOrientation::ROT_ALIGNED:
        actualCornerOffset = aAnchorDiff + aSlice.cellGridOffset;
        break;

    case RelativeIWSliceOrientation::ROT_90DEG_CCW:
        actualCornerOffset =
            aAnchorDiff + hg::math::Vector2f{aSlice.cellGridOffset.y, -aSlice.cellGridOffset.x};
        break;

    case RelativeIWSliceOrientation::ROT_180DEG_CCW:
        actualCornerOffset = aAnchorDiff - aSlice.cellGridOffset;
        break;

    case RelativeIWSliceOrientation::ROT_270DEG_CCW:
        actualCornerOffset =
            aAnchorDiff + hg::math::Vector2f{-aSlice.cellGridOffset.y, aSlice.cellGridOffset.x};
        break;

    default:
        HG_UNREACHABLE("Invalid slice orientation! ({})", (int)aOrientation);
    }

    // Count in cells rather than pixels
    const hg::math::Vector2d ratio = {actualCornerOffset.x / OVERWORLD_CELL_SIZE,
                                      actualCornerOffset.y / OVERWORLD_CELL_SIZE};

    const hg::math::Vector2d floored = {std::floor(ratio.x), std::floor(ratio.y)};

    const auto rfdiff = ratio - floored;

    constexpr double delta = 1.0 / OVERWORLD_CELL_SIZE;

    if (!IsNearZero(rfdiff.x - 0.5, delta)) {
        goto assemble_and_return_hint;
    }
    if (!IsNearZero(rfdiff.y - 0.5, delta)) {
        goto assemble_and_return_hint;
    }

    return floored.cast<int>();

assemble_and_return_hint:
    // clang-format off
    return (hg::math::Vector2d{
        (rfdiff.x <= 0.5) ? (-rfdiff.x) : (1.0 -rfdiff.x),
        (rfdiff.y <= 0.5) ? (-rfdiff.y) : (1.0 -rfdiff.y)
    }.cast<float>()) * OVERWORLD_CELL_SIZE;
    // clang-format on
}

namespace {
template <class taMapCell>
char CheckSliceDataToIWIntegration(
    const InteriorWorld&                                       aWorld,
    const ShipAttachable::InteriorWorldSliceData&              aSlice,
    taMapCell&&                                                aMapCell,
    std::vector<AttachmentEvaluation::BondStrength>& /* out */ aOutBonds) //
{
    char rv = 0;

    for (hg::PZInteger y = 0; y < aSlice.cells.getHeight(); ++y) {
        for (hg::PZInteger x = 0; x < aSlice.cells.getWidth(); ++x) {
            if (aSlice.cells[y][x].cellKindId.value == ToU16(interior::CellArchE::SOLID_VOID)) {
                continue;
            }

            const hg::math::Vector2i dst = aMapCell(x, y);

            // Check for out-of-bounds:

            if (dst.x < 0 || dst.y < 0 || dst.x >= InteriorWorld::CELL_COUNT_X ||
                dst.y >= InteriorWorld::CELL_COUNT_Y) //
            {
                rv |= AttachmentEvaluation::OUT_OF_BOUNDS;
                continue;
            }

            // Check for overlap:

            jbatnozic::gridgoblin::cell::CellKindId cellKindId;
            if (aWorld.getUnderlying().getCellDataAtUnchecked(dst, &cellKindId) &&
                cellKindId.value != ToU16(interior::CellArchE::SOLID_VOID)) //
            {
                rv |= AttachmentEvaluation::OVERLAP;
                continue;
            }

            // Check neighbours for bond strength:

            for (int yOff = -1; yOff <= +1; ++yOff) {
                for (int xOff = -1; xOff <= +1; ++xOff) {
                    if ((yOff == 0 && xOff == 0) || (xOff * yOff != 0)) { // skip self and diagonals
                        continue;
                    }

                    const auto dst2 = hg::math::Vector2i{dst.x + xOff, dst.y + yOff};
                    if (dst2.x < 0 || dst2.y < 0 || dst2.x >= InteriorWorld::CELL_COUNT_X ||
                        dst2.y >= InteriorWorld::CELL_COUNT_Y) //
                    {
                        continue;
                    }

                    jbatnozic::gridgoblin::cell::CellKindId cellKindId2;
                    jbatnozic::gridgoblin::cell::UserData   userData2;
                    if (aWorld.getUnderlying().getCellDataAtUnchecked(dst2, &cellKindId2, &userData2) &&
                        cellKindId2.value != ToU16(interior::CellArchE::SOLID_VOID)) //
                    {
                        const auto p = interior::UserData_GetParentAttachableId(userData2);
                        if (auto iter =
                                std::find_if(aOutBonds.begin(),
                                             aOutBonds.end(),
                                             [p](AttachmentEvaluation::BondStrength aBondStrength) {
                                                 return aBondStrength.attachableId == p;
                                             });
                            iter != aOutBonds.end()) //
                        {
                            iter->attachmentPointCount++;
                        } else {
                            aOutBonds.push_back(AttachmentEvaluation::BondStrength{p, 1});
                        }
                    }
                }
            }
        }
    }

    if (aOutBonds.empty()) {
        rv |= AttachmentEvaluation::NO_CONTACT;
    }

    return rv;
}

using WorldEditor = jbatnozic::gridgoblin::World::Editor;

//! Copies every cell of `aSlice` into `aWorld`. `aMapCell` maps a slice-local cell coordinate
//! (x, y) to its destination cell coordinate in the interior world, thereby accounting for the
//! slice's orientation. Note: only the cell POSITIONS are rotated; the cell contents themselves
//! (e.g. directional wall sprites) are copied verbatim.
template <class taMapCell>
void CopySliceDataToInteriorWorld(InteriorWorld&                                aWorld,
                                  const ShipAttachable::InteriorWorldSliceData& aSlice,
                                  taMapCell&&                                   aMapCell) {
    aWorld.editWorld([&](WorldEditor& aEditor) {
        for (hg::PZInteger y = 0; y < aSlice.cells.getHeight(); ++y) {
            for (hg::PZInteger x = 0; x < aSlice.cells.getWidth(); ++x) {
                if (aSlice.cells[y][x].cellKindId.value == ToU16(interior::CellArchE::SOLID_VOID)) {
                    continue;
                }
                const hg::math::Vector2pz dst = aMapCell(x, y);
                aEditor.setCellDataAt(dst.x,
                                      dst.y,
                                      &aSlice.cells[y][x].cellKindId,
                                      &aSlice.cells[y][x].floorSprite,
                                      &aSlice.cells[y][x].wallSprite,
                                      &aSlice.cells[y][x].spatialInfo,
                                      &aSlice.cells[y][x].userData);
            }
        }
    });
}
} // namespace

char ShipController::_checkSliceDataToIWIntegration_rot000(
    const ShipAttachable::InteriorWorldSliceData&    aSlice,
    hg::math::Vector2pz                              aStartingCorner,
    std::vector<AttachmentEvaluation::BondStrength>& aOutBonds) //
{
    return CheckSliceDataToIWIntegration(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2i {
            return {aStartingCorner.x + x, aStartingCorner.y + y};
        },
        aOutBonds);
}

char ShipController::_checkSliceDataToIWIntegration_rot090(
    const ShipAttachable::InteriorWorldSliceData&    aSlice,
    hg::math::Vector2pz                              aStartingCorner,
    std::vector<AttachmentEvaluation::BondStrength>& aOutBonds) //
{
    return CheckSliceDataToIWIntegration(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2i {
            return {aStartingCorner.x + y, aStartingCorner.y - x};
        },
        aOutBonds);
}

char ShipController::_checkSliceDataToIWIntegration_rot180(
    const ShipAttachable::InteriorWorldSliceData&    aSlice,
    hg::math::Vector2pz                              aStartingCorner,
    std::vector<AttachmentEvaluation::BondStrength>& aOutBonds) //
{
    return CheckSliceDataToIWIntegration(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2i {
            return {aStartingCorner.x - x, aStartingCorner.y - y};
        },
        aOutBonds);
}

char ShipController::_checkSliceDataToIWIntegration_rot270(
    const ShipAttachable::InteriorWorldSliceData&    aSlice,
    hg::math::Vector2pz                              aStartingCorner,
    std::vector<AttachmentEvaluation::BondStrength>& aOutBonds) //
{
    return CheckSliceDataToIWIntegration(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2i {
            return {aStartingCorner.x - y, aStartingCorner.y + x};
        },
        aOutBonds);
}

void ShipController::_copySliceDataToInteriorWorld_rot000(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x + x, aStartingCorner.y + y};
        });
}

void ShipController::_copySliceDataToInteriorWorld_rot090(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x + y, aStartingCorner.y - x};
        });
}

void ShipController::_copySliceDataToInteriorWorld_rot180(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x - x, aStartingCorner.y - y};
        });
}

void ShipController::_copySliceDataToInteriorWorld_rot270(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x - y, aStartingCorner.y + x};
        });
}

// MARK: ShipController SYNC

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

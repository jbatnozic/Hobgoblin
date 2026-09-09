// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Ship_controller.hpp>

#include <Attachable_ghost.hpp>
#include <Graphics_system_provider.hpp>
#include <InteriorWorld/Cell_archs.hpp>
#include <InteriorWorld/Cell_props.hpp>
#include <Ship/Constants.hpp>
#include <Overworld_manager.hpp>

#include <GridGoblin/World/World_config.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Circle_shape.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Rectangle_shape.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>
#include <Hobgoblin/Alvin/Constraint.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace cinnabar {

using hg::math::AngleF;
using hg::math::IsNearZero;

using WorldEditor = jbatnozic::gridgoblin::World::Editor;

namespace {
constexpr float ONE_DEG_AS_RAD = hg::math::DegToRad(1.f);

// Defined further below; forward-declared here because `evalAttachment` (above its definition) uses it.
char CheckProjectionBondStrength(const InteriorWorld&                             aWorld,
                                 const CellFootprint&                             aCellFootprint,
                                 std::vector<AttachmentEvaluation::BondStrength>& aOutBonds);

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

// MARK: ShipController PUBLIC

#define HOLDS_ANGLE(_variant_)     std::holds_alternative<hg::math::AngleF>(_variant_)
#define HOLDS_VECTOR2F(_variant_)  std::holds_alternative<hg::math::Vector2f>(_variant_)
#define GET_ANGLE(_variant_)       std::get<hg::math::AngleF>(_variant_)
#define GET_ORIENTATION(_variant_) std::get<RelativeIWSliceOrientation>(_variant_)
#define GET_VECTOR2I(_variant_)    std::get<hg::math::Vector2i>(_variant_)
#define GET_VECTOR2F(_variant_)    std::get<hg::math::Vector2f>(_variant_)

ShipController::ShipController(QAO_InstGuard aInstGuard, spe::SyncId aSyncId)
    : SyncObjSuper{aInstGuard,
                   QAO_ExeCon::GAMEPLAY,
                   PRIORITY_ENTITIES, // TODO: set in relation to attachables
                   QAO_STATIC_NAME("cinnabar::ShipController"),
                   aSyncId} //
{
    if (isMasterObject()) {
        _masterData->graphOfAttachables.init(*this);
    }
}

void ShipController::init(ShipAttachable& aInitialShipAttachable) {
    HG_VALIDATE_PRECONDITION(isMasterObject());

    const auto* iwSliceData = aInitialShipAttachable.getInteriorWorldSliceData();
    HG_VALIDATE_ARGUMENT(
        iwSliceData != nullptr,
        "A ShipController can only be initialized with an attachable that contains an IW slice!");

    const auto tlCellMappingVariant =
        _checkIWTopLeftCellMapping(*iwSliceData, {}, RelativeIWSliceOrientation::ROT_ALIGNED);
    HG_ASSERT(!HOLDS_VECTOR2F(tlCellMappingVariant));

    const auto tlCellPosInIW =
        GET_VECTOR2I(tlCellMappingVariant) +
        hg::math::Vector2pz{InteriorWorld::CELL_COUNT_X / 2, InteriorWorld::CELL_COUNT_Y / 2};

    const auto id = _masterData->graphOfAttachables.insertInitialAttachable(aInitialShipAttachable);

    _copySliceDataToInteriorWorld_rot000(*iwSliceData, tlCellPosInIW, id);
}

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

AttachmentEvaluation ShipController::evalAttachment(const AttachableGhost& aGhost,
                                                    const CellFootprint&   aCellFootprint) //
{
    HG_VALIDATE_ARGUMENT(
        aGhost.getAssociatedAttachable().getInteriorWorldSliceData() == nullptr,
        "This overload of evalAttachment() is only valid for AttachableGhosts that carry no IW slice!");

    AttachmentEvaluation result;

    // An attachable that carries no interior world slice does not occupy any IW cells of its own, so
    // it can be attached at any position and rotation. The sole requirement is that its footprint
    // makes contact with at least one existing attachable, so only bond strength has to be analyzed.
    result.orientation = RelativeIWSliceOrientation::NOT_RELEVANT;

    result.status =
        CheckProjectionBondStrength(_masterData->interiorWorld, aCellFootprint, result.bonds);

    if (aCellFootprint.totalBitmask & CellFootprint::COLLIDES_WITH_IW) {
        result.status |= AttachmentEvaluation::OVERLAP;
    }

    if (aCellFootprint.totalBitmask & CellFootprint::OUT_OF_BOUNDS) {
        result.status |= AttachmentEvaluation::OUT_OF_BOUNDS;
    }

    return result;
}

void ShipController::attach(AttachableGhost& aGhost, const AttachmentEvaluation& aAttachmentEval) {
    auto& attachable  = aGhost.getAssociatedAttachable();
    auto* iwSliceData = attachable.getInteriorWorldSliceData();

    HG_VALIDATE_ARGUMENT(
        iwSliceData != nullptr,
        "This overload of attach() is only valid for AttachableGhosts that carry an IW slice!");

    HG_VALIDATE_ARGUMENT(aAttachmentEval.status == AttachmentEvaluation::ALL_VALID);

#if 0
    const auto tlCellInIW =
        aAttachmentEval.topLeftCellMapping +
        hg::math::Vector2i{InteriorWorld::CELL_COUNT_X / 2, InteriorWorld::CELL_COUNT_Y / 2};

    const auto id = _masterData->graphOfAttachables.insert(attachable);

    switch (aAttachmentEval.orientation) {
    case RelativeIWSliceOrientation::ROT_ALIGNED:
        _copySliceDataToInteriorWorld_rot000(*iwSliceData, tlCellInIW);
        break;
    case RelativeIWSliceOrientation::ROT_90DEG_CCW:
        _copySliceDataToInteriorWorld_rot090(*iwSliceData, tlCellInIW);
        break;
    case RelativeIWSliceOrientation::ROT_180DEG_CCW:
        _copySliceDataToInteriorWorld_rot180(*iwSliceData, tlCellInIW);
        break;
    case RelativeIWSliceOrientation::ROT_270DEG_CCW:
        _copySliceDataToInteriorWorld_rot270(*iwSliceData, tlCellInIW);
        break;
    default:
        HG_UNREACHABLE("Invalid slice orientation! ({})", (int)aAttachmentEval.orientation);
    }
#endif
}

void ShipController::attach(AttachableGhost&            aGhost,
                            const CellFootprint&        aCellFootprint,
                            const AttachmentEvaluation& aAttachmentEval) {
    auto& attachable = aGhost.getAssociatedAttachable();

    HG_VALIDATE_ARGUMENT(
        aGhost.getAssociatedAttachable().getInteriorWorldSliceData() == nullptr,
        "This overload of attach() is only valid for AttachableGhosts that carry no IW slice!");

    HG_VALIDATE_ARGUMENT(aAttachmentEval.status == AttachmentEvaluation::ALL_VALID);

    HG_ASSERT(aAttachmentEval.orientation == RelativeIWSliceOrientation::NOT_RELEVANT);

    // Align the attachable perfectly with its ghost
    {
        const auto poly = aGhost.getPolyShape();
        auto       body = attachable.getPhysicsBody();
        cpBodySetPosition(body, {poly.getAnchor().x, poly.getAnchor().y});
        // Note: chipmunk physics uses an inverted Y axis compared to the rest of the code,
        //       so we must flip the angle.
        cpBodySetAngle(body, -poly.getRotation().asRadians());
    }

    const auto index =
        _masterData->graphOfAttachables.insertAttachable(attachable,
                                                         aAttachmentEval.bonds,
                                                         ccomp<MOverworld>().getAlvinSpace());
    const auto& grid  = aCellFootprint.cells;

    _masterData->interiorWorld.editWorld([&](WorldEditor& aEditor) {
        for (hg::PZInteger y = 0; y < grid.getHeight(); ++y) {
            for (hg::PZInteger x = 0; x < grid.getWidth(); ++x) {
                const auto cell = grid[y][x];
                if (cell == CellFootprint::EMPTY) {
                    continue;
                }
                HG_ASSERT(cell == CellFootprint::INSIDE_SHAPE);

                const auto iwXY = hg::math::Vector2pz{
                    x + aCellFootprint.topLeftPos.x + InteriorWorld::CELL_COUNT_X / 2,
                    y + aCellFootprint.topLeftPos.y + InteriorWorld::CELL_COUNT_Y / 2};

                // TODO: temp.
                // (implement generator function instead)

                jbatnozic::gridgoblin::cell::CellKindId cellKindId =
                    interior::cell_archetype::METALLIC_FLOOR.cellKindId;

                jbatnozic::gridgoblin::cell::UserData userData;
                interior::UserData_SetParentAttachableId(userData, index);

                aEditor.setCellDataAt(iwXY, &cellKindId);
            }
        }
    });

    // _createConstraintsUponAttach(aGhost, index, aAttachmentEval.bonds);
}

void ShipController::attach(ShipAttachable&    aShipAttachable,
                            hg::math::Vector2f aAnchorOffset,
                            hg::math::AngleF   aRotationOffset) {
#if 0
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
#endif
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

void ShipController::drawGridOverProjection(const CellFootprint& aCellFootprint,
                                            uwga::Canvas&        aCanvas) const {
    uwga::RectangleShape rect{
        aCanvas.getSystem(),
        {OVERWORLD_CELL_SIZE - 2.f, OVERWORLD_CELL_SIZE - 2.f}
    };
    rect.setOrigin(-1.f, -1.f);
    rect.setRotation(-_rotation);
    rect.setOutlineThickness(2.f);
    rect.setFillColor(uwga::COLOR_TRANSPARENT);

    for (int y = 0; y < aCellFootprint.cells.getHeight(); ++y) {
        for (int x = 0; x < aCellFootprint.cells.getWidth(); ++x) {
            const auto squareTopLeft =
                hg::math::Vector2f{(x + aCellFootprint.topLeftPos.x) * OVERWORLD_CELL_SIZE,
                                   (y + aCellFootprint.topLeftPos.y) * OVERWORLD_CELL_SIZE};
            const auto mask = aCellFootprint.cells[y][x];

            if (mask == CellFootprint::EMPTY) {
                continue;
            }

            if ((mask & (CellFootprint::COLLIDES_WITH_IW | CellFootprint::OUT_OF_BOUNDS))) {
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
    drawGridOverProjection(aAttachableGhost.getCellFootprint(), aCanvas);
}

void ShipController::calcFootprint(const PolyShape& aShape, CellFootprint& aCellFootprint) {
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
    aCellFootprint.topLeftPos = aabbGridTopLeft;
    aCellFootprint.cells.reset(aabbGridBottomRight.x - aabbGridTopLeft.x + 1,
                               aabbGridBottomRight.y - aabbGridTopLeft.y + 1);
    aCellFootprint.totalBitmask = CellFootprint::EMPTY;

    const auto&                             ggwld = _masterData->interiorWorld.getUnderlying();
    jbatnozic::gridgoblin::cell::CellKindId cellKindId;
    for (int y = aabbGridTopLeft.y; y <= aabbGridBottomRight.y; ++y) {
        for (int x = aabbGridTopLeft.x; x <= aabbGridBottomRight.x; ++x) {
            const bool isSquareInsideShape =
                isPointInsideShape({(x + 0) * OVERWORLD_CELL_SIZE, (y + 0) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(x + 1) * OVERWORLD_CELL_SIZE, (y + 0) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(x + 0) * OVERWORLD_CELL_SIZE, (y + 1) * OVERWORLD_CELL_SIZE}) &&
                isPointInsideShape({(x + 1) * OVERWORLD_CELL_SIZE, (y + 1) * OVERWORLD_CELL_SIZE});

            if (!isSquareInsideShape) {
                aCellFootprint.cells[y - aabbGridTopLeft.y][x - aabbGridTopLeft.x] =
                    CellFootprint::EMPTY;
                continue;
            }

            std::int8_t cellValue = CellFootprint::INSIDE_SHAPE;

            // Since the origin of the ship's coordinate system is in the center of the ship, but the
            // origin of the interior world's coordinate system is in its top-left corner, we must
            // offset the X and Y values in order to target the correct IW cells.
            const int iwX = x + (InteriorWorld::CELL_COUNT_X / 2);
            const int iwY = y + (InteriorWorld::CELL_COUNT_Y / 2);
            if (iwX < 0 || iwX >= ggwld.getCellCountX() || iwY < 0 || iwY >= ggwld.getCellCountY()) {
                cellValue |= CellFootprint::OUT_OF_BOUNDS;
            } else if (ggwld.getCellDataAt(iwX, iwY, &cellKindId) &&
                       cellKindId.value != ToU16(interior::CellArchE::SOLID_VOID)) {
                cellValue |= CellFootprint::COLLIDES_WITH_IW;
            }

            aCellFootprint.totalBitmask |= cellValue;
            aCellFootprint.cells[y - aabbGridTopLeft.y][x - aabbGridTopLeft.x] = cellValue;
        }
    }
}

const ShipAttachable* ShipController::getAttachableWithIndex(std::int16_t aIndex) const {
    // TODO: temporary implementation
    return &(_masterData->graphOfAttachables.getNode(aIndex)->associatedAttachable);
}

hg::math::Vector2d ShipController::getAnchor() const {
    // TODO: temporary implementation
    return getAttachableWithIndex(0)->getPolyShape().getAnchor();
}

hg::math::AngleF ShipController::getRotation() const {
    // TODO: temporary implementation
    return getAttachableWithIndex(0)->getPolyShape().getRotation();
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
//! Examines the four orthogonal neighbours of the interior-world cell `aCell` and, for every
//! neighbour that belongs to an existing (non-void) attachable, records a bond or increments the
//! attachment point count of an existing bond in `aOutBonds`.
void AccumulateBondsAroundCell(const InteriorWorld&                             aWorld,
                               hg::math::Vector2i                               aCell,
                               std::vector<AttachmentEvaluation::BondStrength>& aOutBonds) {
    constexpr static hg::math::Vector2i CORNER_OFFSETS[4] = {
        {-1, -1},
        {-1, +1},
        {+1, -1},
        {+1, +1}
    };

    for (const auto offsetPack : CORNER_OFFSETS) {
        const auto xOff = offsetPack.x;
        const auto yOff = offsetPack.y;

        const auto dst2 = hg::math::Vector2i{aCell.x + xOff, aCell.y + yOff};
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
            if (auto iter = std::find_if(aOutBonds.begin(),
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

            AccumulateBondsAroundCell(aWorld, dst, aOutBonds);
        }
    }

    if (aOutBonds.empty()) {
        rv |= AttachmentEvaluation::NO_CONTACT;
    }

    return rv;
}

//! Analyzes the bond strength of an attachable that carries no IW slice, described by its projected
//! cell positions. For every projected cell that lies inside the attachable's shape, the neighbouring
//! interior-world cells are examined and bonds to existing attachables are recorded in `aOutBonds`.
//! Returns `NO_CONTACT` if not a single bond was found, or `ALL_VALID` otherwise.
char CheckProjectionBondStrength(const InteriorWorld&                             aWorld,
                                 const CellFootprint&                             aProjection,
                                 std::vector<AttachmentEvaluation::BondStrength>& aOutBonds) {
    // The projection's coordinates are relative to the ship's center, whereas interior-world cell
    // coordinates originate in its top-left corner, so we must offset by half the world dimensions.
    static constexpr hg::math::Vector2i IW_CENTER = {InteriorWorld::CELL_COUNT_X / 2,
                                                     InteriorWorld::CELL_COUNT_Y / 2};

    for (hg::PZInteger y = 0; y < aProjection.cells.getHeight(); ++y) {
        for (hg::PZInteger x = 0; x < aProjection.cells.getWidth(); ++x) {
            if (!(aProjection.cells[y][x] & CellFootprint::INSIDE_SHAPE)) {
                continue;
            }

            const auto dst = hg::math::Vector2i{aProjection.topLeftPos.x + x + IW_CENTER.x,
                                                aProjection.topLeftPos.y + y + IW_CENTER.y};
            if (dst.x < 0 || dst.y < 0 || dst.x >= InteriorWorld::CELL_COUNT_X ||
                dst.y >= InteriorWorld::CELL_COUNT_Y) //
            {
                continue;
            }

            AccumulateBondsAroundCell(aWorld, dst, aOutBonds);
        }
    }

    return aOutBonds.empty() ? AttachmentEvaluation::NO_CONTACT : AttachmentEvaluation::ALL_VALID;
}

//! Copies every cell of `aSlice` into `aWorld`. `aMapCell` maps a slice-local cell coordinate
//! (x, y) to its destination cell coordinate in the interior world, thereby accounting for the
//! slice's orientation. Note: only the cell POSITIONS are rotated; the cell contents themselves
//! (e.g. directional wall sprites) are copied verbatim.
template <class taMapCell>
void CopySliceDataToInteriorWorld(InteriorWorld&                                aWorld,
                                  const ShipAttachable::InteriorWorldSliceData& aSlice,
                                  std::int16_t                                  aAttachableId,
                                  taMapCell&&                                   aMapCell) {
    aWorld.editWorld([&](WorldEditor& aEditor) {
        for (hg::PZInteger y = 0; y < aSlice.cells.getHeight(); ++y) {
            for (hg::PZInteger x = 0; x < aSlice.cells.getWidth(); ++x) {
                if (aSlice.cells[y][x].cellKindId.value == ToU16(interior::CellArchE::SOLID_VOID)) {
                    continue;
                }

                const hg::math::Vector2pz dst = aMapCell(x, y);

                const auto& cell     = aSlice.cells[y][x];
                auto        userData = cell.userData;
                interior::UserData_SetParentAttachableId(userData, aAttachableId);
                aEditor.setCellDataAt(dst.x,
                                      dst.y,
                                      &cell.cellKindId,
                                      &cell.floorSprite,
                                      &cell.wallSprite,
                                      &cell.spatialInfo,
                                      &cell.userData);
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
    hg::math::Vector2pz                           aStartingCorner,
    std::int16_t                                  aAttachableId) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        aAttachableId,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x + x, aStartingCorner.y + y};
        });
}

void ShipController::_copySliceDataToInteriorWorld_rot090(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner,
    std::int16_t                                  aAttachableId) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        aAttachableId,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x + y, aStartingCorner.y - x};
        });
}

void ShipController::_copySliceDataToInteriorWorld_rot180(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner,
    std::int16_t                                  aAttachableId) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        aAttachableId,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x - x, aStartingCorner.y - y};
        });
}

void ShipController::_copySliceDataToInteriorWorld_rot270(
    const ShipAttachable::InteriorWorldSliceData& aSlice,
    hg::math::Vector2pz                           aStartingCorner,
    std::int16_t                                  aAttachableId) //
{
    CopySliceDataToInteriorWorld(
        _masterData->interiorWorld,
        aSlice,
        aAttachableId,
        [aStartingCorner](hg::PZInteger x, hg::PZInteger y) -> hg::math::Vector2pz {
            return {aStartingCorner.x - y, aStartingCorner.y + x};
        });
}

void ShipController::_createConstraintsUponAttach(
    AttachableGhost&                                       aGhost,
    std::int16_t                                           aAttachableId,
    const std::vector<AttachmentEvaluation::BondStrength>& aBonds) //
{
    auto& space = ccomp<MOverworld>().getAlvinSpace();
    auto  body1 = aGhost.getAssociatedAttachable().getPhysicsBody();

    // Align the attachable perfectly with its ghost
    {
        const auto poly = aGhost.getPolyShape();
        cpBodySetPosition(body1, cpv(poly.getAnchor().x, poly.getAnchor().y));
        // Note: chipmunk physics uses an inverted Y axis compared to the rest of the code,
        //       so we must flip the angle.
        cpBodySetAngle(body1, -poly.getRotation().asRadians());
    }

    for (const auto bond : aBonds) {
        const auto* node = _masterData->graphOfAttachables.getNode(bond.attachableId);
        HG_ASSERT(node != nullptr);

        auto body2 = node->associatedAttachable.getPhysicsBody();

        auto* pivot =
            cpPivotJointNew(body1,
                            body2,
                            cpvmult(cpvadd(cpBodyGetPosition(body1), cpBodyGetPosition(body2)), 0.5));

        cpConstraintSetCollideBodies(pivot, cpFalse); // TODO: temporary

        cpFloat       phase = cpBodyGetAngle(body2) - cpBodyGetAngle(body1);
        cpFloat       ratio = 1.0f; // 1:1 angular lock
        cpConstraint* gear  = cpGearJointNew(body1, body2, phase, ratio);

        cpConstraintSetCollideBodies(gear, cpFalse); // TODO: temporary

        space.add(pivot);
        space.add(gear);
    }
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

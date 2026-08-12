// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Poly_shape.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <cstddef>
#include <new>
#include <type_traits>

namespace cinnabar {

namespace {
// The `getOutputVerticesAsCpVect()` accessor relies on `cpVect` and `hg::math::Vector2d` being
// layout-compatible so that the output vertex buffer can be reinterpreted in place. Verify that
// assumption here so any ABI mismatch (e.g. a differently-configured Chipmunk build) is caught
// at compile time rather than becoming silent undefined behaviour at runtime.
using OutputVertex = hg::math::Vector2d;

static_assert(std::is_standard_layout_v<cpVect>, "cpVect must be a standard-layout type.");
static_assert(std::is_standard_layout_v<OutputVertex>, "Vector2d must be a standard-layout type.");
static_assert(sizeof(cpVect) == sizeof(OutputVertex), "cpVect and Vector2d must have the same size.");
static_assert(alignof(cpVect) == alignof(OutputVertex),
              "cpVect and Vector2d must have the same alignment.");
static_assert(offsetof(cpVect, x) == offsetof(OutputVertex, x),
              "The 'x' field of cpVect and Vector2d must be at the same offset.");
static_assert(offsetof(cpVect, y) == offsetof(OutputVertex, y),
              "The 'y' field of cpVect and Vector2d must be at the same offset.");
static_assert(std::is_same_v<decltype(cpVect::x), decltype(OutputVertex::x)>,
              "The 'x' field of cpVect and Vector2d must be of the same type.");
static_assert(std::is_same_v<decltype(cpVect::y), decltype(OutputVertex::y)>,
              "The 'y' field of cpVect and Vector2d must be of the same type.");
} // namespace

PolyShape::PolyShape(hg::PZInteger aVertexCount, hg::math::Vector2d aAnchor, hg::math::AngleF aRotation)
    : _anchor{aAnchor}
    , _rawVertices{hg::pztos(aVertexCount)}
    , _rotation{aRotation}
    , _outputVertices(hg::pztos(aVertexCount)) {}

void PolyShape::setVertexCount(hg::PZInteger aVertexCount) {
    const auto sz = hg::stopz(aVertexCount);
    if (sz == _rawVertices.size()) {
        return;
    }
    const bool shrinking = (sz < _rawVertices.size());
    _rawVertices.resize(sz);
    _outputVertices.resize(sz);
    // Growing appends zero-length vertices, which can never be the farthest; shrinking may drop
    // the vertex that used to be the farthest, so the cached distance must be recomputed.
    if (shrinking) {
        _recalcDistanceToFarthestRawVertex();
    }
}

hg::PZInteger PolyShape::getVertexCount() const {
    return hg::stopz(_rawVertices.size());
}

// MARK: Inputs

void PolyShape::setAnchor(hg::math::Vector2d aAnchor) {
#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
    move(aAnchor - _anchor);
#else
    _anchor = aAnchor;
#endif
}

hg::math::Vector2d PolyShape::getAnchor() const {
    return _anchor;
}

void PolyShape::setRotation(hg::math::AngleF aRotation) {
    if (aRotation == _rotation) {
        return;
    }
    _rotation = aRotation;
    _state    = DIRTY;
}

hg::math::AngleF PolyShape::getRotation() const {
    return _rotation;
}

void PolyShape::setRawVertexAt(hg::PZInteger aIndex, hg::math::Vector2f aVertex) {
    HG_VALIDATE_ARGUMENT(hg::pztos(aIndex) < _rawVertices.size());
    setRawVertexAtUnchecked(aIndex, aVertex);
}

const hg::math::Vector2f& PolyShape::getRawVertexAt(hg::PZInteger aIndex) const {
    HG_VALIDATE_ARGUMENT(hg::pztos(aIndex) < _rawVertices.size());
    return getRawVertexAtUnchecked(aIndex);
}

void PolyShape::setRawVertexAtUnchecked(hg::PZInteger aIndex, hg::math::Vector2f aVertex) {
    auto& vertex = _rawVertices[hg::pztos(aIndex)];
    if (aVertex == vertex) {
        return;
    }
    const float oldDistanceSquared = vertex.lengthSquared();
    vertex                         = aVertex;
    _state                         = DIRTY;

    const float newDistanceSquared = aVertex.lengthSquared();
    if (newDistanceSquared >= _distanceToFarthestRawVertexSquared) {
        // The moved vertex is now (at least tied for) the farthest one.
        _distanceToFarthestRawVertexSquared = newDistanceSquared;
    } else if (oldDistanceSquared >= _distanceToFarthestRawVertexSquared) {
        // The moved vertex used to be (one of) the farthest and moved closer, so the farthest
        // distance may have decreased; recompute it from scratch.
        _recalcDistanceToFarthestRawVertex();
    }
}

const hg::math::Vector2f& PolyShape::getRawVertexAtUnchecked(hg::PZInteger aIndex) const {
    return _rawVertices[hg::pztos(aIndex)];
}

void PolyShape::_recalcDistanceToFarthestRawVertex() {
    float maxLenSq = 0.f;
    for (const auto& vertex : _rawVertices) {
        const auto lenSq = vertex.lengthSquared();
        if (lenSq > maxLenSq) {
            maxLenSq = lenSq;
        }
    }
    _distanceToFarthestRawVertexSquared = maxLenSq;
}

hg::math::Vector2f PolyShape::calculateBaricenterOffset() const {
    const auto count = _rawVertices.size();

    if (count == 0) {
        return {0.f, 0.f};
    }

    // With fewer than 3 vertices, the concept of area is undefined, so do a simple arithmetic mean
    // of the vertices.
    if (count < 3) {
        hg::math::Vector2f sum{0.f, 0.f};
        for (std::size_t i = 0; i < count; ++i) {
            sum += _rawVertices[i];
        }
        return sum / static_cast<float>(count);
    }

    // Area-weighted centroid of a simple polygon with uniform density.
    float              signedAreaX2 = 0.f; // 2 * signed area
    hg::math::Vector2f weightedSum{0.f, 0.f};
    for (std::size_t i = 0; i < count; ++i) {
        const auto& p0 = _rawVertices[i];
        const auto& p1 = _rawVertices[(i + 1) % count];

        const float cross = p0.cross(p1); // p0.x * p1.y - p0.y * p1.x
        signedAreaX2 += cross;
        weightedSum.x += (p0.x + p1.x) * cross;
        weightedSum.y += (p0.y + p1.y) * cross;
    }

    // Degenerate (collinear) polygon; fall back to the arithmetic mean of the vertices.
    if (signedAreaX2 == 0.f) { // TODO: if X < delta
        hg::math::Vector2f sum{0.f, 0.f};
        for (std::size_t i = 0; i < count; ++i) {
            sum += _rawVertices[i];
        }
        return sum / static_cast<float>(count);
    }

    return weightedSum / (3.f * signedAreaX2);
}

hg::PZInteger PolyShape::getShortestRawVertexIndex() const {
    HG_ASSERT(!_rawVertices.empty());

    const auto  count     = _rawVertices.size();
    std::size_t bestIndex = 0;
    auto        bestLenSq = _rawVertices[0].lengthSquared();
    for (std::size_t i = 1; i < count; ++i) {
        const auto lenSq = _rawVertices[i].lengthSquared();
        if (lenSq < bestLenSq) {
            bestLenSq = lenSq;
            bestIndex = i;
        }
    }
    return hg::stopz(bestIndex);
}

hg::PZInteger PolyShape::getLongestRawVertexIndex() const {
    HG_ASSERT(!_rawVertices.empty());

    const auto  count     = _rawVertices.size();
    std::size_t bestIndex = 0;
    auto        bestLenSq = _rawVertices[0].lengthSquared();
    for (std::size_t i = 1; i < count; ++i) {
        const auto lenSq = _rawVertices[i].lengthSquared();
        if (lenSq > bestLenSq) {
            bestLenSq = lenSq;
            bestIndex = i;
        }
    }
    return hg::stopz(bestIndex);
}

// MARK: Outputs

PolyShape::State PolyShape::getState() const {
    return _state;
}

void PolyShape::recalcRel() {
    if (_state == READY_RELATIVE) {
        return;
    }

    const auto count = _rawVertices.size();
    _outputVertices.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        _outputVertices[i] = hg::math::RotateVector(_rawVertices[i], _rotation).cast<double>();
    }
    _state = READY_RELATIVE;
}

#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
void PolyShape::recalcAbs() {
    if (_state == READY_ABSOLUTE) {
        return;
    }

    const auto count = _rawVertices.size();
    _outputVertices.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        _outputVertices[i] = _anchor + hg::math::RotateVector(_rawVertices[i], _rotation).cast<double>();
    }
    _state = READY_ABSOLUTE;
}
#endif

void PolyShape::move(hg::math::Vector2d aDelta) {
    _anchor += aDelta;

#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
    if (_state == READY_ABSOLUTE) {
        for (auto& vertex : _outputVertices) {
            vertex += aDelta;
        }
    }
#endif
}

std::span<const hg::math::Vector2d> PolyShape::getOutputVertices() const {
    return {_outputVertices.data(), _outputVertices.size()};
}

const cpVect* PolyShape::getOutputVerticesAsCpVect() const {
    // Layout compatibility between `cpVect` and `Vector2d` is enforced by the static_asserts above.
    return std::launder(reinterpret_cast<const cpVect*>(_outputVertices.data()));
}

#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
bool PolyShape::intersectsWithPointAbs(hg::math::Vector2d aPoint) const {
    HG_ASSERT(_state == READY_ABSOLUTE);

    const auto vertCount = _outputVertices.size();
    if (vertCount < 3) {
        return false;
    }

    for (std::size_t i = 0; i < vertCount - 1; ++i) {
        if (hg::math::IsPointInsideTriangle<double>(
                aPoint,
                {.a = _anchor, .b = _outputVertices[i], .c = _outputVertices[i + 1]})) {
            return true;
        }
    }

    return hg::math::IsPointInsideTriangle<double>(
        aPoint,
        {.a = _anchor, .b = _outputVertices[vertCount - 1], .c = _outputVertices[0]});
}
#endif

bool PolyShape::intersectsWithPointRel(hg::math::Vector2d aPoint) const {
    HG_ASSERT(_state == READY_RELATIVE);

    const auto vertCount = _outputVertices.size();
    if (vertCount < 3) {
        return false;
    }

    if (aPoint.lengthSquared() > _distanceToFarthestRawVertexSquared) {
        return false;
    }

    for (std::size_t i = 0; i < vertCount - 1; ++i) {
        if (hg::math::IsPointInsideTriangle<double>(
                aPoint,
                {.a = {0.0, 0.0}, .b = _outputVertices[i], .c = _outputVertices[i + 1]})) {
            return true;
        }
    }

    return hg::math::IsPointInsideTriangle<double>(
        aPoint,
        {.a = {0.0, 0.0}, .b = _outputVertices[vertCount - 1], .c = _outputVertices[0]});
}

void PolyShape::debugDraw(hg::uwga::Color               aColor,
                          hg::uwga::Canvas&             aCanvas,
                          const hg::uwga::RenderStates& aRenderStates) const {
    HG_ASSERT(_state == READY_RELATIVE);

    hg::uwga::VertexArray vArr{hg::uwga::PrimitiveType::LINE_STRIP, getVertexCount() + 1, _anchor};

    const auto vertCount = _outputVertices.size();
    for (std::size_t i = 0; i < vertCount; ++i) {
        vArr.vertices[i] = {.position = _outputVertices[i].cast<float>(), .color = aColor};
    }
    vArr.vertices[vertCount] = {.position = _outputVertices[0].cast<float>(), .color = aColor};

    aCanvas.draw(vArr, aRenderStates);
}

} // namespace cinnabar

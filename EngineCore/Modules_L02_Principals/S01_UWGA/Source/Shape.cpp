// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2018 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#include <Hobgoblin/UWGA/Shape.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/System.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>

#include <cassert>
#include <cmath>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

namespace {
// Compute the direction of a segment
math::Vector2f ComputeDirection(math::Vector2f aLhs, math::Vector2f aRhs) {
    math::Vector2f direction = aRhs - aLhs;
    const float    length    = std::sqrt(math::Sqr(direction.x) + math::Sqr(direction.y));
    if (length != 0.f) {
        direction /= length;
    }
    return direction;
}
} // namespace

Shape::Shape(const System& aSystem)
    : Transformable{aSystem.createTransform()} {}

const System& Shape::getSystem() const {
    return getTransform().getSystem();
}

// MARK: Texture

void Shape::setTexture(const Texture* aTexture, bool aResetRect) {
    if (aTexture) {
        // Recompute the texture area if requested, or if there was no texture & rect before
        if (aResetRect || (!_texture && (_textureRect == TextureRect{}))) {
            const auto texSize = aTexture->getSize();
            setTextureRect(
                {0, 0, static_cast<std::uint16_t>(texSize.x), static_cast<std::uint16_t>(texSize.y)});
        }
    }

    // Assign the new texture
    _texture = aTexture;
}

[[nodiscard]] const Texture* Shape::getTexture() const {
    return _texture;
}

void Shape::setTextureRect(TextureRect aTextureRect) {
    _textureRect = aTextureRect;
    _updateTexCoords();
}

[[nodiscard]] TextureRect Shape::getTextureRect() const {
    return _textureRect;
}

// MARK: Fill

void Shape::setFillColor(Color aColor) {
    _fillColor = aColor;
    _updateFillColors();
}

[[nodiscard]] Color Shape::getFillColor() const {
    return _fillColor;
}

// MARK: Outline

void Shape::setOutlineColor(Color aColor) {
    _outlineColor = aColor;
    _updateOutlineColors();
}

[[nodiscard]] Color Shape::getOutlineColor() const {
    return _outlineColor;
}

void Shape::setOutlineThickness(float aThickness) {
    _outlineThickness = aThickness;
    _outlineDirty     = true;
}

[[nodiscard]] float Shape::getOutlineThickness() const {
    return _outlineThickness;
}

void Shape::setMiterLimit(float aMiterLimit) {
    HG_HARD_ASSERT(aMiterLimit >= 1.f &&
                   "Shape::setMiterLimit(float) cannot set miter limit to a value lower than 1.");
    _miterLimit   = aMiterLimit;
    _outlineDirty = true;
}

[[nodiscard]] float Shape::getMiterLimit() const {
    return _miterLimit;
}

// MARK: Points & Bounds

[[nodiscard]] math::Vector2f Shape::getGeometricCenter() const {
    const auto count = getPointCount();

    switch (count) {
    case 0:
        HG_HARD_ASSERT(false && "Cannot calculate geometric center of shape with no points");
        return {};

    case 1:
        return getPoint(0);

    case 2:
        return (getPoint(0) + getPoint(1)) / 2.f;

    default: // more than two points
        math::Vector2f centroid;
        float          twiceArea = 0;

        auto previousPoint = getPoint(count - 1);
        for (PZInteger i = 0; i < count; ++i) {
            const auto  currentPoint = getPoint(i);
            const float product      = previousPoint.cross(currentPoint);
            twiceArea += product;
            centroid += (currentPoint + previousPoint) * product;

            previousPoint = currentPoint;
        }

        if (twiceArea != 0.f) {
            return centroid / 3.f / twiceArea;
        }

        // Fallback for no area - find the center of the bounding box
        auto minPoint = getPoint(0);
        auto maxPoint = minPoint;
        for (PZInteger i = 1; i < count; ++i) {
            const auto currentPoint = getPoint(i);
            minPoint.x              = std::min(minPoint.x, currentPoint.x);
            maxPoint.x              = std::max(maxPoint.x, currentPoint.x);
            minPoint.y              = std::min(minPoint.y, currentPoint.y);
            maxPoint.y              = std::max(maxPoint.y, currentPoint.y);
        }
        return (maxPoint + minPoint) / 2.f;
    }
}

[[nodiscard]] math::Rectangle<float> Shape::getLocalBounds() const {
    _updateIfNeeded();
    return _bounds;
}

[[nodiscard]] math::Rectangle<float> Shape::getGlobalBounds() const {
    return getTransform().transformRect(getLocalBounds());
}

// MARK: Drawable

void Shape::drawOnto(Canvas& aCanvas, const RenderStates& aRenderStates) const {
    _updateIfNeeded();

    RenderStates states{aRenderStates};

    if (!states.transform) {
        states.transform = &getTransform();
    } else {
        if (!_drawTransform) {
            _drawTransform = states.transform->clone();
        } else {
            _drawTransform->setToCopyOf(*states.transform);
        }
        _drawTransform->combine(getTransform());
        states.transform = _drawTransform.get();
    }

    // states.coordinateType = CoordinateType::Pixels;

    // Render the inside
    states.texture = _texture;
    aCanvas.draw(_vertices.vertices.data(),
                 stopz(_vertices.vertices.size()),
                 _vertices.primitiveType,
                 getAnchor(),
                 states);

    // Render the outline
    if (_outlineThickness != 0.f) {
        states.texture = nullptr;
        aCanvas.draw(_outlineVertices.vertices.data(),
                     stopz(_outlineVertices.vertices.size()),
                     _outlineVertices.primitiveType,
                     getAnchor(),
                     states);
    }
}

// MARK: Copying

void Shape::_copyShapeDataFrom(const Shape& aOther) {
    static_cast<Transformable&>(SELF) = static_cast<const Transformable&>(aOther);

    setTexture(aOther.getTexture());
    setTextureRect(aOther.getTextureRect());
    setFillColor(aOther.getFillColor());
    setOutlineColor(aOther.getOutlineColor());
    setOutlineThickness(aOther.getOutlineThickness());
    setMiterLimit(aOther.getMiterLimit());

    _pointsDirty = true;
}

// MARK: Updating

void Shape::_update() const {
    // Get the total number of points of the shape
    const PZInteger count = getPointCount();
    if (count < 3) {
        _vertices.vertices.clear();
        _outlineVertices.vertices.clear();
        return;
    }

    _vertices.vertices.resize(count + 2); // + 2 for center and repeated first point

    // Position
    for (PZInteger i = 0; i < count; ++i) {
        _vertices.vertices[pztos(i + 1)].position = getPoint(i);
    }
    _vertices.vertices[pztos(count + 1)].position = _vertices.vertices[1].position;

    // Update the bounding rectangle
    _vertices.vertices[0] = _vertices.vertices[1]; // so that the result of getBounds() is correct
    _insideBounds         = _vertices.calculateBounds();

    // Compute the center and make it the first vertex
    _vertices.vertices[0].position = math::Vector2f{_insideBounds.x + _insideBounds.w * 0.5f,
                                                    _insideBounds.y + _insideBounds.h * 0.5f};

    // Update stuff
    _updateFillColors();
    _updateTexCoords();
    _updateOutline();
}

void Shape::_updateIfNeeded() const {
    if (_pointsDirty) {
        _update();
        _pointsDirty = _outlineDirty = false;
    } else if (_outlineDirty) {
        _updateOutline();
        _outlineDirty = false;
    }
}

void Shape::_updateFillColors() const {
    for (auto& vertex : _vertices.vertices) {
        vertex.color = _fillColor;
    }
}

void Shape::_updateTexCoords() const {
    const math::Rectangle<float> convertedTextureRect{static_cast<float>(_textureRect.x),
                                                      static_cast<float>(_textureRect.y),
                                                      static_cast<float>(_textureRect.w),
                                                      static_cast<float>(_textureRect.h)};

    // Make sure not to divide by zero when the points are aligned on a vertical or horizontal line
    const math::Vector2f safeInsideSize(_insideBounds.w > 0 ? _insideBounds.w : 1.f,
                                        _insideBounds.h > 0 ? _insideBounds.h : 1.f);

    for (auto& vertex : _vertices.vertices) {
        const math::Vector2f ratio = (vertex.position - math::Vector2f{_insideBounds.x, _insideBounds.y})
                                         .componentWiseDiv(safeInsideSize);
        vertex.texCoords =
            math::Vector2f{convertedTextureRect.x, convertedTextureRect.y} +
            math::Vector2f{convertedTextureRect.w, convertedTextureRect.h}.componentWiseMul(ratio);
    }
}

void Shape::_updateOutline() const {
    // Return if there is no outline or no vertices
    if (_outlineThickness == 0.f || _vertices.vertices.size() < 2) {
        _outlineVertices.vertices.clear();
        _bounds = _insideBounds;
        return;
    }

    const std::size_t count = _vertices.vertices.size() - 2;
    _outlineVertices.vertices.resize((count + 1) *
                                     2); // We need at least that many vertices.
                                         // We will add two more vertices each time we need a bevel.

    // Determine if points are defined clockwise or counterclockwise. This will impact normals
    // computation.
    const bool flipNormals = [this, count]() {
        // p0 is either strictly inside the shape, or on an edge.
        const math::Vector2f p0 = _vertices.vertices[0].position;
        for (std::size_t i = 0; i < count; ++i) {
            const math::Vector2f p1      = _vertices.vertices[i + 1].position;
            const math::Vector2f p2      = _vertices.vertices[i + 2].position;
            const float          product = (p1 - p0).cross(p2 - p0);
            if (product == 0.f) {
                // p0 is on the edge p1-p2. We cannot determine shape orientation yet, so continue.
                continue;
            }
            return product > 0.f;
        }
        return true;
    }();

    std::size_t outlineIndex = 0;
    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t index = i + 1;

        // Get the two segments shared by the current point
        const math::Vector2f p0 =
            (i == 0) ? _vertices.vertices[count].position : _vertices.vertices[index - 1].position;
        const math::Vector2f p1 = _vertices.vertices[index].position;
        const math::Vector2f p2 = _vertices.vertices[index + 1].position;

        // Compute their direction
        const math::Vector2f d1 = ComputeDirection(p0, p1);
        const math::Vector2f d2 = ComputeDirection(p1, p2);

        // Compute their normal pointing towards the outside of the shape
        const math::Vector2f n1 = flipNormals ? (-d1).perpendicular() : d1.perpendicular();
        const math::Vector2f n2 = flipNormals ? (-d2).perpendicular() : d2.perpendicular();

        // Decide whether to add a bevel or not
        const float twoCos2            = 1.f + n1.dot(n2);
        const float squaredLengthRatio = _miterLimit * _miterLimit * twoCos2 / 2.f;
        const bool  isConvexCorner     = n1.dot(n2) * _outlineThickness >= 0.f;
        const bool  needsBevel         = twoCos2 == 0.f || (squaredLengthRatio < 1.f && isConvexCorner);

        if (needsBevel) {
            // Make room for two more vertices
            _outlineVertices.vertices.resize(_outlineVertices.vertices.size() + 2);

            // Combine normals to get bevel edge's direction and normal vector pointing towards the
            // outside of the shape
            const float          twoSin2   = 1.f - n1.dot(n2);
            const math::Vector2f direction = (n2 - n1) / twoSin2; // Length is 1 / sin
            const math::Vector2f extrusion =
                (flipNormals != (n1.dot(n2) >= 0.f) ? direction : -direction).perpendicular();

            // Compute bevel corner position in (direction, extrusion) coordinates
            const float sin = std::sqrt(twoSin2 / 2.f);
            const float u   = _miterLimit * sin;
            const float v   = 1.f - std::sqrt(squaredLengthRatio);

            // Update the outline points
            _outlineVertices.vertices[outlineIndex++].position = p1;
            _outlineVertices.vertices[outlineIndex++].position =
                p1 + (u * extrusion - v * direction) * _outlineThickness;
            _outlineVertices.vertices[outlineIndex++].position = p1;
            _outlineVertices.vertices[outlineIndex++].position =
                p1 + (u * extrusion + v * direction) * _outlineThickness;
        } else {
            // Combine normals to get the extrusion direction
            const math::Vector2f extrusion = (n1 + n2) / twoCos2;

            // Update the outline points
            _outlineVertices.vertices[outlineIndex++].position = p1;
            _outlineVertices.vertices[outlineIndex++].position = p1 + extrusion * _outlineThickness;
        }
    }

    // Duplicate the first point at the end, to close the outline
    _outlineVertices.vertices[outlineIndex++].position = _outlineVertices.vertices[0].position;
    _outlineVertices.vertices[outlineIndex++].position = _outlineVertices.vertices[1].position;

    // Update outline colors
    _updateOutlineColors();

    // Update the shape's bounds
    _bounds = _outlineVertices.calculateBounds();
}

void Shape::_updateOutlineColors() const {
    for (auto& vertex : _outlineVertices.vertices) {
        vertex.color = _outlineColor;
    }
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

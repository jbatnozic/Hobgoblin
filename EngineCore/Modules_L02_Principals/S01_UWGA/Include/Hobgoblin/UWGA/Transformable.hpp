// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_TRANSFORMABLE_HPP
#define UHOBGOBLIN_UWGA_TRANSFORMABLE_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Transform;

//! Decomposed transform defined by a position, a rotation and a scale.
//! \see https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1Transformable.html
class Transformable {
public:
    Transformable(AvoidNull<std::unique_ptr<Transform>> aTransform);

    void setPosition(float aX, float aY);

    void setPosition(const math::Vector2f& aPosition);

    void setRotation(math::AngleF aAngle);

    void setScale(float aFactorX, float aFactorY);

    void setScale(const math::Vector2f& aFactors);

    void setOrigin(float aX, float aY);

    void setOrigin(const math::Vector2f& aOrigin);

    math::Vector2f getPosition() const;

    math::AngleF getRotation() const;

    math::Vector2f getScale() const;

    math::Vector2f getOrigin() const;

    void move(float aOffsetX, float aOffsetY);

    void move(const math::Vector2f& aOffset);

    void rotate(math::AngleF aAngle);

    void scale(float aFactorX, float aFactorY);

    void scale(const math::Vector2f& aFactor);

    const Transform& getTransform() const;

    const Transform& getInverseTransform() const;

private:
    // clang-format off
    math::Vector2f             _origin;                           //!< Origin of translation/rotation/scaling of the object
    math::Vector2f             _position;                         //!< Position of the object in the 2D world
    math::AngleF               _rotation{math::AngleF::zero()};   //!< Orientation of the object
    math::Vector2f             _scale{1, 1};                      //!< Scale of the object
    std::unique_ptr<Transform> _transform;                        //!< Combined transformation of the object
    std::unique_ptr<Transform> _inverseTransform;                 //!< Combined transformation of the object
    mutable bool               _transformNeedUpdate{true};        //!< Does the transform need to be recomputed?
    mutable bool               _inverseTransformNeedUpdate{true}; //!< Does the transform need to be recomputed?
    // clang-format on
};

// MARK: Method definitions

inline void Transformable::setPosition(float aX, float aY) {
    setPosition({aX, aY});
}

inline void Transformable::setPosition(const math::Vector2f& position) {
    _position                   = position;
    _transformNeedUpdate        = true;
    _inverseTransformNeedUpdate = true;
}

inline void Transformable::setRotation(math::AngleF angle) {
    _rotation                   = angle;
    _transformNeedUpdate        = true;
    _inverseTransformNeedUpdate = true;
}

inline void Transformable::setScale(float aFactorX, float aFactorY) {
    setScale({aFactorX, aFactorY});
}

inline void Transformable::setScale(const math::Vector2f& factors) {
    _scale                      = factors;
    _transformNeedUpdate        = true;
    _inverseTransformNeedUpdate = true;
}

inline void Transformable::setOrigin(float aX, float aY) {
    setOrigin({aX, aY});
}

inline void Transformable::setOrigin(const math::Vector2f& origin) {
    _origin                     = origin;
    _transformNeedUpdate        = true;
    _inverseTransformNeedUpdate = true;
}

inline math::Vector2f Transformable::getPosition() const {
    return _position;
}

inline math::AngleF Transformable::getRotation() const {
    return _rotation;
}

inline math::Vector2f Transformable::getScale() const {
    return _scale;
}

inline math::Vector2f Transformable::getOrigin() const {
    return _origin;
}

inline void Transformable::move(float aOffsetX, float aOffsetY) {
    const auto currentPosition = getPosition();
    setPosition(currentPosition.x + aOffsetX, currentPosition.y + aOffsetY);
}

inline void Transformable::move(const math::Vector2f& offset) {
    setPosition(_position + offset);
}

inline void Transformable::rotate(math::AngleF angle) {
    setRotation(_rotation + angle);
}

inline void Transformable::scale(float aFactorX, float aFactorY) {
    setScale({aFactorX, aFactorY});
}

inline void Transformable::scale(const math::Vector2f& factor) {
    setScale({_scale.x * factor.x, _scale.y * factor.y});
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TRANSFORMABLE_HPP

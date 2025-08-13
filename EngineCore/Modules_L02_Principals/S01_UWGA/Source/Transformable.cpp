// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Transform.hpp>
#include <Hobgoblin/UWGA/Transformable.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

Transformable::Transformable(AvoidNull<std::unique_ptr<Transform>> aTransform)
    : _transform{MoveToUnderlying(std::move(aTransform))}
    , _inverseTransform{_transform->clone()} //
{
    _inverseTransform->setToInverse();
}

Transformable::Transformable(const Transformable& aOther)
    : _origin{aOther._origin}
    , _position{aOther._position}
    , _rotation{aOther._rotation}
    , _scale{aOther._scale}
    , _transform{aOther._transform->clone()}
    , _inverseTransform{aOther._inverseTransform->clone()}
    , _transformNeedUpdate{aOther._inverseTransformNeedUpdate}
    , _inverseTransformNeedUpdate{aOther._inverseTransformNeedUpdate} {}

Transformable& Transformable::operator=(const Transformable& aOther) {
    if (&aOther != this) {
        _origin   = aOther._origin;
        _position = aOther._position;
        _rotation = aOther._rotation;
        _scale    = aOther._scale;
        _transform->setToCopyOf(aOther.getTransform());
        _inverseTransform->setToCopyOf(aOther.getInverseTransform());
        _transformNeedUpdate        = aOther._transformNeedUpdate;
        _inverseTransformNeedUpdate = aOther._inverseTransformNeedUpdate;
    }
    return SELF;
}

const Transform& Transformable::getTransform() const {
    // Recompute the combined transform if needed
    if (_transformNeedUpdate) {
        const float angle  = -_rotation.asRadians();
        const float cosine = std::cos(angle);
        const float sine   = std::sin(angle);
        const float sxc    = _scale.x * cosine;
        const float syc    = _scale.y * cosine;
        const float sxs    = _scale.x * sine;
        const float sys    = _scale.y * sine;
        const float tx     = -_origin.x * sxc - _origin.y * sys + _position.x;
        const float ty     = _origin.x * sxs - _origin.y * syc + _position.y;

        // clang-format off
        _transform->setTo3x3Matrix( sxc, sys,  tx,
                                   -sxs, syc,  ty,
                                    0.f, 0.f, 1.f);
        // clang-format on

        _transformNeedUpdate = false;
    }

    return *_transform;
}

const Transform& Transformable::getInverseTransform() const {
    // Recompute the inverse transform if needed
    if (_inverseTransformNeedUpdate) {
        _inverseTransform->setToInverseOf(getTransform());
        _inverseTransformNeedUpdate = false;
    }

    return *_inverseTransform;
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

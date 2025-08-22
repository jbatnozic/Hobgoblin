// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TRANSFORM_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_TRANSFORM_IMPL_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/UWGA/Transform.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLTransformImpl : public Transform {
public:
    SFMLTransformImpl(const System& aSystem);

    SFMLTransformImpl(const System& aSystem, const sf::Transform& aSfmlTransform);

    ~SFMLTransformImpl() override = default;

    sf::RenderStates& getUnderlyingRenderStates() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Transform                                                       //
    ///////////////////////////////////////////////////////////////////////////

    std::unique_ptr<Transform> clone() const override;

    // Setters

    // clang-format off
    void setTo3x3Matrix(float a00, float a01, float a02,
                        float a10, float a11, float a12,
                        float a20, float a21, float a22) override;
    // clang-format on
    void setToIdentity() override;
    void setToCopyOf(const Transform& aOther) override;
    void setToInverse() override;
    void setToInverseOf(const Transform& aOther) override;

    // Applying to objects

    math::Vector2f transformPoint(float aX, float aY) const override;
    math::Vector2f transformPoint(const math::Vector2f& aPoint) const override;

    math::Rectangle<float> transformRect(const math::Rectangle<float>& aRectangle) const override;

    // Mutators

    Transform& combine(const Transform& aOther) override;

    Transform& translate(float aX, float aY) override;
    Transform& translate(math::Vector2f aOffset) override;

    Transform& rotate(float aAngle) override;
    Transform& rotate(math::AngleF aAngle) override;
    Transform& rotate(float aAngle, float aCenterX, float aCenterY) override;
    Transform& rotate(math::AngleF aAngle, float aCenterX, float aCenterY) override;
    Transform& rotate(float aAngle, math::Vector2f aCenter) override;
    Transform& rotate(math::AngleF aAngle, math::Vector2f aCenter) override;

    Transform& scale(float aScaleX, float aScaleY) override;
    Transform& scale(float aScaleX, float aScaleY, float aCenterX, float aCenterY) override;
    Transform& scale(const math::Vector2f& aFactors) override;
    Transform& scale(const math::Vector2f& aFactors, const math::Vector2f& aCenter) override;

    // Comparison

    bool eq(const Transform* aOther) const override;

private:
    const System* _system;

    // Storing the transform as a `sf::RenderStates` object allows us to use it directly during
    // SFML draw calls, instead of copying the transform over to a different `sf::RenderStates` object.
    // It is mutable to allow for translation adjustment (due to anchors).
    mutable sf::RenderStates _renderStates;

    sf::Transform&       _getTransform();
    const sf::Transform& _getTransform() const;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TRANSFORM_IMPL_HPP

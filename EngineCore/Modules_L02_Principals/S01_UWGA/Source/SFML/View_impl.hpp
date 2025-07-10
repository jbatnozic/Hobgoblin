// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_VIEW_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_VIEW_IMPL_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/UWGA/View.hpp>

#include <SFML/Graphics/View.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLViewImpl : public View {
public:
    SFMLViewImpl(const System& aSystem);

    SFMLViewImpl(const System& aSystem, const sf::View& aView, math::Vector2d aAnchor);

    ~SFMLViewImpl() override = default;

    std::unique_ptr<View> clone() const override;

    const sf::View& getUnderlyingView() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: Element                                                         //
    ///////////////////////////////////////////////////////////////////////////

    const System& getSystem() const override;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: View                                                            //
    ///////////////////////////////////////////////////////////////////////////

    // Size

    void           setSize(math::Vector2f aSize) override;
    void           setSize(float aWidth, float aHeight) override;
    math::Vector2f getSize() const override;

    // Viewport

    void                   setViewport(const math::Rectangle<float>& aViewport) override;
    math::Rectangle<float> getViewport() const override;

    // Anchor

    void           setAnchor(math::Vector2d aAnchor) override;
    void           setAnchor(double aX, double aY) override;
    math::Vector2d getAnchor() const override;

    // Center

    void           setCenter(math::Vector2f aCenter) override;
    void           setCenter(float aX, float aY) override;
    math::Vector2f getCenter() const override;

    // Rotation

    void         setRotation(math::AngleF aAngle) override;
    void         rotate(math::AngleF aAngle) override;
    math::AngleF getRotation() const override;

private:
    const System*  _system;
    sf::View       _view;
    math::Vector2d _anchor;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_VIEW_IMPL_HPP

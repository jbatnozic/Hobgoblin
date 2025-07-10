// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef HOBGOBLIN_UGE_VIEW_HPP
#define HOBGOBLIN_UGE_VIEW_HPP

#include <Hobgoblin/UWGA/Element.hpp>

#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! \brief 2D camera that defines what region is shown on screen.
class View : virtual public Element {
public:
    virtual ~View() = default;

    //! Make and return a copy of this object.
    virtual std::unique_ptr<View> clone() const = 0;

    // MARK: Size

    //! \brief Set the size of the view.
    virtual void setSize(math::Vector2f aSize) = 0;

    //! \brief Set the size of the view.
    virtual void setSize(float aWidth, float aHeight) = 0;

    //! \brief Get the size of the view.
    virtual math::Vector2f getSize() const = 0;

    // MARK: Viewport

    //! \brief Set the target viewport
    //!
    //! The viewport is the rectangle into which the contents of the
    //! view are displayed, expressed as a factor (between 0 and 1)
    //! of the size of the RenderTarget to which the view is applied.
    //! For example, a view which takes the left side of the target would
    //! be defined with View.setViewport(sf::FloatRect(0, 0, 0.5, 1)).
    //! By default, a view has a viewport which covers the entire target.
    //!
    //! \param aViewport New viewport rectangle
    virtual void setViewport(const math::Rectangle<float>& aViewport) = 0;

    //! \brief Get the target viewport rectangle of the view
    //!
    //! \return Viewport rectangle, expressed as a factor of the target size
    virtual math::Rectangle<float> getViewport() const = 0;

    // MARK: Anchor

    //! \brief Set the Anchor of the view.
    //!
    //! The Anchor is the position relative to which all other positions are defined.
    //! For example: a view with an anchor at (32, 0) and center at (0, 32) is actually
    //! positioned at (32, 32) in the game world.
    //!
    //! \note the anchor uses 64-bit floats while the center (setCenter() etc.) uses 32-bit floats.
    //!       Therefore it's recommended to leave the center at (0, 0) and only move the anchor if
    //!       you expect your game to use very large coordinates. (a good example is how graphics
    //!       gets very jittery in Minecraft if you walk far enough away from the origin because it
    //!       uses 32-bit floats for rendering.)
    virtual void setAnchor(math::Vector2d aAnchor) = 0;

    virtual void setAnchor(double aX, double aY) = 0;

    virtual math::Vector2d getAnchor() const = 0;

    // MARK: Center

    //! \brief Set the center of the view.
    //! \note the center is relative to the anchor of the view.
    virtual void setCenter(math::Vector2f aCenter) = 0;

    //! \brief Set the center of the view.
    //! \note the center is relative to the anchor of the view.
    virtual void setCenter(float aX, float aY) = 0;

    //! \brief Get the center of the view.
    //! \note the center is relative to the anchor of the view.
    virtual math::Vector2f getCenter() const = 0;

    // MARK: Rotation

    //! \brief Set the orientation of the view.
    //!
    //! The default rotation of a view is 0.
    virtual void setRotation(math::AngleF aAngle) = 0;

    //! \brief Rotate the view relatively to its current orientation (positive values rotate
    //!        counter-clockwise).
    virtual void rotate(math::AngleF aAngle) = 0;

    //! \brief Get the current orientation of the view.
    virtual math::AngleF getRotation() const = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !HOBGOBLIN_UGE_VIEW_HPP

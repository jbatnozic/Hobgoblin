// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef HOBGOBLIN_UGE_VIEW_HPP
#define HOBGOBLIN_UGE_VIEW_HPP

#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

//! \brief 2D camera that defines what region is shown on screen.
class View {
public:
    virtual ~View() = default;

    //! \brief Enable or disable the view.
    virtual void setEnabled(bool aEnabled) = 0;

    //! \brief Check whether the view is enabled or not.
    virtual bool isEnabled() const = 0;

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
    //! \param viewport New viewport rectangle
    //!
    //! \see getViewport
    virtual void setViewport(const math::Rectangle<float>& aViewport) = 0;

    //! \brief Get the target viewport rectangle of the view
    //!
    //! \return Viewport rectangle, expressed as a factor of the target size
    //!
    //! \see setViewport
    virtual math::Rectangle<float> getViewport() const = 0;

    // MARK: Center

    //! \brief Set the center of the view
    virtual void setCenter(math::Vector2f aCenter) = 0;

    //! \brief Set the center of the view
    virtual void setCenter(float aX, float aY) = 0;

    //! \brief Get the center of the view
    virtual math::Vector2f getCenter() const = 0;

    // MARK: Rotation

    //! \brief Set the orientation of the view
    //!
    //! The default rotation of a view is 0 degree.
    //!
    //! \param angle New angle, in degrees
    virtual void setRotation(float aAngle) = 0;

    //! \brief Rotate the view relatively to its current orientation
    //!
    //! \param angle Angle to rotate, in degrees
    virtual void rotate(float aAngle) = 0;

    //! \brief Get the current orientation of the view
    //!
    //! \return Rotation angle of the view, in degrees
    virtual float getRotation() const = 0;
};

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !HOBGOBLIN_UGE_VIEW_HPP

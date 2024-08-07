// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
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

// clang-format off

#ifndef HOBGOBLIN_GRAPHICS_VIEW_HPP
#define HOBGOBLIN_GRAPHICS_VIEW_HPP

#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/Graphics/Transform.hpp>

#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

namespace detail {
class GraphicsImplAccessor;
} // namespace detail

//! \brief 2D camera that defines what region is shown on screen.
class View {
public:
    //! \brief Default constructor
    //!
    //! This constructor creates a default view of (0, 0, 1000, 1000)
    View();

    //! \brief Copy constructor
    View(const View& aOther);

    //! \brief Copy assignment operator
    View& operator=(const View& aOther);

    //! \brief Move constructor
    View(View&& aOther) noexcept;

    //! \brief Move assignment operator
    View& operator=(View&& aOther) noexcept;
    
    //! \brief Construct the view from a rectangle
    //!
    //! \param rectangle Rectangle defining the zone to display
    explicit View(const math::Rectangle<float>& aRectangle);

    //! \brief Construct the view from its center and size
    //!
    //! \param center Center of the zone to display
    //! \param size   Size of zone to display
    View(const math::Vector2f& aCenter, const math::Vector2f& aSize);

    //! \brief Destructor.
    ~View();

    //! \brief Enables or disables the view.
    void setEnabled(bool aEnabled);

    //! \brief Set the center of the view
    //!
    //! \param x X coordinate of the new center
    //! \param y Y coordinate of the new center
    //!
    //! \see setSize, getCenter
    void setCenter(float aX, float aY);

    //! \brief Set the center of the view
    //!
    //! \param center New center
    //!
    //! \see setSize, getCenter
    void setCenter(const math::Vector2f& aCenter);

    //! \brief Set the size of the view
    //!
    //! \param width  New width of the view
    //! \param height New height of the view
    //!
    //! \see setCenter, getCenter
    void setSize(float aWidth, float aHeight);

    //! \brief Set the size of the view
    //!
    //! \param size New size
    //!
    //! \see setCenter, getCenter
    void setSize(const math::Vector2f& aSize);

    //! \brief Set the orientation of the view
    //!
    //! The default rotation of a view is 0 degree.
    //!
    //! \param angle New angle, in degrees
    //!
    //! \see getRotation
    void setRotation(float aAngle);

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
    void setViewport(const math::Rectangle<float>& aViewport);

    //! \brief Reset the view to the given rectangle
    //!
    //! Note that this function resets the rotation angle to 0.
    //!
    //! \param rectangle Rectangle defining the zone to display
    //!
    //! \see setCenter, setSize, setRotation
    void reset(const math::Rectangle<float>& aRectangle);

    //! \brief Checks whether the view is enabled or not.
    bool isEnabled() const;

    //! \brief Get the center of the view
    //!
    //! \return Center of the view
    //!
    //! \see getSize, setCenter
    math::Vector2f getCenter() const;

    //! \brief Get the size of the view
    //!
    //! \return Size of the view
    //!
    //! \see getCenter, setSize
    math::Vector2f getSize() const;

    //! \brief Get the current orientation of the view
    //!
    //! \return Rotation angle of the view, in degrees
    //!
    //! \see setRotation
    float getRotation() const;

    //! \brief Get the target viewport rectangle of the view
    //!
    //! \return Viewport rectangle, expressed as a factor of the target size
    //!
    //! \see setViewport
    const math::Rectangle<float> getViewport() const;

    //! \brief Move the view relatively to its current position
    //!
    //! \param offsetX X coordinate of the move offset
    //! \param offsetY Y coordinate of the move offset
    //!
    //! \see setCenter, rotate, zoom
    void move(float aOffsetX, float aOffsetY);

    //! \brief Move the view relatively to its current position
    //!
    //! \param offset Move offset
    //!
    //! \see setCenter, rotate, zoom
    void move(const math::Vector2f& aOffset);

    //! \brief Rotate the view relatively to its current orientation
    //!
    //! \param angle Angle to rotate, in degrees
    //!
    //! \see setRotation, move, zoom
    void rotate(float aAngle);

    //! \brief Resize the view rectangle relatively to its current size
    //!
    //! Resizing the view simulates a zoom, as the zone displayed on
    //! screen grows or shrinks.
    //! \a factor is a multiplier:
    //! \li 1 keeps the size unchanged
    //! \li > 1 makes the view bigger (objects appear smaller)
    //! \li < 1 makes the view smaller (objects appear bigger)
    //!
    //! \param factor Zoom factor to apply
    //!
    //! \see setSize, move, rotate
    void zoom(float aFactor);

    //! \brief Get the projection transform of the view
    //!
    //! This function is meant for internal use only.
    //!
    //! \return Projection transform defining the view
    //!
    //! \see getInverseTransform
    Transform getTransform() const;

    //! \brief Get the inverse projection transform of the view
    //!
    //! This function is meant for internal use only.
    //!
    //! \return Inverse of the projection transform defining the view
    //!
    //! \see getTransform
    Transform getInverseTransform() const;

private:
    friend class detail::GraphicsImplAccessor;

    void* _getSFMLImpl();
    const void* _getSFMLImpl() const;

    static constexpr std::size_t STORAGE_SIZE  = 168;
    static constexpr std::size_t STORAGE_ALIGN = 4;
    std::aligned_storage<STORAGE_SIZE, STORAGE_ALIGN>::type _storage;

    bool _enabled = true;
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !HOBGOBLIN_GRAPHICS_VIEW_HPP

////////////////////////////////////////////////////////////
/// \class sf::View
/// \ingroup graphics
///
/// sf::View defines a camera in the 2D scene. This is a
/// very powerful concept: you can scroll, rotate or zoom
/// the entire scene without altering the way that your
/// drawable objects are drawn.
///
/// A view is composed of a source rectangle, which defines
/// what part of the 2D scene is shown, and a target viewport,
/// which defines where the contents of the source rectangle
/// will be displayed on the render target (window or texture).
///
/// The viewport allows to map the scene to a custom part
/// of the render target, and can be used for split-screen
/// or for displaying a minimap, for example. If the source
/// rectangle doesn't have the same size as the viewport, its
/// contents will be stretched to fit in.
///
/// To apply a view, you have to assign it to the render target.
/// Then, objects drawn in this render target will be
/// affected by the view until you use another view.
///
/// Usage example:
/// \code
/// sf::RenderWindow window;
/// sf::View view;
///
/// // Initialize the view to a rectangle located at (100, 100) and with a size of 400x200
/// view.reset(sf::FloatRect(100, 100, 400, 200));
///
/// // Rotate it by 45 degrees
/// view.rotate(45);
///
/// // Set its target viewport to be half of the window
/// view.setViewport(sf::FloatRect(0.f, 0.f, 0.5f, 1.f));
///
/// // Apply it
/// window.setView(view);
///
/// // Render stuff
/// window.draw(someSprite);
///
/// // Set the default view back
/// window.setView(window.getDefaultView());
///
/// // Render stuff not affected by the view
/// window.draw(someText);
/// \endcode
///
/// See also the note on coordinates and undistorted rendering in sf::Transformable.
///
/// \see sf::RenderWindow, sf::RenderTexture
///
////////////////////////////////////////////////////////////

// clang-format on

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

// clang-format off

#ifndef UHOBGOBLIN_WINDOW_WINDOW_HPP
#define UHOBGOBLIN_WINDOW_WINDOW_HPP

#include <Hobgoblin/Window/Cursor.hpp>
#include <Hobgoblin/Window/Event.hpp>
#include <Hobgoblin/Window/Context_settings.hpp>
#include <Hobgoblin/Window/Video_mode.hpp>
#include <Hobgoblin/Window/Window_handle.hpp>
#include <Hobgoblin/Window/Window_style.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include <cstdint>
#include <memory>
#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

namespace gr {
namespace detail {
class GraphicsImplAccessor;
} // namespace detail
} // namespace gr

namespace win {

namespace detail {
class WindowImplAccessor;
class WindowPolymorphismAdapter {
public:
    virtual ~WindowPolymorphismAdapter() = default;
    virtual void create(VideoMode mode,
                        const std::string& title,
                        WindowStyle style = WindowStyle::Default,
                        const ContextSettings& settings = ContextSettings()) = 0;
    virtual void create(WindowHandle handle, const ContextSettings& settings = ContextSettings()) = 0;
    virtual void close() = 0;
    virtual void onCreate() = 0;
    virtual void onResize() = 0;
};
} // namespace detail

//! \brief Window that serves as a target for OpenGL rendering.
class Window : NO_COPY, private detail::WindowPolymorphismAdapter {
public:
    //! \brief Default constructor
    //!
    //! This constructor doesn't actually create the window,
    //! use the other constructors or call create() to do so.
    Window();

    //! \brief Construct a new window
    //!
    //! This constructor creates the window with the size and pixel
    //! depth defined in \a mode. An optional style can be passed to
    //! customize the look and behavior of the window (borders,
    //! title bar, resizable, closable, ...). If \a style contains
    //! Style::Fullscreen, then \a mode must be a valid video mode.
    //!
    //! The fourth parameter is an optional structure specifying
    //! advanced OpenGL context settings such as antialiasing,
    //! depth-buffer bits, etc.
    //!
    //! \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
    //! \param title    Title of the window
    //! \param style    Window style, a bitwise OR combination of sf::Style enumerators
    //! \param settings Additional settings for the underlying OpenGL context
    Window(VideoMode aMode,
           const std::string& aTitle,
           WindowStyle aStyle = WindowStyle::Default,
           const ContextSettings& aSettings = ContextSettings());

    //! \brief Construct the window from an existing control
    //!
    //! Use this constructor if you want to create an OpenGL
    //! rendering area into an already existing control.
    //!
    //! The second parameter is an optional structure specifying
    //! advanced OpenGL context settings such as antialiasing,
    //! depth-buffer bits, etc.
    //!
    //! \param handle   Platform-specific handle of the control
    //! \param settings Additional settings for the underlying OpenGL context
    explicit Window(WindowHandle aHandle, const ContextSettings& aSettings = ContextSettings());

    //! \brief Move constructor.
    Window(Window&& aOther);

    //! \brief Move assignment operator.
    Window& operator=(Window&& aOther);

    //! \brief Destructor
    //!
    //! Closes the window and frees all the resources attached to it.
    virtual ~Window();

    //! \brief Create (or recreate) the window
    //!
    //! If the window was already created, it closes it first.
    //! If \a style contains Style::Fullscreen, then \a mode
    //! must be a valid video mode.
    //!
    //! The fourth parameter is an optional structure specifying
    //! advanced OpenGL context settings such as antialiasing,
    //! depth-buffer bits, etc.
    //!
    //! \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
    //! \param title    Title of the window
    //! \param style    %Window style, a bitwise OR combination of sf::Style enumerators
    //! \param settings Additional settings for the underlying OpenGL context
    virtual void create(VideoMode aMode,
                        const std::string& aTitle,
                        WindowStyle aStyle = WindowStyle::Default,
                        const ContextSettings& aSettings = ContextSettings()) override;

    //! \brief Create (or recreate) the window from an existing control
    //!
    //! Use this function if you want to create an OpenGL
    //! rendering area into an already existing control.
    //! If the window was already created, it closes it first.
    //!
    //! The second parameter is an optional structure specifying
    //! advanced OpenGL context settings such as antialiasing,
    //! depth-buffer bits, etc.
    //!
    //! \param handle   Platform-specific handle of the control
    //! \param settings Additional settings for the underlying OpenGL context
    virtual void create(WindowHandle aHandle, const ContextSettings& aSettings = ContextSettings()) override;

    //! \brief Close the window and destroy all the attached resources
    //!
    //! After calling this function, the sf::Window instance remains
    //! valid and you can call create() to recreate the window.
    //! All other functions such as pollEvent() or display() will
    //! still work (i.e. you don't have to test isOpen() every time),
    //! and will have no effect on closed windows.
    virtual void close() override;

    //! \brief Tell whether or not the window is open
    //!
    //! This function returns whether or not the window exists.
    //! Note that a hidden window (setVisible(false)) is open
    //! (therefore this function would return true).
    //!
    //! \return True if the window is open, false if it has been closed
    bool isOpen() const;

    //! \brief Get the settings of the OpenGL context of the window
    //!
    //! Note that these settings may be different from what was
    //! passed to the constructor or the create() function,
    //! if one or more settings were not supported. In this case,
    //! SFML chose the closest match.
    //!
    //! \return Structure containing the OpenGL context settings
    ContextSettings getSettings() const;

    //! \brief Pop the event on top of the event queue, if any, and return it
    //!
    //! This function is not blocking: if there's no pending event then
    //! it will return false and leave \a event unmodified.
    //! Note that more than one event may be present in the event queue,
    //! thus you should always call this function in a loop
    //! to make sure that you process every pending event.
    //! \code
    //! sf::Event event;
    //! while (window.pollEvent(event))
    //! {
    //!    // process event...
    //! }
    //! \endcode
    //!
    //! \param event Event to be returned
    //!
    //! \return True if an event was returned, or false if the event queue was empty
    //!
    //! \see waitEvent
    bool pollEvent(Event& event);

    //! \brief Wait for an event and return it
    //!
    //! This function is blocking: if there's no pending event then
    //! it will wait until an event is received.
    //! After this function returns (and no error occurred),
    //! the \a event object is always valid and filled properly.
    //! This function is typically used when you have a thread that
    //! is dedicated to events handling: you want to make this thread
    //! sleep as long as no new event is received.
    //! \code
    //! sf::Event event;
    //! if (window.waitEvent(event))
    //! {
    //!    // process event...
    //! }
    //! \endcode
    //!
    //! \param event Event to be returned
    //!
    //! \return False if any error occurred
    //!
    //! \see pollEvent
    bool waitEvent(Event& event);

    //! \brief Get the position of the window
    //!
    //! \return Position of the window, in pixels
    //!
    //! \see setPosition
    math::Vector2i getPosition() const;

    //! \brief Change the position of the window on screen
    //!
    //! This function only works for top-level windows
    //! (i.e. it will be ignored for windows created from
    //! the handle of a child window/control).
    //!
    //! \param position New position, in pixels
    //!
    //! \see getPosition
    void setPosition(const math::Vector2i& aPosition);

    //! \brief Get the size of the rendering region of the window
    //!
    //! The size doesn't include the titlebar and borders
    //! of the window.
    //!
    //! \return Size in pixels
    //!
    //! \see setSize
    math::Vector2pz getSize() const;

    //! \brief Change the size of the rendering region of the window
    //!
    //! \param size New size, in pixels
    //!
    //! \see getSize
    void setSize(const math::Vector2pz& aSize);

    //! \brief Change the title of the window
    //!
    //! \param title New title
    //!
    //! \see setIcon
    void setTitle(const std::string& aTitle);

    //! \brief Change the window's icon
    //!
    //! \a pixels must be an array of \a width x \a height pixels
    //! in 32-bits RGBA format.
    //!
    //! The OS default icon is used by default.
    //!
    //! \param width  Icon's width, in pixels
    //! \param height Icon's height, in pixels
    //! \param pixels Pointer to the array of pixels in memory. The
    //!               pixels are copied, so you need not keep the
    //!               source alive after calling this function.
    //!
    //! \see setTitle
    void setIcon(PZInteger aWidth, PZInteger aHeight, const std::uint8_t* aPixels);

    //! \brief Show or hide the window
    //!
    //! The window is shown by default.
    //!
    //! \param visible True to show the window, false to hide it
    void setVisible(bool aVisible);

    //! \brief Enable or disable vertical synchronization
    //!
    //! Activating vertical synchronization will limit the number
    //! of frames displayed to the refresh rate of the monitor.
    //! This can avoid some visual artifacts, and limit the framerate
    //! to a good value (but not constant across different computers).
    //!
    //! Vertical synchronization is disabled by default.
    //!
    //! \param enabled True to enable v-sync, false to deactivate it
    void setVerticalSyncEnabled(bool aEnabled);

    //! \brief Show or hide the mouse cursor
    //!
    //! The mouse cursor is visible by default.
    //!
    //! \param visible True to show the mouse cursor, false to hide it
    void setMouseCursorVisible(bool aVisible);

    //! \brief Grab or release the mouse cursor
    //!
    //! If set, grabs the mouse cursor inside this window's client
    //! area so it may no longer be moved outside its bounds.
    //! Note that grabbing is only active while the window has
    //! focus.
    //!
    //! \param grabbed True to enable, false to disable
    void setMouseCursorGrabbed(bool aGrabbed);

    //! \brief Set the displayed cursor to a native system cursor
    //!
    //! Upon window creation, the arrow cursor is used by default.
    //!
    //! \warning The cursor must not be destroyed while in use by
    //!          the window.
    //!
    //! \warning Features related to Cursor are not supported on
    //!          iOS and Android.
    //!
    //! \param cursor Native system cursor type to display
    //!
    //! \see sf::Cursor::loadFromSystem
    //! \see sf::Cursor::loadFromPixels
    void setMouseCursor(const Cursor& aCursor);

    //! \brief Enable or disable automatic key-repeat
    //!
    //! If key repeat is enabled, you will receive repeated
    //! KeyPressed events while keeping a key pressed. If it is disabled,
    //! you will only get a single event when the key is pressed.
    //!
    //! Key repeat is enabled by default.
    //!
    //! \param enabled True to enable, false to disable
    void setKeyRepeatEnabled(bool aEnabled);

    //! \brief Limit the framerate to a maximum fixed frequency
    //!
    //! If a limit is set, the window will use a small delay after
    //! each call to display() to ensure that the current frame
    //! lasted long enough to match the framerate limit.
    //! SFML will try to match the given limit as much as it can,
    //! but since it internally uses sf::sleep, whose precision
    //! depends on the underlying OS, the results may be a little
    //! unprecise as well (for example, you can get 65 FPS when
    //! requesting 60).
    //!
    //! \param limit Framerate limit, in frames per seconds (use 0 to disable limit)
    void setFramerateLimit(PZInteger aLimit);

    //! \brief Change the joystick threshold
    //!
    //! The joystick threshold is the value below which
    //! no JoystickMoved event will be generated.
    //!
    //! The threshold value is 0.1 by default.
    //!
    //! \param threshold New threshold, in the range [0, 100]
    void setJoystickThreshold(float aThreshold);

    //! \brief Activate or deactivate the window as the current target
    //!        for OpenGL rendering
    //!
    //! A window is active only on the current thread, if you want to
    //! make it active on another thread you have to deactivate it
    //! on the previous thread first if it was active.
    //! Only one window can be active on a thread at a time, thus
    //! the window previously active (if any) automatically gets deactivated.
    //! This is not to be confused with requestFocus().
    //!
    //! \param active True to activate, false to deactivate
    //!
    //! \return True if operation was successful, false otherwise
    bool setActive(bool aActive = true);

    //! \brief Request the current window to be made the active
    //!        foreground window
    //!
    //! At any given time, only one window may have the input focus
    //! to receive input events such as keystrokes or mouse events.
    //! If a window requests focus, it only hints to the operating
    //! system, that it would like to be focused. The operating system
    //! is free to deny the request.
    //! This is not to be confused with setActive().
    //!
    //! \see hasFocus
    void requestFocus();

    //! \brief Check whether the window has the input focus
    //!
    //! At any given time, only one window may have the input focus
    //! to receive input events such as keystrokes or most mouse
    //! events.
    //!
    //! \return True if window has focus, false otherwise
    //! \see requestFocus
    bool hasFocus() const;

    //! \brief Display on screen what has been rendered to the window so far
    //!
    //! This function is typically called after all OpenGL rendering
    //! has been done for the current frame, in order to show
    //! it on screen.
    void display();

    //! \brief Get the OS-specific handle of the window
    //!
    //! The type of the returned handle is sf::WindowHandle,
    //! which is a typedef to the handle type defined by the OS.
    //! You shouldn't need to use this function, unless you have
    //! very specific stuff to implement that SFML doesn't support,
    //! or implement a temporary workaround until a bug is fixed.
    //!
    //! \return System handle of the window
    WindowHandle getSystemHandle() const;

protected:
    //! \brief Function called after the window has been created
    //!
    //! This function is called so that derived classes can
    //! perform their own specific initialization as soon as
    //! the window is created.
    void onCreate() override;

    //! \brief Function called after the window has been resized
    //!
    //! This function is called so that derived classes can
    //! perform custom actions when the size of the window changes.
    void onResize() override;

private:
    friend class gr::detail::GraphicsImplAccessor;
    friend class detail::WindowImplAccessor;

    void* _getSFMLImpl();
    const void* _getSFMLImpl() const;

    class Impl;
    std::unique_ptr<Impl> _impl;
};

math::Vector2i GetMousePositionRelativeToWindow(const Window& aWindow);

} // namespace win
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_WINDOW_WINDOW_HPP

////////////////////////////////////////////////////////////
/// \class sf::Window
/// \ingroup window
///
/// sf::Window is the main class of the Window module. It defines
/// an OS window that is able to receive an OpenGL rendering.
///
/// A sf::Window can create its own new window, or be embedded into
/// an already existing control using the create(handle) function.
/// This can be useful for embedding an OpenGL rendering area into
/// a view which is part of a bigger GUI with existing windows,
/// controls, etc. It can also serve as embedding an OpenGL rendering
/// area into a window created by another (probably richer) GUI library
/// like Qt or wxWidgets.
///
/// The sf::Window class provides a simple interface for manipulating
/// the window: move, resize, show/hide, control mouse cursor, etc.
/// It also provides event handling through its pollEvent() and waitEvent()
/// functions.
///
/// Note that OpenGL experts can pass their own parameters (antialiasing
/// level, bits for the depth and stencil buffers, etc.) to the
/// OpenGL context attached to the window, with the sf::ContextSettings
/// structure which is passed as an optional argument when creating the
/// window.
///
/// On dual-graphics systems consisting of a low-power integrated GPU
/// and a powerful discrete GPU, the driver picks which GPU will run an
/// SFML application. In order to inform the driver that an SFML application
/// can benefit from being run on the more powerful discrete GPU,
/// #SFML_DEFINE_DISCRETE_GPU_PREFERENCE can be placed in a source file
/// that is compiled and linked into the final application. The macro
/// should be placed outside of any scopes in the global namespace.
///
/// Usage example:
/// \code
/// // Declare and create a new window
/// sf::Window window(sf::VideoMode(800, 600), "SFML window");
///
/// // Limit the framerate to 60 frames per second (this step is optional)
/// window.setFramerateLimit(60);
///
/// // The main loop - ends as soon as the window is closed
/// while (window.isOpen())
/// {
///    // Event processing
///    sf::Event event;
///    while (window.pollEvent(event))
///    {
///        // Request for closing the window
///        if (event.type == sf::Event::Closed)
///            window.close();
///    }
///
///    // Activate the window for OpenGL rendering
///    window.setActive();
///
///    // OpenGL drawing commands go here...
///
///    // End the current frame and display its contents on screen
///    window.display();
/// }
/// \endcode
///
////////////////////////////////////////////////////////////

// clang-format on

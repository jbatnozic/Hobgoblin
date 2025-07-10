// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_WINDOW_HPP
#define UHOBGOBLIN_UWGA_WINDOW_HPP

#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Element.hpp>
#include <Hobgoblin/UWGA/Window_event.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! \brief Window that serves as a target for OpenGL rendering.
class Window : virtual public Element {
public:
    virtual ~Window() = default;

    // MARK: Title

    //! \brief Change the title of the window
    //!
    //! \param title New title
    //!
    //! \see setIcon
    virtual void setTitle(const UnicodeString& aTitle) = 0;

    // MARK: Event handling

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
    virtual bool pollEvent(WindowEvent& aEvent) = 0;

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
    virtual bool waitEvent(WindowEvent& aEvent) = 0;

    // MARK: Size

    //! \brief Change the size of the rendering region of the window
    //!
    //! \param size New size, in pixels
    //!
    //! \see getSize
    virtual void setSize(math::Vector2pz aSize) = 0;

    //! \brief Get the size of the rendering region of the window
    //!
    //! The size doesn't include the titlebar and borders
    //! of the window.
    //!
    //! \return Size in pixels
    //!
    //! \see setSize
    virtual math::Vector2pz getSize() const = 0;

    // MARK: Position

    //! \brief Change the position of the window on screen
    //!
    //! This function only works for top-level windows
    //! (i.e. it will be ignored for windows created from
    //! the handle of a child window/control).
    //!
    //! \param position New position, in pixels
    //!
    //! \see getPosition
    virtual void setPosition(math::Vector2i aPosition) = 0;

    //! \brief Get the position of the window
    //!
    //! \return Position of the window, in pixels
    //!
    //! \see setPosition
    virtual math::Vector2i getPosition() const = 0;

    // MARK: Focus

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
    virtual void requestFocus() = 0;

    //! \brief Check whether the window has the input focus
    //!
    //! At any given time, only one window may have the input focus
    //! to receive input events such as keystrokes or most mouse
    //! events.
    //!
    //! \return True if window has focus, false otherwise
    //! \see requestFocus
    virtual bool hasFocus() const = 0;

    // MARK: Cursor

    //! \brief Show or hide the mouse cursor
    //!
    //! The mouse cursor is visible by default.
    //!
    //! \param visible True to show the mouse cursor, false to hide it
    virtual void setMouseCursorVisible(bool aVisible) = 0;

    //! \brief Grab or release the mouse cursor
    //!
    //! If set, grabs the mouse cursor inside this window's client
    //! area so it may no longer be moved outside its bounds.
    //! Note that grabbing is only active while the window has
    //! focus.
    //!
    //! \param grabbed True to enable, false to disable
    virtual void setMouseCursorGrabbed(bool aGrabbed) = 0;

    // MARK: Contents

    //! \brief Display on screen what has been rendered to the window so far
    //!
    //! This function is typically called after all OpenGL rendering
    //! has been done for the current frame, in order to show
    //! it on screen.
    virtual void display() = 0;

    // MARK: Other

    //! \brief Enable or disable automatic key-repeat
    //!
    //! If key repeat is enabled, you will receive repeated
    //! KeyPressed events while keeping a key pressed. If it is disabled,
    //! you will only get a single event when the key is pressed.
    //!
    //! Key repeat is enabled by default.
    //!
    //! \param enabled True to enable, false to disable
    virtual void setKeyRepeatEnabled(bool aEnabled) = 0;

    //! \brief Show or hide the window
    //!
    //! The window is shown by default.
    //!
    //! \param visible True to show the window, false to hide it
    virtual void setVisible(bool aVisible) = 0;

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
    virtual void setVerticalSyncEnabled(bool aEnabled) = 0;

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
    virtual void setFramerateLimit(PZInteger aLimit) = 0;
};

math::Vector2i GetMousePositionRelativeToWindow(const Window& aWindow);

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_WINDOW_HPP

#include <Hobgoblin/Graphics/Window.hpp>

#include <SFML/Window/Window.hpp>

#include "SFML_conversions.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

using ImplType = sf::Window;
constexpr auto IMPL_SIZE  = sizeof(ImplType);
constexpr auto IMPL_ALIGN = alignof(ImplType);
#define  IMPLOF(_obj_) (reinterpret_cast<ImplType*>(&((_obj_)._storage)))
#define CIMPLOF(_obj_) (reinterpret_cast<const ImplType*>(&((_obj_)._storage)))
#define  SELF_IMPL (IMPLOF(SELF))
#define SELF_CIMPL (CIMPLOF(SELF))

Window::Window() {
    static_assert(STORAGE_SIZE  == IMPL_SIZE,  "Window::STORAGE_SIZE is inadequate.");
    static_assert(STORAGE_ALIGN == IMPL_ALIGN, "Window::STORAGE_ALIGN is inadequate.");

    new (&_storage) ImplType();
}

Window::Window(VideoMode aMode, const std::string& aTitle, WindowStyle aStyle, const ContextSettings& aSettings) {
    new (&_storage) ImplType(ToSf(aMode), aTitle, ToSf(aStyle), ToSf(aSettings));
}

Window::Window(WindowHandle aHandle, const ContextSettings& aSettings) {
    new (&_storage) ImplType(aHandle, ToSf(aSettings)); 
}

Window::~Window() {
    SELF_IMPL->~ImplType();
}

void Window::create(VideoMode aMode, const std::string& aTitle, WindowStyle aStyle, const ContextSettings& aSettings) {
    SELF_IMPL->create(ToSf(aMode), aTitle, ToSf(aStyle), ToSf(aSettings));
}

void Window::create(WindowHandle aHandle, const ContextSettings& aSettings) {
    SELF_IMPL->create(aHandle, ToSf(aSettings));
}

void Window::close() {
    SELF_IMPL->close();
}

bool Window::isOpen() const {
    return SELF_CIMPL->isOpen();
}

ContextSettings Window::getSettings() const {
    return ToHg(SELF_CIMPL->getSettings());
}

bool Window::pollEvent(Event& aEvent) {
    // TODO
    return false;
}

bool Window::waitEvent(Event& aEvent) {
    // TODO
    return false;
}

math::Vector2i Window::getPosition() const {
    return ToHg(SELF_CIMPL->getPosition());
}

void Window::setPosition(const math::Vector2i& aPosition) {
    // TODO
}

math::Vector2pz Window::getSize() const {
    const auto size = SELF_CIMPL->getSize();
    return {
        static_cast<PZInteger>(size.x),
        static_cast<PZInteger>(size.y)
    };
}

void Window::setSize(const math::Vector2pz& aSize) {
    // TODO
}

void Window::setTitle(const std::string& aTitle) {
    // TODO
}

void Window::setIcon(unsigned int aWidth, unsigned int aHeight, const std::uint8_t* aPixels) {
    // TODO
}

void Window::setVisible(bool aVisible) {
    SELF_IMPL->setVisible(aVisible);
}

void Window::setVerticalSyncEnabled(bool aEnabled) {
    SELF_IMPL->setVerticalSyncEnabled(aEnabled);
}

void Window::setMouseCursorVisible(bool aVisible) {
    SELF_IMPL->setMouseCursorVisible(aVisible);
}

void Window::setMouseCursorGrabbed(bool aGrabbed) {
    SELF_IMPL->setMouseCursorGrabbed(aGrabbed);
}

void Window::setMouseCursor(const Cursor& aCursor) {
    // TODO
}

void Window::setKeyRepeatEnabled(bool aEnabled) {
    SELF_IMPL->setKeyRepeatEnabled(aEnabled);
}

void Window::setFramerateLimit(unsigned int limit) {
    // TODO
}

void Window::setJoystickThreshold(float aThreshold) {
    SELF_IMPL->setJoystickThreshold(aThreshold);
}

bool Window::setActive(bool aActive) const {
    return SELF_CIMPL->setActive(aActive);
}

void Window::requestFocus() {
    SELF_IMPL->requestFocus();
}

bool Window::hasFocus() const {
    return SELF_CIMPL->hasFocus();
}

void Window::display() {
    SELF_IMPL->display();
}

WindowHandle Window::getSystemHandle() const {
    return SELF_CIMPL->getSystemHandle();
}

void* Window::_getSFMLImpl() {
    return SELF_IMPL;
}

const void* Window::_getSFMLImpl() const {
    return SELF_CIMPL;
}

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
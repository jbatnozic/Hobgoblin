// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Input/Private/Conversions.hpp>
#include <Hobgoblin/UWGA/Window_event.hpp>

#include <SFML/Window/Event.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

inline WindowEvent ToHg(const sf::Event& aEvent) {
    switch (aEvent.type) {
    case sf::Event::Closed:
        return {WindowEvent::Closed{}};

    case sf::Event::Resized:
        {
            WindowEvent::Resized data;
            data.width  = static_cast<PZInteger>(aEvent.size.width);
            data.height = static_cast<PZInteger>(aEvent.size.height);
            return {data};
        }

    case sf::Event::LostFocus:
        return {WindowEvent::LostFocus{}};

    case sf::Event::GainedFocus:
        return {WindowEvent::GainedFocus{}};

    case sf::Event::TextEntered:
        return {WindowEvent::TextEntered{aEvent.text.unicode}};

    case sf::Event::KeyPressed:
        {
            WindowEvent::KeyPressed data;
            data.virtualKey  = in::VirtualKeyboardKey{static_cast<in::UniversalInputEnum>(
                in::detail::SfKeyboardKeyToInputEnum(aEvent.key.code))};
            data.physicalKey = in::PhysicalKeyboardKey{static_cast<in::UniversalInputEnum>(
                in::detail::SfKeyboardScancodeToInputEnum(aEvent.key.scancode))};
            data.alt         = aEvent.key.alt;
            data.control     = aEvent.key.control;
            data.shift       = aEvent.key.shift;
            data.system      = aEvent.key.system;
            return {data};
        }

    case sf::Event::KeyReleased:
        {
            WindowEvent::KeyReleased data;
            data.virtualKey  = in::VirtualKeyboardKey{static_cast<in::UniversalInputEnum>(
                in::detail::SfKeyboardKeyToInputEnum(aEvent.key.code))};
            data.physicalKey = in::PhysicalKeyboardKey{static_cast<in::UniversalInputEnum>(
                in::detail::SfKeyboardScancodeToInputEnum(aEvent.key.scancode))};
            data.alt         = aEvent.key.alt;
            data.control     = aEvent.key.control;
            data.shift       = aEvent.key.shift;
            data.system      = aEvent.key.system;
            return {data};
        }

    case sf::Event::MouseWheelMoved:
        return {WindowEvent::Unknown{}}; // MouseWheelMoved is deprecated

    case sf::Event::MouseWheelScrolled:
        {
            WindowEvent::MouseWheelScrolled data;
            data.wheel = in::MouseWheel{static_cast<in::UniversalInputEnum>(
                in::detail::SfMouseWheelToInputEnum(aEvent.mouseWheelScroll.wheel))};
            data.delta = aEvent.mouseWheelScroll.delta;
            data.x     = aEvent.mouseWheelScroll.x;
            data.y     = aEvent.mouseWheelScroll.y;
            return {data};
        }

    case sf::Event::MouseButtonPressed:
        {
            WindowEvent::MouseButtonPressed data;
            data.button = in::MouseButton{static_cast<in::UniversalInputEnum>(
                in::detail::SfMouseButtonToInputEnum(aEvent.mouseButton.button))};
            data.x      = aEvent.mouseButton.x;
            data.y      = aEvent.mouseButton.y;
            return {data};
        }

    case sf::Event::MouseButtonReleased:
        {
            WindowEvent::MouseButtonReleased data;
            data.button = in::MouseButton{static_cast<in::UniversalInputEnum>(
                in::detail::SfMouseButtonToInputEnum(aEvent.mouseButton.button))};
            data.x      = aEvent.mouseButton.x;
            data.y      = aEvent.mouseButton.y;
            return {data};
        }

    case sf::Event::MouseMoved:
        {
            WindowEvent::MouseMoved data;
            data.x = aEvent.mouseMove.x;
            data.y = aEvent.mouseMove.y;
            return {data};
        }

    case sf::Event::MouseEntered:
        return {WindowEvent::MouseEntered{}};

    case sf::Event::MouseLeft:
        return {WindowEvent::MouseLeft{}};

#if 0
        case sf::Event::JoystickButtonPressed: // TODO
        case sf::Event::JoystickButtonReleased: // TODO
        case sf::Event::JoystickMoved: // TODO
        case sf::Event::JoystickConnected: // TODO
        case sf::Event::JoystickDisconnected: // TODO

        case sf::Event::TouchBegan: // TODO
        case sf::Event::TouchMoved: // TODO
        case sf::Event::TouchEnded: // TODO

        case sf::Event::SensorChanged: // TODO
#endif

    default:
        return {WindowEvent::Unknown{}};
    }
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Input/Virtual_keyboard_key.hpp>

#include <SFML/Window/Keyboard.hpp>

#include "SFML_conversions.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace in {

bool CheckPressedVK(VirtualKeyboardKey aKey) {
    if (aKey.val() == VK_UNKNOWN) {
        return false;
    }
    return sf::Keyboard::isKeyPressed((sf::Keyboard::Key)ToSfKeyboardKey(aKey));
}

std::optional<VirtualKeyboardKey> DetectPressedVK() {
    for (int i = HG_INPUT_VIRTUALKEY_FIRST; i <= HG_INPUT_VIRTUALKEY_LAST; i += 1) {
        const auto key = VirtualKeyboardKey{static_cast<UniversalInputEnum>(i)};
        if (CheckPressedVK(key)) {
            return key;
        }
    }
    return {};
}

} // namespace in
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

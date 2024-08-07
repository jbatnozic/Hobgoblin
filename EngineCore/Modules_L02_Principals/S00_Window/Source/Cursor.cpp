// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#include <Hobgoblin/Window/Cursor.hpp>

#include <SFML/Window/Cursor.hpp>

#include <new>

#include "SFML_conversions.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace win {

using ImplType = sf::Cursor;
constexpr auto IMPL_SIZE  = sizeof(ImplType);
constexpr auto IMPL_ALIGN = alignof(ImplType);
#define  IMPLOF(_obj_) (reinterpret_cast<ImplType*>(&((_obj_)._storage)))
#define CIMPLOF(_obj_) (reinterpret_cast<const ImplType*>(&((_obj_)._storage)))
#define  SELF_IMPL (IMPLOF(SELF))
#define SELF_CIMPL (CIMPLOF(SELF))

Cursor::Cursor() {
    static_assert(STORAGE_SIZE  == IMPL_SIZE,  "VertexBuffer::STORAGE_SIZE is inadequate.");
    static_assert(STORAGE_ALIGN == IMPL_ALIGN, "VertexBuffer::STORAGE_ALIGN is inadequate.");

    new (&_storage) ImplType();
}

#if 0 // TODO(enable when SFML supports it)
Cursor::Cursor(Cursor&& aOther) {
    auto& sfCursor = detail::WindowImplAccessor::getImplOf<sf::Cursor>(aOther);
    new (&_storage) ImplType(std::move(*sfCursor));
}

Cursor& Cursor::operator=(Cursor&& aOther) {

}
#endif

Cursor::~Cursor() {
    SELF_IMPL->~ImplType();
}

bool Cursor::loadFromPixels(const std::uint8_t* aPixels, math::Vector2pz aSize, math::Vector2pz aHotspot) {
    return SELF_IMPL->loadFromPixels(
        reinterpret_cast<const sf::Uint8*>(aPixels),
        { static_cast<unsigned>(aSize.x), static_cast<unsigned>(aSize.y) },
        { static_cast<unsigned>(aHotspot.x), static_cast<unsigned>(aHotspot.y) }
    );
}

bool Cursor::loadFromSystem(Type aType) {
    return SELF_IMPL->loadFromSystem(ToSf(aType));
}

void* Cursor::_getSFMLImpl() {
    return SELF_IMPL;
}

const void* Cursor::_getSFMLImpl() const {
    return SELF_CIMPL;
}

} // namespace win
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// clang-format on

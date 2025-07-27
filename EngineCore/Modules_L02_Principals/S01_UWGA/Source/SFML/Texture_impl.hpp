// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TEXTURE_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_TEXTURE_IMPL_HPP

#include <Hobgoblin/UWGA/Texture.hpp>

#include <SFML/Graphics/Texture.hpp>

#include "Texture_common_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLTextureImpl : public SFMLTextureCommonImpl<sf::Texture> {
public:
    using SFMLTextureCommonImpl<sf::Texture>::SFMLTextureCommonImpl;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TEXTURE_IMPL_HPP

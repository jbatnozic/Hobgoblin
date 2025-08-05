// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_RENDER_TEXTURE_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_RENDER_TEXTURE_IMPL_HPP

#include <Hobgoblin/UWGA/Texture.hpp>

#include <SFML/Graphics/RenderTexture.hpp>

#include "Texture_common_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLRenderTextureImpl : public SFMLTextureCommonImpl<sf::RenderTexture> {
public:
    using SFMLTextureCommonImpl<sf::RenderTexture>::SFMLTextureCommonImpl;

    // TODO: Add drawing methods
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_RENDER_TEXTURE_IMPL_HPP

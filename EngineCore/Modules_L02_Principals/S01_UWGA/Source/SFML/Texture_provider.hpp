// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_TEXTURE_PROVIDER_HPP
#define UHOBGOBLIN_UWGA_SFML_TEXTURE_PROVIDER_HPP

#include <Hobgoblin/UWGA/Render_texture.hpp>

#include <SFML/Graphics/Texture.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLTextureProvider : public RenderTexture {
public:
    virtual sf::Texture&       getUnderlyingTexture()       = 0;
    virtual const sf::Texture& getUnderlyingTexture() const = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_TEXTURE_PROVIDER_HPP

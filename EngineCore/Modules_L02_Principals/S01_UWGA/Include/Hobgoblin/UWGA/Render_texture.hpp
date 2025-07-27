// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_RENDER_TEXTURE_HPP
#define UHOBGOBLIN_UWGA_RENDER_TEXTURE_HPP

#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! A Texture that can also serve as a target for 2D drawing.
class RenderTexture
    : public Texture
    , public Canvas {
public:
    virtual ~RenderTexture() = default;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_RENDER_TEXTURE_HPP

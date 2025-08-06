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

    //! Update the contents of the target texture.
    //!
    //! This function updates the target texture with what has been drawn so far. Like for windows,
    //! calling this function is mandatory at the end of rendering. Not calling it may leave the
    //! texture in an undefined state.
    virtual void display() = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_RENDER_TEXTURE_HPP

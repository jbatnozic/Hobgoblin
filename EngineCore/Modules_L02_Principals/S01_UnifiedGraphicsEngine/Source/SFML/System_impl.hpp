// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_SFML_SYSTEM_HPP
#define UHOBGOBLIN_UGE_SFML_SYSTEM_HPP

#include <Hobgoblin/UnifiedGraphicsEngine/System.hpp>

#include "Render_window_impl.hpp"
#include "Vertex_array_impl.hpp"

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class SFMLSystemImpl : public System {
public:
    ~SFMLSystemImpl() override = default;

    std::unique_ptr<RenderWindow> createRenderWindow() override {
        return std::make_unique<SFMLRenderWindowImpl>(SELF, win::VideoMode{640, 480});
    }

    std::unique_ptr<VertexArray> createVertexArray(PZInteger aSize) override {
        // return std::make_unique
    }
};

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UGE_SFML_SYSTEM_HPP


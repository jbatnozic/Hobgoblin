// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_SYSTEM_HPP
#define UHOBGOBLIN_UGE_SYSTEM_HPP

#include <Hobgoblin/Common.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class RenderWindow;
class CircleShape;
class VertexArray;

class System {
public:
    virtual ~System() = default;

    virtual std::unique_ptr<RenderWindow> createRenderWindow() = 0;

    virtual std::unique_ptr<VertexArray> createVertexArray(PZInteger aSize) = 0;
};

std::unique_ptr<System> CreateRenderSystem(const char* aSystemProviderName);

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_SYSTEM_HPP


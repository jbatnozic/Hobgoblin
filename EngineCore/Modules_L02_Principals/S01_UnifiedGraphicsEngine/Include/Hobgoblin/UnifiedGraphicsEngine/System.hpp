// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UGE_SYSTEM_HPP
#define UHOBGOBLIN_UGE_SYSTEM_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Unicode.hpp>
#include <Hobgoblin/UnifiedGraphicsEngine/Window_style.hpp>

#include <memory>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uge {

class RenderWindow;
class VertexArray;
class View;

class System {
public:
    virtual ~System() = default;

    virtual std::unique_ptr<RenderWindow> createRenderWindow(
        PZInteger            aWidth  = 640,
        PZInteger            aHeight = 480,
        WindowStyle          aStyle  = WindowStyle::DEFAULT,
        const UnicodeString& aTitle  = HG_UNILIT("Hobgoblin")) const = 0;

    // TODO: createRenderTexture

    virtual std::unique_ptr<View> createView() const = 0;

    virtual std::unique_ptr<View> createDefaultView(const RenderWindow& aRenderWindow) const = 0;

    virtual std::unique_ptr<VertexArray> createVertexArray(PZInteger aSize) const = 0;
};

//! Create a new rendering system.
//!
//! \param aSystemProviderName name of the rendering system provider. Must be one of the following:
//!                            - "SFML": Use SFML (OpenGL underneath).
//!
//! \throws TracedLogicError if an invalid provider name is passed.
std::unique_ptr<System> CreateRenderSystem(const char* aSystemProviderName);

} // namespace uge
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UGE_SYSTEM_HPP

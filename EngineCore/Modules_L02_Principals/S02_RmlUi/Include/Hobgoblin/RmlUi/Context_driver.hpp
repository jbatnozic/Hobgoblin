// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_CONTEXT_DRIVER_HPP
#define UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_CONTEXT_DRIVER_HPP

#include <RmlUi/Core.h>

#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Window.hpp>

#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>
HOBGOBLIN_NAMESPACE_BEGIN
namespace rml {

//! TODO(add description)
class ContextDriver {
public:
    ContextDriver(const std::string& aContextName, uwga::Canvas& aCanvas);
    ~ContextDriver();

    Rml::Context&       operator*();
    const Rml::Context& operator*() const;

    Rml::Context*       operator->();
    const Rml::Context* operator->() const;

    void render();

    //! Returns: Whether the event was consumed by an element (true) in the context or not (false).
    bool processEvent(const uwga::WindowEvent& aEvent);

    void update();

    void setCanvas(uwga::Canvas& aCanvas);

private:
    NeverNull<uwga::Canvas*> _canvas;
    Rml::Context*            _context = nullptr;
};

} // namespace rml
HOBGOBLIN_NAMESPACE_END
#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RMLUI_PRIVATE_RMLUI_CONTEXT_DRIVER_HPP

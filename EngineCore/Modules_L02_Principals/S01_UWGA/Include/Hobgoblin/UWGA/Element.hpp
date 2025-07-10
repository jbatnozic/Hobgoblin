// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_ELEMENT_HPP
#define UHOBGOBLIN_UWGA_ELEMENT_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class System;

//! Base class for all objects that are elements of the graphics system:
//! Windows, Canvases, Views, Drawables, etc.
class Element {
public:
    virtual ~Element() = default;

    virtual const System& getSystem() const = 0;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_ELEMENT_HPP

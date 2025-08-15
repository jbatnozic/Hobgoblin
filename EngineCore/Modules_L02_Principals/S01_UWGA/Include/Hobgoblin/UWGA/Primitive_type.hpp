// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_PRIMITIVE_TYPE_HPP
#define UHOBGOBLIN_UWGA_PRIMITIVE_TYPE_HPP

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! \brief Types of primitives that arrays of vertices (including `VertexArray`) can be interpreted as
//!        and rendered as.
//!
//! Points and lines have no area, therefore their thickness will always be 1 pixel,
//! regardless the current transform and view.
enum PrimitiveType {
    POINTS,         //!< List of individual points.
    LINES,          //!< List of individual lines.
    LINE_STRIP,     //!< List of connected lines, a point uses the previous point to form a line.
    TRIANGLES,      //!< List of individual triangles.
    TRIANGLE_STRIP, //!< List of connected triangles, a point uses the two previous points to form a
                    //!< triangle.
    TRIANGLE_FAN,   //!< List of connected triangles, a point uses the common center and the previous
                    //!< point to form a triangle.

    //! Number of different primitive types.
    //! \warning Always keep last in the enum!
    PRIMITIVE_TYPE_COUNT,
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_PRIMITIVE_TYPE_HPP

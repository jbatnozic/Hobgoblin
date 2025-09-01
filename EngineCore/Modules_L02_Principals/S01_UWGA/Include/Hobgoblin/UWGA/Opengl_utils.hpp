// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_OPENGL_UTILS_HPP
#define UHOBGOBLIN_UWGA_OPENGL_UTILS_HPP

#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Glsl_shader.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {
namespace opengl {

//! \brief Save the current OpenGL render states and matrices.
//! 
//! This function can be used when you mix UWGA drawing and direct OpenGL rendering.
//! Combined with `PopStates`, it ensures that:
//! - UWGA's internal states are not messed up by your OpenGL code
//! - your OpenGL states are not modified by a call to a UWGA function
//!
//! More specifically, it must be used around code that calls Draw functions. Example:
//!     // OpenGL code here...
//!     opengl::PushStates(canvas);
//!     canvas.draw(...);
//!     canvas.draw(...);
//!     opengl::PopStates(canvas);
//!     // OpenGL code here...
//!
//! \warning this function is quite expensive: it saves all the possible OpenGL states and
//!          matrices, even the ones you don't care about. Therefore it should be used wisely.
//!          It is provided for convenience, but the best results will be achieved if you handle
//!          OpenGL states yourself (because you know which states have really changed, and need
//!          to be saved and restored). Take a look at the resetGLStates function if you do so.
//!
//! \param aCanvas an instance of a Canvas coming from a System based on OpenGL.
//!
//! \throws TracedLogicError if `aCanvas.getSystem().getGraphicsAPI() != System::GraphicsAPI::OPENGL`.
void PushStates(Canvas& aCanvas);

//! \brief Restore the previously saved OpenGL render states and matrices.
//!
//! See the description of `PushStates` to get a detailed description of these functions.
//!
//! \param aCanvas an instance of a Canvas coming from a System based on OpenGL (doesn't
//!                have to be the same one used to call `PushStates`).
//!
//! \throws TracedLogicError if `aCanvas.getSystem().getGraphicsAPI() != System::GraphicsAPI::OPENGL`.
void PopStates(Canvas& aCanvas);

//! \brief Reset the internal OpenGL states so that the target is ready for drawing.
//!
//! This function can be used when you mix UWGA drawing and direct OpenGL rendering, if you 
//! choose not to use `PushStates`/`PopStates`. It makes sure that all OpenGL states needed by UWGA
//! are set, so that subsequent draw() calls will work as expected.
//!
//! Example:
//!     // OpenGL code here...
//!     glPushAttrib(...);
//!     opengl::ResetStates(canvas);
//!     canvas.draw(...);
//!     canvas.draw(...);
//!     glPopAttrib(...);
//!     // OpenGL code here...
//!
//! \param aCanvas an instance of a Canvas coming from a System based on OpenGL.
//!
//! \throws TracedLogicError if `aCanvas.getSystem().getGraphicsAPI() != System::GraphicsAPI::OPENGL`.
void ResetStates(Canvas& aCanvas);

//! \brief Activate or deactivate the canvas as the current target for OpenGL rendering.
//!
//! A canvas is active only on the current thread, if you want to make it active on another thread
//! you have to deactivate it on the previous thread first if it was active.
//! Only one canvas can be active on a thread at a time, thus the window previously active (if any)
//! automatically gets deactivated.
//! For `RenderWindow` canvases, This is not to be confused with `requestFocus()`!
//!
//! \param aCanvas an instance of a Canvas coming from a System based on OpenGL.
//! \param aActive whether to activate (true) or deactivate (false) the canvas.
//!
//! \return True if operation was successful, false otherwise.
//!
//! \throws TracedLogicError if `aCanvas.getSystem().getGraphicsAPI() != System::GraphicsAPI::OPENGL`.
bool SetActive(Canvas& aCanvas, bool aActive = true);

/// \brief Bind a shader for rendering
///
/// This function is not part of the graphics API, it mustn't be
/// used when drawing SFML entities. It must be used only if you
/// mix `sf::Shader` with OpenGL code.
///
/// \code
/// sf::Shader s1, s2;
/// ...
/// sf::Shader::bind(&s1);
/// // draw OpenGL stuff that use s1...
/// sf::Shader::bind(&s2);
/// // draw OpenGL stuff that use s2...
/// sf::Shader::bind(nullptr);
/// // draw OpenGL stuff that use no shader...
/// \endcode
///
/// \param shader Shader to bind, can be null to use no shader
void Bind(const GLSLShader* aShader);

} // namespace opengl
} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_OPENGL_UTILS_HPP

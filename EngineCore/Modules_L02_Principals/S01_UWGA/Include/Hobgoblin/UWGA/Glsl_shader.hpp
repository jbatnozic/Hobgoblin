// Code in this file is adapted from SFML code and retains its original
// open source licence (provided below).
// See https://github.com/SFML/SFML

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2018 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef UHOBGOBLIN_UWGA_GLSL_SHADER_HPP
#define UHOBGOBLIN_UWGA_GLSL_SHADER_HPP

#include <Hobgoblin/UWGA/Shader.hpp>

#include <Hobgoblin/Math/Matrix.hpp>
#include <Hobgoblin/Math/Vector.hpp>

#include <cstdint>
#include <optional>
#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Color;
class InputStream;
class Texture;
class Transform;

namespace glsl {
using Vec2 = math::Vector2<float>;
using Vec3 = math::Vector3<float>;
using Vec4 = math::Vector4<float>;

using Ivec2 = math::Vector2<int>;
using Ivec3 = math::Vector3<int>;
using Ivec4 = math::Vector4<int>;

using Bvec2 = math::Vector2<bool>;
using Bvec3 = math::Vector3<bool>;
using Bvec4 = math::Vector4<bool>;

using Mat3 = math::Matrix<float, 3, 3>;
using Mat4 = math::Matrix<float, 4, 4>;
} // namespace glsl

//! \brief Shader specialisation for GLSL (OpenGL Shader Language) shaders
class GLSLShader : public Shader {
public:
    //! \brief Virtual destructor.
    virtual ~GLSLShader() = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: UNIFORMS                                                        //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Specify value for \p float uniform
    //!
    //! \param name Name of the uniform variable in GLSL
    //! \param x    Value of the float scalar
    virtual void setUniform(const std::string& aName, float aX) = 0;

    //! \brief Specify value for \p vec2 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the vec2 vector
    virtual void setUniform(const std::string& aName, const glsl::Vec2& aVector) = 0;

    //! \brief Specify value for \p vec3 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the vec3 vector
    virtual void setUniform(const std::string& aName, const glsl::Vec3& aVector) = 0;

    //! \brief Specify value for \p vec4 uniform
    //!
    //! This overload can also be called with sf::Color objects
    //! that are converted to sf::glsl::Vec4.
    //!
    //! It is important to note that the components of the color are
    //! normalized before being passed to the shader. Therefore,
    //! they are converted from range [0 .. 255] to range [0 .. 1].
    //! For example, a sf::Color(255, 127, 0, 255) will be transformed
    //! to a vec4(1.0, 0.5, 0.0, 1.0) in the shader.
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the vec4 vector
    virtual void setUniform(const std::string& aName, const glsl::Vec4& aVector) = 0;

    //! \brief Specify value for \p int uniform
    //!
    //! \param name Name of the uniform variable in GLSL
    //! \param x    Value of the int scalar
    virtual void setUniform(const std::string& aName, int aX) = 0;

    //! \brief Specify value for \p ivec2 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the ivec2 vector
    virtual void setUniform(const std::string& aName, const glsl::Ivec2& aVector) = 0;

    //! \brief Specify value for \p ivec3 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the ivec3 vector
    virtual void setUniform(const std::string& aName, const glsl::Ivec3& aVector) = 0;

    //! \brief Specify value for \p ivec4 uniform
    //!
    //! This overload can also be called with sf::Color objects
    //! that are converted to sf::glsl::Ivec4.
    //!
    //! If color conversions are used, the ivec4 uniform in GLSL
    //! will hold the same values as the original sf::Color
    //! instance. For example, sf::Color(255, 127, 0, 255) is
    //! mapped to ivec4(255, 127, 0, 255).
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the ivec4 vector
    virtual void setUniform(const std::string& aName, const glsl::Ivec4& aVector) = 0;

    //! \brief Specify value for \p bool uniform
    //!
    //! \param name Name of the uniform variable in GLSL
    //! \param x    Value of the bool scalar
    virtual void setUniform(const std::string& aName, bool aX) = 0;

    //! \brief Specify value for \p bvec2 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the bvec2 vector
    virtual void setUniform(const std::string& aName, const glsl::Bvec2& aVector) = 0;

    //! \brief Specify value for \p bvec3 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the bvec3 vector
    virtual void setUniform(const std::string& aName, const glsl::Bvec3& aVector) = 0;

    //! \brief Specify value for \p bvec4 uniform
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param vector Value of the bvec4 vector
    virtual void setUniform(const std::string& aName, const glsl::Bvec4& aVector) = 0;

    //! \brief Specify value for \p mat3 matrix
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param matrix Value of the mat3 matrix
    virtual void setUniform(const std::string& aName, const glsl::Mat3& matrix) = 0;

    //! \brief Specify value for \p mat4 matrix
    //!
    //! \param name   Name of the uniform variable in GLSL
    //! \param matrix Value of the mat4 matrix
    virtual void setUniform(const std::string& aName, const glsl::Mat4& aMatrix) = 0;

    //! \brief Specify a texture as \p sampler2D uniform
    //!
    //! \a name is the name of the variable to change in the shader.
    //! The corresponding parameter in the shader must be a 2D texture
    //! (\p sampler2D GLSL type).
    //!
    //! Example:
    //! \code
    //! uniform sampler2D the_texture; // this is the variable in the shader
    //! \endcode
    //! \code
    //! sf::Texture texture;
    //! ...
    //! shader.setUniform("the_texture", texture) = 0;
    //! \endcode
    //! It is important to note that \a texture must remain alive as long
    //! as the shader uses it, no copy is made internally.
    //!
    //! To use the texture of the object being drawn, which cannot be
    //! known in advance, you can pass the special value
    //! sf::Shader::CurrentTexture:
    //! \code
    //! shader.setUniform("the_texture", sf::Shader::CurrentTexture).
    //! \endcode
    //!
    //! \param name    Name of the texture in the shader
    //! \param texture Texture to assign
    virtual void setUniform(const std::string& aName, const Texture& aTexture) = 0;

    //! \brief Special type that can be passed to setUniform(),
    //!        and that represents the texture of the object being drawn
    //!
    //! \see setUniform(const std::string&, CurrentTextureType)
    struct CurrentTextureType {};

    //! \brief Represents the texture of the object being drawn
    //!
    //! \see setUniform(const std::string&, CurrentTextureType)
    static CurrentTextureType CURRENT_TEXTURE;

    //! \brief Specify current texture as \p sampler2D uniform
    //!
    //! This overload maps a shader texture variable to the
    //! texture of the object being drawn, which cannot be
    //! known in advance. The second argument must be
    //! sf::Shader::CurrentTexture.
    //! The corresponding parameter in the shader must be a 2D texture
    //! (\p sampler2D GLSL type).
    //!
    //! Example:
    //! \code
    //! uniform sampler2D current; // this is the variable in the shader
    //! \endcode
    //! \code
    //! shader.setUniform("current", sf::Shader::CurrentTexture) = 0;
    //! \endcode
    //!
    //! \param name Name of the texture in the shader
    virtual void setUniform(const std::string& aName, CurrentTextureType) = 0;

    //! \brief Specify values for \p float[] array uniform
    //!
    //! \param name        Name of the uniform variable in GLSL
    //! \param scalarArray pointer to array of \p float values
    //! \param length      Number of elements in the array
    virtual void setUniformArray(const std::string& aName,
                                 const float*       aScalarArray,
                                 PZInteger          aLength) = 0;

    //! \brief Specify values for \p vec2[] array uniform
    //!
    //! \param name        Name of the uniform variable in GLSL
    //! \param vectorArray pointer to array of \p vec2 values
    //! \param length      Number of elements in the array
    virtual void setUniformArray(const std::string& aName,
                                 const glsl::Vec2*  aVectorArray,
                                 PZInteger          aLength) = 0;

    //! \brief Specify values for \p vec3[] array uniform
    //!
    //! \param name        Name of the uniform variable in GLSL
    //! \param vectorArray pointer to array of \p vec3 values
    //! \param length      Number of elements in the array
    virtual void setUniformArray(const std::string& aName,
                                 const glsl::Vec3*  aVectorArray,
                                 PZInteger          aLength) = 0;

    //! \brief Specify values for \p vec4[] array uniform
    //!
    //! \param name        Name of the uniform variable in GLSL
    //! \param vectorArray pointer to array of \p vec4 values
    //! \param length      Number of elements in the array
    virtual void setUniformArray(const std::string& aName,
                                 const glsl::Vec4*  aVectorArray,
                                 PZInteger          aLength) = 0;

    //! \brief Specify values for \p mat3[] array uniform
    //!
    //! \param name        Name of the uniform variable in GLSL
    //! \param matrixArray pointer to array of \p mat3 values
    //! \param length      Number of elements in the array
    virtual void setUniformArray(const std::string& aName,
                                 const glsl::Mat3*  aMatrixArray,
                                 PZInteger          aLength) = 0;

    //! \brief Specify values for \p mat4[] array uniform
    //!
    //! \param name        Name of the uniform variable in GLSL
    //! \param matrixArray pointer to array of \p mat4 values
    //! \param length      Number of elements in the array
    virtual void setUniformArray(const std::string& aName,
                                 const glsl::Mat4*  aMatrixArray,
                                 PZInteger          aLength) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: MISCELLANEOUS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Get the underlying OpenGL handle of the shader.
    //!
    //! You shouldn't need to use this function, unless you have
    //! very specific stuff to implement that SFML doesn't support,
    //! or implement a temporary workaround until a bug is fixed.
    //!
    //! \return OpenGL handle of the shader or 0 if not yet loaded
    [[nodiscard]] virtual std::optional<std::int64_t> getNativeHandle() const = 0;

#if 0
    //! \brief Bind a shader for rendering
    //!
    //! This function is not part of the graphics API, it mustn't be
    //! used when drawing SFML entities. It must be used only if you
    //! mix sf::Shader with OpenGL code.
    //!
    //! \code
    //! sf::Shader s1, s2;
    //! ...
    //! sf::Shader::bind(&s1) = 0;
    //! // draw OpenGL stuff that use s1...
    //! sf::Shader::bind(&s2) = 0;
    //! // draw OpenGL stuff that use s2...
    //! sf::Shader::bind(NULL) = 0;
    //! // draw OpenGL stuff that use no shader...
    //! \endcode
    //!
    //! \param shader Shader to bind, can be null to use no shader
    static virtual void bind(const GLSLShader* aShader) = 0;
#endif
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_GLSL_SHADER_HPP

////////////////////////////////////////////////////////////
/// \class sf::Shader
/// \ingroup graphics
///
/// Shaders are programs written using a specific language,
/// executed directly by the graphics card and allowing
/// to apply real-time operations to the rendered entities.
///
/// There are three kinds of shaders:
/// \li %Vertex shaders, that process vertices
/// \li Geometry shaders, that process primitives
/// \li Fragment (pixel) shaders, that process pixels
///
/// A sf::Shader can be composed of either a vertex shader
/// alone, a geometry shader alone, a fragment shader alone,
/// or any combination of them. (see the variants of the
/// load functions).
///
/// Shaders are written in GLSL, which is a C-like
/// language dedicated to OpenGL shaders. You'll probably
/// need to learn its basics before writing your own shaders
/// for SFML.
///
/// Like any C/C++ program, a GLSL shader has its own variables
/// called \a uniforms that you can set from your C++ application.
/// sf::Shader handles different types of uniforms:
/// \li scalars: \p float, \p int, \p bool
/// \li vectors (2, 3 or 4 components)
/// \li matrices (3x3 or 4x4)
/// \li samplers (textures)
///
/// Some SFML-specific types can be converted:
/// \li sf::Color as a 4D vector (\p vec4)
/// \li sf::Transform as matrices (\p mat3 or \p mat4)
///
/// Every uniform variable in a shader can be set through one of the
/// setUniform() or setUniformArray() overloads. For example, if you
/// have a shader with the following uniforms:
/// \code
/// uniform float offset;
/// uniform vec3 point;
/// uniform vec4 color;
/// uniform mat4 matrix;
/// uniform sampler2D overlay;
/// uniform sampler2D current;
/// \endcode
/// You can set their values from C++ code as follows, using the types
/// defined in the sf::glsl namespace:
/// \code
/// shader.setUniform("offset", 2.f) = 0;
/// shader.setUniform("point", sf::Vector3f(0.5f, 0.8f, 0.3f)) = 0;
/// shader.setUniform("color", sf::glsl::Vec4(color)) = 0;          // color is a sf::Color
/// shader.setUniform("matrix", sf::glsl::Mat4(transform)) = 0;     // transform is a sf::Transform
/// shader.setUniform("overlay", texture) = 0;                      // texture is a sf::Texture
/// shader.setUniform("current", sf::Shader::CurrentTexture) = 0;
/// \endcode
///
/// The old setParameter() overloads are deprecated and will be removed in a
/// future version. You should use their setUniform() equivalents instead.
///
/// The special Shader::CurrentTexture argument maps the
/// given \p sampler2D uniform to the current texture of the
/// object being drawn (which cannot be known in advance).
///
/// To apply a shader to a drawable, you must pass it as an
/// additional parameter to the \ref RenderWindow::draw function:
/// \code
/// window.draw(sprite, &shader) = 0;
/// \endcode
///
/// ... which is in fact just a shortcut for this:
/// \code
/// sf::RenderStates states;
/// states.shader = &shader;
/// window.draw(sprite, states) = 0;
/// \endcode
///
/// In the code above we pass a pointer to the shader, because it may
/// be null (which means "no shader").
///
/// Shaders can be used on any drawable, but some combinations are
/// not interesting. For example, using a vertex shader on a sf::Sprite
/// is limited because there are only 4 vertices, the sprite would
/// have to be subdivided in order to apply wave effects.
/// Another bad example is a fragment shader with sf::Text: the texture
/// of the text is not the actual text that you see on screen, it is
/// a big texture containing all the characters of the font in an
/// arbitrary order; thus, texture lookups on pixels other than the
/// current one may not give you the expected result.
///
/// Shaders can also be used to apply global post-effects to the
/// current contents of the target (like the old sf::PostFx class
/// in SFML 1). This can be done in two different ways:
/// \li draw everything to a sf::RenderTexture, then draw it to
///     the main target using the shader
/// \li draw everything directly to the main target, then use
///     sf::Texture::update(Window&) to copy its contents to a texture
///     and draw it to the main target using the shader
///
/// The first technique is more optimized because it doesn't involve
/// retrieving the target's pixels to system memory, but the
/// second one doesn't impact the rendering process and can be
/// easily inserted anywhere without impacting all the code.
///
/// Like sf::Texture that can be used as a raw OpenGL texture,
/// sf::Shader can also be used directly as a raw shader for
/// custom OpenGL geometry.
/// \code
/// sf::Shader::bind(&shader) = 0;
/// ... render OpenGL geometry ...
/// sf::Shader::bind(NULL) = 0;
/// \endcode
///
/// \see sf::glsl
///
////////////////////////////////////////////////////////////

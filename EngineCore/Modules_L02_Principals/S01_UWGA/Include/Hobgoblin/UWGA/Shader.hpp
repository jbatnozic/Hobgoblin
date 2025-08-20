// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SHADER_HPP
#define UHOBGOBLIN_UWGA_SHADER_HPP

#include <Hobgoblin/UWGA/Element.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class Shader : public Element {
public:
    //! Virtual destructor.
    virtual ~Shader() = default;

    enum class Language : std::uint32_t {
        NONE = 0,
        GLSL = 0x01 //!< OpenGL Shading Language
    };

    virtual Language getLanguage() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: RESETTING                                                       //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Kinds of shaders
    enum class Kind {
        VERTEX,   //!< Vertex shader
        FRAGMENT, //!< Fragment (pixel) shader
        GEOMETRY, //!< Geometry shader
    };

    //! \brief Reset the vertex, geometry or fragment shader by loading from a file
    //!
    //! This function resets only one of: vertex shader, fragment shader, or geometry shader
    //! (identified by the second argument). The new data will be loaded from a text file
    //! which must contain a valid shader in its language.
    //!
    //! \param aFile Path of the text file containing the shader.
    //! \param aKind Kind of shader to load (vertex, geometry or fragment)
    //!
    //! \throws on failure.
    virtual void reset(const std::filesystem::path& aFile, Kind aKind) = 0;

    //! \brief Reset the vertex and fragment shaders by loading from a file
    //!
    //! The new data will be loaded from text files which must contain valid
    //! shaders in the its language.
    //!
    //! \param aVertexShaderFile   Path of the text file containing the vertex shader.
    //! \param aFragmentShaderFile Path of the text file containing the fragment shader.
    //!
    //! \throws on failure (note: if any of the shader fails to load, the others will be
    //!         unloaded and the object will remain empty).
    virtual void reset(const std::filesystem::path& aVertexShaderFile,
                       const std::filesystem::path& aFragmentShaderFile) = 0;

    //! \brief Reset the vertex, geometry and fragment shaders by loading from a file
    //!
    //! This function resets the vertex shader, fragment shader, and geometry shader
    //! The new data will be loaded from text files which must contain valid shaders
    //! in its language.
    //!
    //! \param aVertexShaderFile   Path of the text file containing the vertex shader.
    //! \param aFragmentShaderFile Path of the text file containing the fragment shader.
    //! \param aGeometryShaderFile Path of the text file containing the geometry shader.
    //!
    //! \throws on failure (note: if any of the shader fails to load, the others will be
    //!         unloaded and the object will remain empty).
    virtual void reset(const std::filesystem::path& aVertexShaderFile,
                       const std::filesystem::path& aFragmentShaderFile,
                       const std::filesystem::path& aGeometryShaderFile) = 0;

    //! \brief Reset the vertex, geometry or fragment shader by loading from source code in memory
    //!
    //! This function resets only one of: vertex shader, fragment shader, or geometry shader
    //! (identified by the second argument). The new data will be loaded from a string in memory
    //! which must contain the valid source code of a shader in its language.
    //!
    //! \param aShaderSource String containing the source code of the shader to load.
    //! \param aKind Kind of shader to load (vertex, geometry or fragment).
    //!
    //! \throws on failure.
    virtual void reset(const std::string& aShaderSource, Kind aKind) = 0;

    //! \brief Reset the vertex and fragment shaders by loading from source codes in memory
    //!
    //! This function resets the vertex shader and the fragment shader. The new data will
    //! be loaded from strings in memory which must contain valid source codes of shaders
    //! in its language.
    //!
    //! \param aVertexShaderSource   String containing the source code of the vertex shader.
    //! \param aFragmentShaderSource String containing the source code of the fragment shader.
    //!
    //! \throws on failure (note: if any of the shader fails to load, the others will be
    //!         unloaded and the object will remain empty).
    virtual void reset(const std::string& aVertexShaderSource,
                       const std::string& aFragmentShaderSource) = 0;

    //! \brief Reset the vertex, geometry and fragment shaders by loading from source codes in memory
    //!
    //! This function resets the vertex shader, fragment shader, and the geometry shader. The new
    //! data will be loaded from strings in memory which must contain valid source codes of shaders
    //! in its language.
    //!
    //! \param aVertexShaderSource   String containing the source code of the vertex shader.
    //! \param aFragmentShaderSource String containing the source code of the fragment shader.
    //! \param aGeometryShaderSource String containing the source code of the geometry shader.
    //!
    //! \throws on failure (note: if any of the shader fails to load, the others will be
    //!         unloaded and the object will remain empty).
    virtual void reset(const std::string& aVertexShaderSource,
                       const std::string& aFragmentShaderSource,
                       const std::string& aGeometryShaderSource) = 0;
};

// clang-format off
#define HG_ENUM_DECLARE_ARITHMETIC_OP(_enum_, _op_) \
    _enum_ operator _op_(_enum_ aLhs, _enum_ aRhs);

#define HG_ENUM_DEFINE_ARITHMETIC_OP(_enum_, _op_)                                        \
    _enum_ operator _op_(_enum_ aLhs, _enum_ aRhs) {                                      \
        return static_cast<_enum_>(static_cast<std::underlying_type_t<_enum_>>(aLhs) _op_ \
                                   static_cast<std::underlying_type_t<_enum_>>(aRhs));    \
    }
// clang-format on

inline HG_ENUM_DEFINE_ARITHMETIC_OP(Shader::Language, &);
inline HG_ENUM_DEFINE_ARITHMETIC_OP(Shader::Language, |);

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_SHADER_HPP

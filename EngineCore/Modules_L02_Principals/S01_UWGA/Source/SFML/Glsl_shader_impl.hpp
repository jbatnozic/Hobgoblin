// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_GLSL_SHADER_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_GLSL_SHADER_IMPL_HPP

#include <Hobgoblin/UWGA/Glsl_shader.hpp>

#include <SFML/Graphics/Shader.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLGLSLShaderImpl : public GLSLShader {
public:
    SFMLGLSLShaderImpl(const System& aSystem);

    ~SFMLGLSLShaderImpl() override = default;

    const System& getSystem() const override;

    Language getLanguage() const override;

    const sf::Shader& getUnderlyingShader() const;

    // MARK: Shader/Resetting

    void reset(const std::filesystem::path& aFile, Kind aKind) override;

    void reset(const std::filesystem::path& aVertexShaderFile,
               const std::filesystem::path& aFragmentShaderFile) override;

    void reset(const std::filesystem::path& aVertexShaderFile,
               const std::filesystem::path& aFragmentShaderFile,
               const std::filesystem::path& aGeometryShaderFile) override;

    void reset(const std::string& aShaderSource, Kind aKind) override;

    void reset(const std::string& aVertexShaderSource,
               const std::string& aFragmentShaderSource) override;

    void reset(const std::string& aVertexShaderSource,
               const std::string& aFragmentShaderSource,
               const std::string& aGeometryShaderSource) override;

    // MARK: GLSLShader/Uniforms

    void setUniform(const std::string& aName, float aX) override;

    void setUniform(const std::string& aName, const glsl::Vec2& aVector) override;

    void setUniform(const std::string& aName, const glsl::Vec3& aVector) override;

    void setUniform(const std::string& aName, const glsl::Vec4& aVector) override;

    void setUniform(const std::string& aName, int aX) override;

    void setUniform(const std::string& aName, const glsl::Ivec2& aVector) override;

    void setUniform(const std::string& aName, const glsl::Ivec3& aVector) override;

    void setUniform(const std::string& aName, const glsl::Ivec4& aVector) override;

    void setUniform(const std::string& aName, bool aX) override;

    void setUniform(const std::string& aName, const glsl::Bvec2& aVector) override;

    void setUniform(const std::string& aName, const glsl::Bvec3& aVector) override;

    void setUniform(const std::string& aName, const glsl::Bvec4& aVector) override;

    void setUniform(const std::string& aName, const glsl::Mat3& matrix) override;

    void setUniform(const std::string& aName, const glsl::Mat4& aMatrix) override;

    void setUniform(const std::string& aName, const Texture& aTexture) override;

    void setUniform(const std::string& aName, CurrentTextureType) override;

    void setUniformArray(const std::string& aName,
                         const float*       aScalarArray,
                         PZInteger          aLength) override;

    void setUniformArray(const std::string& aName,
                         const glsl::Vec2*  aVectorArray,
                         PZInteger          aLength) override;

    void setUniformArray(const std::string& aName,
                         const glsl::Vec3*  aVectorArray,
                         PZInteger          aLength) override;

    void setUniformArray(const std::string& aName,
                         const glsl::Vec4*  aVectorArray,
                         PZInteger          aLength) override;

    void setUniformArray(const std::string& aName,
                         const glsl::Mat3*  aMatrixArray,
                         PZInteger          aLength) override;

    void setUniformArray(const std::string& aName,
                         const glsl::Mat4*  aMatrixArray,
                         PZInteger          aLength) override;

    // MARK: GLSLShader/Miscellaneous

    [[nodiscard]] std::optional<std::int64_t> getNativeHandle() const override;

private:
    const System& _system;
    sf::Shader    _shader;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_GLSL_SHADER_IMPL_HPP

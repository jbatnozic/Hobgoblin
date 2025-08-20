// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Glsl_shader_impl.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Glsl_shader.hpp>
#include <Hobgoblin/UWGA/Shader.hpp>

#include "SFML_conversions.hpp"
#include "SFML_err.hpp"
#include "Texture_provider.hpp"

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

#define VERIFY_VEC2_COMPATIBILITY(_hg_type_, _sf_type_)                                                 \
    static_assert(sizeof(_hg_type_) == sizeof(_sf_type_) && alignof(_hg_type_) == alignof(_sf_type_) && \
                      offsetof(_hg_type_, x) == offsetof(_sf_type_, x) &&                               \
                      offsetof(_hg_type_, y) == offsetof(_sf_type_, y),                                 \
                  #_hg_type_ " is not bitwise-compatible with " #_sf_type_)

#define VERIFY_VEC3_COMPATIBILITY(_hg_type_, _sf_type_)                                                 \
    static_assert(sizeof(_hg_type_) == sizeof(_sf_type_) && alignof(_hg_type_) == alignof(_sf_type_) && \
                      offsetof(_hg_type_, x) == offsetof(_sf_type_, x) &&                               \
                      offsetof(_hg_type_, y) == offsetof(_sf_type_, y) &&                               \
                      offsetof(_hg_type_, z) == offsetof(_sf_type_, z),                                 \
                  #_hg_type_ " is not bitwise-compatible with " #_sf_type_)

#define VERIFY_VEC4_COMPATIBILITY(_hg_type_, _sf_type_)                                                 \
    static_assert(sizeof(_hg_type_) == sizeof(_sf_type_) && alignof(_hg_type_) == alignof(_sf_type_) && \
                      offsetof(_hg_type_, x) == offsetof(_sf_type_, x) &&                               \
                      offsetof(_hg_type_, y) == offsetof(_sf_type_, y) &&                               \
                      offsetof(_hg_type_, z) == offsetof(_sf_type_, z) &&                               \
                      offsetof(_hg_type_, w) == offsetof(_sf_type_, w),                                 \
                  #_hg_type_ " is not bitwise-compatible with " #_sf_type_)

#define VERIFY_MATX_COMPATIBILITY(_hg_type_, _sf_type_)                                                 \
    static_assert(sizeof(_hg_type_) == sizeof(_sf_type_) && alignof(_hg_type_) == alignof(_sf_type_) && \
                      offsetof(_hg_type_, data) == offsetof(_sf_type_, array) &&                        \
                      sizeof(std::declval<_hg_type_>().data) ==                                         \
                          sizeof(std::declval<_sf_type_>().array),                                      \
                  #_hg_type_ " is not bitwise-compatible with " #_sf_type_)

// Checks to make sure we can reinterpret_cast between Hobgoblin's and SFML's GLSL types:

VERIFY_VEC2_COMPATIBILITY(glsl::Vec2, sf::Glsl::Vec2);
VERIFY_VEC3_COMPATIBILITY(glsl::Vec3, sf::Glsl::Vec3);
VERIFY_VEC4_COMPATIBILITY(glsl::Vec4, sf::Glsl::Vec4);

VERIFY_VEC2_COMPATIBILITY(glsl::Bvec2, sf::Glsl::Bvec2);
VERIFY_VEC3_COMPATIBILITY(glsl::Bvec3, sf::Glsl::Bvec3);
VERIFY_VEC4_COMPATIBILITY(glsl::Bvec4, sf::Glsl::Bvec4);

VERIFY_VEC2_COMPATIBILITY(glsl::Ivec2, sf::Glsl::Ivec2);
VERIFY_VEC3_COMPATIBILITY(glsl::Ivec3, sf::Glsl::Ivec3);
VERIFY_VEC4_COMPATIBILITY(glsl::Ivec4, sf::Glsl::Ivec4);

VERIFY_MATX_COMPATIBILITY(glsl::Mat3, sf::Glsl::Mat3);
VERIFY_MATX_COMPATIBILITY(glsl::Mat4, sf::Glsl::Mat4);

namespace {
sf::Shader::Type ToSf(Shader::Kind aKind) {
    switch (aKind) {
    case Shader::Kind::VERTEX:
        return sf::Shader::Vertex;

    case Shader::Kind::GEOMETRY:
        return sf::Shader::Geometry;

    case Shader::Kind::FRAGMENT:
        return sf::Shader::Fragment;

    default:
        HG_UNREACHABLE("Invalid value for hg::uwga::Shader::Kind ({}).", (int)aKind);
    }

    return {};
}
} // namespace

// MARK: SFMLGLSLShaderImpl

SFMLGLSLShaderImpl::SFMLGLSLShaderImpl(const System& aSystem)
    : _system{aSystem} {}

const System& SFMLGLSLShaderImpl::getSystem() const {
    return _system;
}

SFMLGLSLShaderImpl::Language SFMLGLSLShaderImpl::getLanguage() const {
    return Language::GLSL;
}

const sf::Shader& SFMLGLSLShaderImpl::getUnderlyingShader() const {
    return _shader;
}

// MARK: Shader/Resetting

void SFMLGLSLShaderImpl::reset(const std::filesystem::path& aFile, Kind aKind) {
    SFMLErrorCatcher sfErr;
    if (!_shader.loadFromFile(FilesystemPathToSfPath(aFile), ToSf(aKind))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

void SFMLGLSLShaderImpl::reset(const std::filesystem::path& aVertexShaderFile,
                               const std::filesystem::path& aFragmentShaderFile) {
    SFMLErrorCatcher sfErr;
    if (!_shader.loadFromFile(FilesystemPathToSfPath(aVertexShaderFile),
                              FilesystemPathToSfPath(aFragmentShaderFile))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

void SFMLGLSLShaderImpl::reset(const std::filesystem::path& aVertexShaderFile,
                               const std::filesystem::path& aFragmentShaderFile,
                               const std::filesystem::path& aGeometryShaderFile) {
    SFMLErrorCatcher sfErr;
    if (!_shader.loadFromFile(FilesystemPathToSfPath(aVertexShaderFile),
                              FilesystemPathToSfPath(aGeometryShaderFile),
                              FilesystemPathToSfPath(aFragmentShaderFile))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

void SFMLGLSLShaderImpl::reset(const std::string& aShaderSource, Kind aKind) {
    SFMLErrorCatcher sfErr;
    if (!_shader.loadFromMemory(aShaderSource, ToSf(aKind))) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

void SFMLGLSLShaderImpl::reset(const std::string& aVertexShaderSource,
                               const std::string& aFragmentShaderSource) {
    SFMLErrorCatcher sfErr;
    if (!_shader.loadFromMemory(aVertexShaderSource, aFragmentShaderSource)) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

void SFMLGLSLShaderImpl::reset(const std::string& aVertexShaderSource,
                               const std::string& aFragmentShaderSource,
                               const std::string& aGeometryShaderSource) {
    SFMLErrorCatcher sfErr;
    if (!_shader.loadFromMemory(aVertexShaderSource, aGeometryShaderSource, aFragmentShaderSource)) {
        HG_THROW_TRACED(TracedRuntimeError, 0, sfErr.getErrorMessage());
    }
}

// MARK: GLSLShader/Uniforms

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, float aX) {
    _shader.setUniform(aName, aX);
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Vec2& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Vec2&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Vec3& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Vec3&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Vec4& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Vec4&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, int aX) {
    _shader.setUniform(aName, aX);
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Ivec2& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Ivec2&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Ivec3& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Ivec3&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Ivec4& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Ivec4&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, bool aX) {
    _shader.setUniform(aName, aX);
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Bvec2& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Bvec2&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Bvec3& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Bvec3&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Bvec4& aVector) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Bvec4&>(aVector));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Mat3& aMatrix) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Mat3&>(aMatrix));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const glsl::Mat4& aMatrix) {
    _shader.setUniform(aName, reinterpret_cast<const sf::Glsl::Mat4&>(aMatrix));
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, const Texture& aTexture) {
    assert(&aTexture.getSystem() == &getSystem());

    const auto& sfTexture = static_cast<const SFMLTextureProvider&>(aTexture).getUnderlyingTexture();
    _shader.setUniform(aName, sfTexture);
}

void SFMLGLSLShaderImpl::setUniform(const std::string& aName, CurrentTextureType) {
    _shader.setUniform(aName, sf::Shader::CurrentTexture);
}

void SFMLGLSLShaderImpl::setUniformArray(const std::string& aName,
                                         const float*       aScalarArray,
                                         PZInteger          aLength) {
    _shader.setUniformArray(aName, aScalarArray, pztos(aLength));
}

void SFMLGLSLShaderImpl::setUniformArray(const std::string& aName,
                                         const glsl::Vec2*  aVectorArray,
                                         PZInteger          aLength) {
    _shader.setUniformArray(aName,
                            reinterpret_cast<const sf::Glsl::Vec2*>(aVectorArray),
                            pztos(aLength));
}

void SFMLGLSLShaderImpl::setUniformArray(const std::string& aName,
                                         const glsl::Vec3*  aVectorArray,
                                         PZInteger          aLength) {
    _shader.setUniformArray(aName,
                            reinterpret_cast<const sf::Glsl::Vec3*>(aVectorArray),
                            pztos(aLength));
}

void SFMLGLSLShaderImpl::setUniformArray(const std::string& aName,
                                         const glsl::Vec4*  aVectorArray,
                                         PZInteger          aLength) {
    _shader.setUniformArray(aName,
                            reinterpret_cast<const sf::Glsl::Vec4*>(aVectorArray),
                            pztos(aLength));
}

void SFMLGLSLShaderImpl::setUniformArray(const std::string& aName,
                                         const glsl::Mat3*  aMatrixArray,
                                         PZInteger          aLength) {
    _shader.setUniformArray(aName,
                            reinterpret_cast<const sf::Glsl::Mat3*>(aMatrixArray),
                            pztos(aLength));
}

void SFMLGLSLShaderImpl::setUniformArray(const std::string& aName,
                                         const glsl::Mat4*  aMatrixArray,
                                         PZInteger          aLength) {
    _shader.setUniformArray(aName,
                            reinterpret_cast<const sf::Glsl::Mat4*>(aMatrixArray),
                            pztos(aLength));
}

// MARK: GLSLShader/Miscellaneous

[[nodiscard]] std::optional<std::int64_t> SFMLGLSLShaderImpl::getNativeHandle() const {
    auto hnd = _shader.getNativeHandle();
    if (hnd == 0) {
        return std::nullopt;
    }
    return {static_cast<std::int64_t>(hnd)};
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

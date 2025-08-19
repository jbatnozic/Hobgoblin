// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef HOBGOBLIN_UGE_SFML_CONVERSIONS_HPP
#define HOBGOBLIN_UGE_SFML_CONVERSIONS_HPP

// Hobgoblin

#include <Hobgoblin/HGExcept.hpp>

#include <Hobgoblin/Math/Rectangle.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Blend_mode.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Glyph.hpp>
#include <Hobgoblin/UWGA/Primitive_type.hpp>
// #include <Hobgoblin/UWGA/Render_states.hpp>
// #include <Hobgoblin/UWGA/Shader.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>
// #include <Hobgoblin/UWGA/Texture_rect.hpp>
// #include <Hobgoblin/UWGA/Vertex_buffer.hpp>
// #include <Hobgoblin/UWGA/View.hpp>
#include <Hobgoblin/UWGA/Window_style.hpp>
#include <Hobgoblin/Window/Context_settings.hpp>

#include "Texture_provider.hpp"

// SFML

#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include <filesystem>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

///////////////////////////////////////////////////////////////////////////
// MARK: MATH                                                            //
///////////////////////////////////////////////////////////////////////////

template <class taArithmetic>
typename sf::Vector2<taArithmetic> ToSf(const typename math::Vector2<taArithmetic>& aVector) {
    return {aVector.x, aVector.y};
}

template <class taArithmetic>
typename math::Vector2<taArithmetic> ToHg(const typename sf::Vector2<taArithmetic>& aVector) {
    return {aVector.x, aVector.y};
}

template <class taArithmetic>
typename sf::Rect<taArithmetic> ToSf(const typename math::Rectangle<taArithmetic>& aRect) {
    return {aRect.getLeft(), aRect.getTop(), aRect.w, aRect.h};
}

template <class taArithmetic>
typename math::Rectangle<taArithmetic> ToHg(const typename sf::Rect<taArithmetic>& aRect) {
    return {aRect.left, aRect.top, aRect.width, aRect.height};
}

///////////////////////////////////////////////////////////////////////////
// MARK: FILSYSTEM                                                       //
///////////////////////////////////////////////////////////////////////////

inline std::string FilesystemPathToSfPath(const std::filesystem::path& aPath) {
    return aPath.string();
}

///////////////////////////////////////////////////////////////////////////
// MARK: DECLARATIONS                                                    //
///////////////////////////////////////////////////////////////////////////

// BlendMode

BlendMode     ToHg(sf::BlendMode aBlendMode);
sf::BlendMode ToSf(BlendMode aBlendMode);

// Color

Color     ToHg(sf::Color aColor);
sf::Color ToSf(Color aColor);

// ContextSettings

using win::ContextSettings;

ContextSettings::Attribute ToHg(sf::ContextSettings::Attribute aAttribute);
unsigned                   ToSf(ContextSettings::Attribute aAttribute);

ContextSettings     ToHg(const sf::ContextSettings& aSettings);
sf::ContextSettings ToSf(const ContextSettings& aSettings);

// Glyph

Glyph     ToHg(const sf::Glyph& aGlyph);
sf::Glyph ToSf(const Glyph& aGlyph);

// PrimitiveType

PrimitiveType     ToHg(sf::PrimitiveType aType);
sf::PrimitiveType ToSf(PrimitiveType aType);

// RenderStates

// sf::RenderStates ToSf(const RenderStates& aRenderStates);

// Texture

const sf::Texture* ToSf(const Texture* aTexture);

// sf::Texture::CoordinateType ToSf(Texture::CoordinateType aCoordType);

// TextureRect

sf::IntRect ConvertTextureRect(TextureRect aTextureRect);

// VertexBuffer

// VertexBuffer::Usage ToHg(sf::VertexBuffer::Usage aUsage);
// sf::VertexBuffer::Usage ToSf(VertexBuffer::Usage aUsage);

// View

// View ToHg(const sf::View& aView);
// const sf::View& ToSf(const View& aView);

// WindowStyle

sf::Uint32 ToSf(WindowStyle aStyle);

///////////////////////////////////////////////////////////////////////////
// MARK: INLINE DEFINITIONS                                              //
///////////////////////////////////////////////////////////////////////////

// BlendMode

// clang-format off
inline
BlendMode::Factor ToHg(sf::BlendMode::Factor aFactor) {
    switch (aFactor) {
    case sf::BlendMode::Zero:             return BlendMode::Factor::ZERO;
    case sf::BlendMode::One:              return BlendMode::Factor::ONE;
    case sf::BlendMode::SrcColor:         return BlendMode::Factor::SRC_COLOR;
    case sf::BlendMode::OneMinusSrcColor: return BlendMode::Factor::ONE_MINUS_DST_COLOR;
    case sf::BlendMode::DstColor:         return BlendMode::Factor::DST_COLOR;
    case sf::BlendMode::OneMinusDstColor: return BlendMode::Factor::ONE_MINUS_DST_COLOR;
    case sf::BlendMode::SrcAlpha:         return BlendMode::Factor::SRC_ALPHA;
    case sf::BlendMode::OneMinusSrcAlpha: return BlendMode::Factor::ONE_MINUS_SRC_ALPHA;
    case sf::BlendMode::DstAlpha:         return BlendMode::Factor::DST_ALPHA;
    case sf::BlendMode::OneMinusDstAlpha: return BlendMode::Factor::ONE_MINUS_DST_ALPHA;
    default:
        HG_UNREACHABLE("Invalid sf::BlendMode::Factor value ({}).", (int)aFactor);
    }
}

inline
BlendMode::Equation ToHg(sf::BlendMode::Equation aEquation) {
    switch (aEquation) {
    case sf::BlendMode::Add:             return BlendMode::Equation::ADD;
    case sf::BlendMode::Subtract:        return BlendMode::Equation::SUBTRACT;
    case sf::BlendMode::ReverseSubtract: return BlendMode::Equation::REVERSE_SUBTRACT;
    default:
        HG_UNREACHABLE("Invalid sf::BlendMode::Equation value ({}).", (int)aEquation);
    }
}

inline
BlendMode ToHg(sf::BlendMode aBlendMode) {
    return {
        ToHg(aBlendMode.colorSrcFactor),
        ToHg(aBlendMode.colorDstFactor),
        ToHg(aBlendMode.colorEquation),
        ToHg(aBlendMode.alphaSrcFactor),
        ToHg(aBlendMode.alphaDstFactor),
        ToHg(aBlendMode.alphaEquation)
    };
}

inline
sf::BlendMode::Factor ToSf(BlendMode::Factor aFactor) {
    switch (aFactor) {
    case BlendMode::Factor::ZERO:                return sf::BlendMode::Zero;
    case BlendMode::Factor::ONE:                 return sf::BlendMode::One;
    case BlendMode::Factor::SRC_COLOR:           return sf::BlendMode::SrcColor;
    case BlendMode::Factor::ONE_MINUS_SRC_COLOR: return sf::BlendMode::OneMinusSrcColor;
    case BlendMode::Factor::DST_COLOR:           return sf::BlendMode::DstColor;
    case BlendMode::Factor::ONE_MINUS_DST_COLOR: return sf::BlendMode::OneMinusDstColor;
    case BlendMode::Factor::SRC_ALPHA:           return sf::BlendMode::SrcAlpha;
    case BlendMode::Factor::ONE_MINUS_SRC_ALPHA: return sf::BlendMode::OneMinusSrcAlpha;
    case BlendMode::Factor::DST_ALPHA:           return sf::BlendMode::DstAlpha;
    case BlendMode::Factor::ONE_MINUS_DST_ALPHA: return sf::BlendMode::OneMinusDstAlpha;
    default:
        HG_UNREACHABLE("Invalid hg::gr::BlendMode::Factor value ({}).", (int)aFactor);
    }
}

inline
sf::BlendMode::Equation ToSf(BlendMode::Equation aEquation) {
    switch (aEquation) {
    case BlendMode::Equation::ADD:              return sf::BlendMode::Add;
    case BlendMode::Equation::SUBTRACT:         return sf::BlendMode::Subtract;
    case BlendMode::Equation::REVERSE_SUBTRACT: return sf::BlendMode::ReverseSubtract;
    default:
        HG_UNREACHABLE("Invalid hg::gr::BlendMode::Equation value ({}).", (int)aEquation);
    }
}

inline
sf::BlendMode ToSf(BlendMode aBlendMode) {
    return {
        ToSf(aBlendMode.colorSrcFactor),
        ToSf(aBlendMode.colorDstFactor),
        ToSf(aBlendMode.colorEquation),
        ToSf(aBlendMode.alphaSrcFactor),
        ToSf(aBlendMode.alphaDstFactor),
        ToSf(aBlendMode.alphaEquation)
    };
}
// clang-format on

// Color

inline Color ToHg(sf::Color aColor) {
    return Color{aColor.toInteger()};
}

inline sf::Color ToSf(Color aColor) {
    return sf::Color{aColor.toInt()};
}

// ContextSettings

// inline
// ContextSettings::Attribute ToHg(sf::ContextSettings::Attribute aAttribute) {
//     if (aAttribute == sf::ContextSettings::Default) {
//         return ContextSettings::Attribute::Default;
//     }

//     auto result = static_cast<ContextSettings::Attribute>(0);

//     if ((aAttribute & sf::ContextSettings::Core)  != 0) result = result |
//     ContextSettings::Attribute::Core; if ((aAttribute & sf::ContextSettings::Debug) != 0) result =
//     result | ContextSettings::Attribute::Debug;

//     return result;
// }

// inline
// unsigned ToSf(ContextSettings::Attribute aAttribute) {
//     if (aAttribute == ContextSettings::Attribute::Default) {
//         return sf::ContextSettings::Default;
//     }

//     unsigned result = 0;

//     if ((aAttribute & ContextSettings::Attribute::Core)  !=
//     static_cast<ContextSettings::Attribute>(0)) result |= sf::ContextSettings::Core; if ((aAttribute &
//     ContextSettings::Attribute::Debug) != static_cast<ContextSettings::Attribute>(0)) result |=
//     sf::ContextSettings::Debug;

//     return result;
// }

// inline
// ContextSettings ToHg(const sf::ContextSettings& aSettings) {
//     return ContextSettings{
//         static_cast<PZInteger>(aSettings.depthBits),
//         static_cast<PZInteger>(aSettings.stencilBits),
//         static_cast<PZInteger>(aSettings.antialiasingLevel),
//         static_cast<PZInteger>(aSettings.majorVersion),
//         static_cast<PZInteger>(aSettings.minorVersion),
//         ToHg(static_cast<sf::ContextSettings::Attribute>(aSettings.attributeFlags)),
//         aSettings.sRgbCapable
//     };
// }

// inline
// sf::ContextSettings ToSf(const ContextSettings& aSettings) {
//     return sf::ContextSettings{
//         static_cast<unsigned>(aSettings.depthBits),
//         static_cast<unsigned>(aSettings.stencilBits),
//         static_cast<unsigned>(aSettings.antialiasingLevel),
//         static_cast<unsigned>(aSettings.majorVersion),
//         static_cast<unsigned>(aSettings.minorVersion),
//         ToSf(aSettings.attributeFlags),
//         aSettings.sRgbCapable
//     };
// }

// Glyph

inline Glyph ToHg(const sf::Glyph& aGlyph) {
    return {
        aGlyph.advance,
        aGlyph.lsbDelta,
        aGlyph.rsbDelta,
        ToHg(aGlyph.bounds),
        TextureRect{static_cast<std::uint16_t>(aGlyph.textureRect.left),
                    static_cast<std::uint16_t>(aGlyph.textureRect.top),
                    static_cast<std::uint16_t>(aGlyph.textureRect.width),
                    static_cast<std::uint16_t>(aGlyph.textureRect.height)}
    };
}

inline sf::Glyph ToSf(const Glyph& aGlyph) {
    sf::Glyph result;
    result.advance     = aGlyph.advance;
    result.lsbDelta    = aGlyph.lsbDelta;
    result.rsbDelta    = aGlyph.rsbDelta;
    result.bounds      = ToSf(aGlyph.bounds);
    result.textureRect = ConvertTextureRect(aGlyph.textureRect);
    return result;
}

// PrimitiveType

// NOTE: MSVC is too stupid to optimize these switch/case convertors (while clang and gcc can do it).

// clang-format off
inline
PrimitiveType ToHg(sf::PrimitiveType aType) {
    switch (aType) {
    case sf::Points:        return PrimitiveType::POINTS;
    case sf::Lines:         return PrimitiveType::LINES;
    case sf::LineStrip:     return PrimitiveType::LINE_STRIP;
    case sf::Triangles:     return PrimitiveType::TRIANGLES;
    case sf::TriangleStrip: return PrimitiveType::TRIANGLE_STRIP;
    case sf::TriangleFan:   return PrimitiveType::TRIANGLE_FAN;
    default:
        HG_UNREACHABLE("Invalid sf::PrimitiveType value ({}).", (int)aType);
    }
}

inline
sf::PrimitiveType ToSf(PrimitiveType aType) {
    switch (aType) {
    case PrimitiveType::POINTS:         return sf::Points;
    case PrimitiveType::LINES:          return sf::Lines;
    case PrimitiveType::LINE_STRIP:     return sf::LineStrip;
    case PrimitiveType::TRIANGLES:      return sf::Triangles;
    case PrimitiveType::TRIANGLE_STRIP: return sf::TriangleStrip;
    case PrimitiveType::TRIANGLE_FAN:   return sf::TriangleFan;
    default:
        HG_UNREACHABLE("Invalid hg::gr::PrimitiveType value ({}).", (int)aType);
    }
}
// clang-format on

// RenderStates

// inline
// sf::RenderStates ToSf(const RenderStates& aRenderStates) {
//     sf::RenderStates rs;

//     rs.blendMode = ToSf(aRenderStates.blendMode);
//     rs.transform = ToSf(aRenderStates.transform);
//     rs.texture   = (aRenderStates.texture != nullptr) ?
//     &detail::GraphicsImplAccessor::getImplOf<sf::Texture>(*aRenderStates.texture)
//                                                       : nullptr;
//     rs.shader    = (aRenderStates.shader != nullptr) ?
//     &detail::GraphicsImplAccessor::getImplOf<sf::Shader>(*aRenderStates.shader)
//                                                      : nullptr;

//     return rs;
// }

// Texture

inline const sf::Texture* ToSf(const Texture* aTexture) {
    if (!aTexture) {
        return nullptr;
    }
    return &static_cast<const SFMLTextureProvider*>(aTexture)->getUnderlyingTexture();
}

// inline
// sf::Texture::CoordinateType ToSf(Texture::CoordinateType aCoordType) {
//     switch (aCoordType) {
//     case Texture::CoordinateType::Normalized: return sf::Texture::Normalized;
//     case Texture::CoordinateType::Pixels: return sf::Texture::Pixels;
//     default:
//         HG_UNREACHABLE("Invalid hg::gr::Texture::CoordinateType value ({}).", (int)aCoordType);
//     };
// }

// TextureRect

inline sf::IntRect ConvertTextureRect(TextureRect aTextureRect) {
    return {static_cast<int>(aTextureRect.getLeft()),
            static_cast<int>(aTextureRect.getTop()),
            static_cast<int>(aTextureRect.w),
            static_cast<int>(aTextureRect.h)};
}

// VertexBuffer

// inline
// VertexBuffer::Usage ToHg(sf::VertexBuffer::Usage aUsage) {
//     switch (aUsage) {
//     case sf::VertexBuffer::Stream:  return VertexBuffer::Usage::Stream;
//     case sf::VertexBuffer::Dynamic: return VertexBuffer::Usage::Dynamic;
//     case sf::VertexBuffer::Static:  return VertexBuffer::Usage::Static;
//     default:
//         HG_UNREACHABLE("Invalid sf::VertexBuffer::Usage value ({}).", (int)aUsage);
//     }
// }

// inline
// sf::VertexBuffer::Usage ToSf(VertexBuffer::Usage aUsage) {
//     switch (aUsage) {
//     case VertexBuffer::Usage::Stream:  return sf::VertexBuffer::Stream;
//     case VertexBuffer::Usage::Dynamic: return sf::VertexBuffer::Dynamic;
//     case VertexBuffer::Usage::Static:  return sf::VertexBuffer::Static;
//     default:
//         HG_UNREACHABLE("Invalid hg::gr::VertexBuffer::Usage value ({}).", (int)aUsage);
//     }
// }

// View

// inline
// View ToHg(const sf::View& aView) {
//     View result;
//     auto& sfView = detail::GraphicsImplAccessor::getImplOf<sf::View>(result);
//     sfView = aView; // TODO: unnecessary copy when objects are bitwise compatible
//     return result;
// }

// inline
// const sf::View& ToSf(const View& aView) {
//     const auto& sfView = detail::GraphicsImplAccessor::getImplOf<sf::View>(aView);
//     return sfView;
// }

// WindowStyle

// clang-format off
inline
sf::Uint32 ToSf(WindowStyle aStyle) {
    if (aStyle == WindowStyle::NONE) {
        return sf::Style::None;
    }

    int result = 0;

    if ((aStyle & WindowStyle::TITLEBAR)   != WindowStyle::NONE) result |= sf::Style::Titlebar;
    if ((aStyle & WindowStyle::RESIZE)     != WindowStyle::NONE) result |= sf::Style::Resize;
    if ((aStyle & WindowStyle::CLOSE)      != WindowStyle::NONE) result |= sf::Style::Close;
    if ((aStyle & WindowStyle::FULLSCREEN) != WindowStyle::NONE) result |= sf::Style::Fullscreen;
    if ((aStyle & WindowStyle::DEFAULT)    != WindowStyle::NONE) result |= sf::Style::Default;

    return result;
}
// clang-format on

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !HOBGOBLIN_UGE_SFML_CONVERSIONS_HPP

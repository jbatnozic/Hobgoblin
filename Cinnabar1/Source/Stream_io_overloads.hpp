// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Math/Angle.hpp>
#include <Hobgoblin/Math/Vector2.hpp>
#include <Hobgoblin/Utility/Stream_input.hpp>
#include <Hobgoblin/Utility/Stream_output.hpp>

#include <cstdint>

// Note: These overloads are defined in the same namespace as the operands
//       (`jbatnozic::hobgoblin::math`) so that they can be found via ADL when
//       used with Hobgoblin's `OutputStream`/`InputStream`.
namespace jbatnozic {
namespace hobgoblin {
namespace math {

///////////////////////////////////////////////////////////////////////////
// MARK: Vector2d                                                         //
///////////////////////////////////////////////////////////////////////////

//! Packing operator for `hg::math::Vector2d`.
inline util::OutputStream& operator<<(util::OutputStreamExtender& aOStream, const Vector2d& aVec) {
    aOStream->noThrow() << aVec.x << aVec.y;
    return *aOStream;
}

//! Unpacking operator for `hg::math::Vector2d`.
inline util::InputStream& operator>>(util::InputStreamExtender& aIStream, Vector2d& aVec) {
    aIStream->noThrow() >> aVec.x >> aVec.y;
    return *aIStream;
}

///////////////////////////////////////////////////////////////////////////
// MARK: Vector2i                                                         //
///////////////////////////////////////////////////////////////////////////

//! Packing operator for `hg::math::Vector2i`.
inline util::OutputStream& operator<<(util::OutputStreamExtender& aOStream, const Vector2i& aVec) {
    aOStream->noThrow() << static_cast<std::int32_t>(aVec.x) << static_cast<std::int32_t>(aVec.y);
    return *aOStream;
}

//! Unpacking operator for `hg::math::Vector2i`.
inline util::InputStream& operator>>(util::InputStreamExtender& aIStream, Vector2i& aVec) {
    auto& istream = *aIStream;
    aVec.x        = static_cast<int>(istream.extractNoThrow<std::int32_t>());
    aVec.y        = static_cast<int>(istream.extractNoThrow<std::int32_t>());
    return istream;
}

///////////////////////////////////////////////////////////////////////////
// MARK: AngleF                                                           //
///////////////////////////////////////////////////////////////////////////

//! Packing operator for `hg::math::AngleF`.
inline util::OutputStream& operator<<(util::OutputStreamExtender& aOStream, AngleF aAngle) {
    aOStream->noThrow() << aAngle.asRadians();
    return *aOStream;
}

//! Unpacking operator for `hg::math::AngleF`.
inline util::InputStream& operator>>(util::InputStreamExtender& aIStream, AngleF& aAngle) {
    auto& istream = *aIStream;
    aAngle        = AngleF::fromRadians(istream.extractNoThrow<float>());
    return istream;
}

} // namespace math
} // namespace hobgoblin
} // namespace jbatnozic

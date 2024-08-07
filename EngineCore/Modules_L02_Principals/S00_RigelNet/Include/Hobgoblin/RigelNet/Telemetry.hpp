// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

// clang-format off

#ifndef UHOBGOBLIN_RN_TELEMETRY_HPP
#define UHOBGOBLIN_RN_TELEMETRY_HPP

#include <Hobgoblin/Common.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace rn {

struct RN_Telemetry {
    //! Estimated number of bytes uploaded to the network.
    //! Note that this does NOT count bytes exchanged between
    //! locally connected nodes.
    hobgoblin::PZInteger uploadByteCount = 0;
    //! Estimated number of bytes downloaded from the network.
    //! Note that this does NOT count bytes exchanged between
    //! locally connected nodes.
    hobgoblin::PZInteger downloadByteCount = 0;
};

inline
RN_Telemetry operator+(const RN_Telemetry& aLhs, const RN_Telemetry& aRhs) {
    return RN_Telemetry{
        aLhs.uploadByteCount   + aRhs.uploadByteCount,
        aLhs.downloadByteCount + aRhs.downloadByteCount
    };
}

inline
RN_Telemetry& operator+=(RN_Telemetry& aLhs, const RN_Telemetry& aRhs) {
    return (aLhs = aLhs + aRhs);
}

} // namespace rn
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_RN_TELEMETRY_HPP

// clang-format on

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Parcel_pool.hpp>

#include <algorithm>
#include <cstdlib>
#include <utility>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

// Because malloc/free actually outperforms parcelization by miles
// NOTE: there is also a macro like this in Parcel.cpp
#define UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION

std::shared_ptr<ParcelPool> ParcelPool::create() {
    class ParcelPoolMakeSharedEnabler : public ParcelPool {};
    return std::make_shared<ParcelPoolMakeSharedEnabler>();
}

Parcel ParcelPool::obtainParcel(PZInt64 aCapacity) {
#ifndef UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION
    Parcel::UHOBGOBLIN_UnderlyingContainer buf;

    std::unique_lock<decltype(_mutex)> lock{_mutex};
    if (_buffers.empty()) {
        buf.reserve(pz64tos(aCapacity));
    } else {
        auto iter = _buffers.lower_bound(aCapacity);
        if (iter != _buffers.end()) {
            buf = std::move(_buffers.extract(iter).value());
        } else {
            buf = std::move(_buffers.extract(_buffers.begin()).value());
            buf.reserve(pz64tos(aCapacity));
        }
    }
    lock.unlock();

    buf.resize(0);
    return Parcel(std::move(buf), weak_from_this());
#else
    Parcel::UHOBGOBLIN_UnderlyingContainer buf;
    buf.reserve(pz64tos(aCapacity));
    return Parcel(std::move(buf), weak_from_this());
#endif
}

Parcel ParcelPool::obtainParcel(PZInt64 aCapacity, PZInt64 aSize) {
#ifndef UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION
#error FUNCTION_NOT_IMPLEMENTED
#else
    Parcel::UHOBGOBLIN_UnderlyingContainer buf;
    buf.reserve(pz64tos(std::max(aCapacity, aSize)));
    if (aSize > 0) {
        buf.resize(pz64tos(aSize));
    }
    return Parcel(std::move(buf), weak_from_this());
#endif
}

Parcel ParcelPool::obtainParcel(PZInt64 aCapacity, PZInt64 aSize, char aFill) {
#ifndef UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION
#error FUNCTION_NOT_IMPLEMENTED
#else
    Parcel::UHOBGOBLIN_UnderlyingContainer buf;
    buf.reserve(pz64tos(std::max(aCapacity, aSize)));
    if (aSize > 0) {
        buf.resize(pz64tos(aSize));
        std::memset(buf.data(), static_cast<int>(aFill), pz64tos(aSize));
    }
    return Parcel(std::move(buf), weak_from_this());
#endif
}

void ParcelPool::_returnBuffer(Parcel::UHOBGOBLIN_UnderlyingContainer aBuffer) {
#ifndef UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION
    if (aBuffer.capacity() == 0) {
        return;
    }

    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _buffers.insert(std::move(aBuffer));
#else
    // Do nothing
#endif
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

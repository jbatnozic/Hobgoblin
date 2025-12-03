// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Private/Pmacro_define.hpp>
#include <Hobgoblin/Utility/Parcel_pool.hpp>

#include <utility>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

std::shared_ptr<ParcelPool> ParcelPool::create() {
    class ParcelPoolMakeSharedEnabler : public ParcelPool {};
    return std::make_shared<ParcelPoolMakeSharedEnabler>();
}

Parcel ParcelPool::obtainParcel(PZInt64 aCapacity) {
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
}

void ParcelPool::_returnBuffer(Parcel::UHOBGOBLIN_UnderlyingContainer aBuffer) {
    if (aBuffer.capacity() == 0) {
        return;
    }

    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _buffers.insert(std::move(aBuffer));
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

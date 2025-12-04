// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Parcel.hpp>
#include <Hobgoblin/Utility/Parcel_pool.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

// Because malloc/free actually outperforms parcelization by miles
// NOTE: there is also a macro like this in Parcel_pool.cpp
#define UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION

Parcel::Parcel(Parcel&& aOther)
    : _buffer{std::move(aOther._buffer)}
    , _pool{std::move(aOther._pool)} //
{
    aOther._pool.reset();
}

Parcel& Parcel::operator=(Parcel&& aOther) {
    if (this != &aOther) {
        SELF._buffer = std::move(aOther._buffer);
        SELF._pool   = std::move(aOther._pool);
        aOther._pool.reset();
    }
    return SELF;
}

Parcel::~Parcel() {
#ifndef UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION
    relinquish();
#endif
}

void Parcel::relinquish() {
#ifndef UHOBGOBLIN_PARCEL_TEMPORARY_IMPLEMENTATION
    if (auto pool = _pool.lock()) {
        pool->_returnBuffer(std::move(_buffer));
        _pool.reset();
    }
#else
    _pool.reset();
#endif
}

std::shared_ptr<ParcelPool> Parcel::getPool() const {
    return _pool.lock();
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UTIL_PARCEL_HPP
#define UHOBGOBLIN_UTIL_PARCEL_HPP

#include <Hobgoblin/Common/Positive_or_zero_integer.hpp>
#include <Hobgoblin/Utility/Stream_out_adapter.hpp>
#include <Hobgoblin/Utility/Stream_view.hpp>

#include <cassert>
#include <memory>
#include <span>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

class ParcelPool;

//! A Parcel represents an expandable region of raw memory which can be used to store binary data or
//! objects, similar to the region returned by malloc().
//!
//! The main difference is that a Parcel is reusable: a `ParcelPool` from which you obtain a Parcel will
//! re-acquire the Parcel's internal buffer when the Parcel is destroyed. Then, when you request another
//! Parcel from the Pool, it will try to reuse a pre-existing buffer of sufficient size.
class Parcel {
public:
    //! *******************************
    //! \warning it is necessary that these types remain public, but they should NOT be considered
    //!          a part of the user-facing API! DO NOT use them unless you're a library maintainer!
    struct UHOBGOBLIN_RawByte {
        char c;
        UHOBGOBLIN_RawByte() { /* IMPORTANT: no initialisation at all */ }
    };
    using UHOBGOBLIN_UnderlyingContainer = std::vector<UHOBGOBLIN_RawByte>;
    using UHOBGOBLIN_InStream            = ViewStream;
    using UHOBGOBLIN_OutStream           = OutputStreamContainerAdapter<UHOBGOBLIN_UnderlyingContainer>;
    //! *******************************

public:
    //! Initializes a non-good parcel.
    //! Use `ParcelPool::obtainParcel` to obtain a good parcel.
    Parcel() = default;

    //! No copying allowed.
    Parcel(const Parcel& aOther)            = delete;
    Parcel& operator=(const Parcel& aOther) = delete;

    //! Moving is OK (moved-from parcels assume a non-good but valid state)
    Parcel(Parcel&& aOther);
    Parcel& operator=(Parcel&& aOther);

    //! Destructor. Will relinquish the parcel automatically.
    ~Parcel();

    //! Check if a parcel is in a usable state.
    //! Parcels are good unless they have been default-constructed or relinquished.
    //! \note the only valid operations on a non-good parcel are to reassign it or destroy it.
    bool isGood() const;

    //! Return the parcel's resources to its pool of origin.
    //!
    //! Call this when you know you won't be using the parcel anymore.
    //! `relinquish()` is idempotent.
    //!
    //! \warning once this call returns, the parcel won't be good anymore, and the only valid operations
    //!          after that will be to reassign it or destroy it.
    void relinquish();

    //! Get a pointer to the parcel's pool of origin.
    //! Returns `nullptr` if the pool has been destroyed or the parcel isn't good anymore.
    std::shared_ptr<ParcelPool> getPool() const;

    //! Return the number of bytes in the parcel.
    //! \note same as with standard containers, this is not the same as the actual allocated capacity
    //!       of the parcel.
    PZInt64 getSize() const;

    //! Change the size of the parcel, reserving additional capacity automatically if needed.
    //!
    //! \note same as with standard containers, this does not affect the actual allocated capacity of
    //!       the parcel, just the number of bytes that are considered 'used'.
    //! \note in case of upsizing, the newly-added region of memory will NOT be zero'd out and will
    //!       contain random values.
    //! \warning in case of upsizing, all pointers, spans, and similar pointing into the parcel are
    //!          invalidated.
    void resize(PZInt64 aSize);

    //! Reduce the logical size of the parcel back to 0 without affecting the actual allocated capacity.
    //! \note equivalent to `resize(0)`.
    void clear();

    //! Increase the total capacity of the parcel without changing the contents or the size.
    //! \warning in case of upsizing, all pointers, spans, and similar pointing into the parcel are
    //!          invalidated.
    void reserve(PZInt64 aSize);

    //! Get the actual allocated memory capacity of the parcel.
    PZInt64 getCapacity() const;

    //! Get a const pointer to the first allocated byte of the parcel.
    //! \note the pointer is guaranteed to be aligned to at least an 8-byte boundary.
    //! \warning when `getSize()` returns 0, calling this function produces Undefined Behaviour!
    const char* getData() const;

    //! Get a non-const pointer to the first allocated byte of the parcel.
    //! \note the pointer is guaranteed to be aligned to at least an 8-byte boundary.
    //! \warning when `getSize()` returns 0, calling this function produces Undefined Behaviour!
    char* getMutableData();

    //! Create and return a read-only span which views the data in the parcel as such:
    //! `[ getData(0), getData(getSize()) )`
    std::span<const char> getSpan() const;

    //! Create and return a read-write span which views the data in the parcel as such:
    //! `[ getMutableData(0), getData(getMutableData()) )`
    std::span<char> getMutableSpan();

    //! Create and return an object that fulfils the `InputStream` interface.
    //! This object can be used to read all the data in the parcel, starting from the byte at
    //! position `aPosition` (use 0 to read all the data).
    //! \warning if `aPosition` is greater than `getSize()`, the function produces Undefined Behaviour!
    //! \warning modifying the parcel in any way will invalidate the returned object.
    UHOBGOBLIN_InStream createInputStream(PZInt64 aPosition) const;

    //! Create and return an object that fulfils the `InputStream` interface.
    //! This object can be used to read all the data in the parcel, starting from the first byte.
    //! \note equivalent to `createInputStream(0)`.
    //! \warning modifying the parcel in any way will invalidate the returned object.
    UHOBGOBLIN_InStream createInputStream() const;

    //! Create and return an object that fulfils the `OutputStream` interface.
    //! This object can be used to write data to the parcel. The parcel will be resized automatically
    //! if needed. All the data written using this object will end up before the byte that was at
    //! position `aPosition` at the time of the object's creation.
    //!
    //! \warning if `aPosition` is greater than `getSize()`, the function produces Undefined Behaviour!
    //! \warning modifying the parcel in any way other than through the returned object's stream API
    //!          will invalidate the returned object. Also, all other pointers, spans, and similar
    //!          pointing into the parcel will be invalidated should you write enough data to exceed
    //!          the parcel's capacity and trigger a reallocation.
    UHOBGOBLIN_OutStream createOutputStream(PZInt64 aPosition);

    //! Create and return an object that fulfils the `OutputStream` interface.
    //! This object can be used to write data to the end of the parcel (in other words, to append data).
    //! The parcel will be resized automatically if needed.
    //!
    //! \note equivalent to `createOutputStream(getSize())`.
    //! \warning modifying the parcel in any way other than through the returned object's stream API
    //!          will invalidate the returned object. Also, all other pointers, spans, and similar
    //!          pointing into the parcel will be invalidated should you write enough data to exceed
    //!          the parcel's capacity and trigger a reallocation.
    UHOBGOBLIN_OutStream createOutputStream();

private:
    friend class ParcelPool;

    UHOBGOBLIN_UnderlyingContainer _buffer;
    std::weak_ptr<ParcelPool>      _pool;

    Parcel(UHOBGOBLIN_UnderlyingContainer aBuffer, std::weak_ptr<ParcelPool> aPool);
};

#ifndef NDEBUG
// We just kind of assume that the default allocator will return sufficiently aligned
// addresses, but we also check in a few places just in case
#define IS_ALIGNED(_pointer_, _byte_count_) \
    ((reinterpret_cast<uintptr_t>((const void*)(_pointer_)) % (_byte_count_)) == 0)
#endif

inline bool Parcel::isGood() const {
    const auto empty = std::weak_ptr<ParcelPool>{};
    return (_pool.owner_before(empty) || empty.owner_before(_pool));
}

inline PZInt64 Parcel::getSize() const {
    return stopz64(_buffer.size());
}

inline void Parcel::resize(PZInt64 aSize) {
    _buffer.resize(pztos(aSize));
    assert(_buffer.empty() || IS_ALIGNED(_buffer.data(), sizeof(void*)));
}

inline void Parcel::clear() {
    _buffer.resize(0);
}

inline void Parcel::reserve(PZInt64 aSize) {
    const auto s = pz64tos(aSize);
    if (s > _buffer.capacity()) {
        _buffer.reserve(s);
    }
}

PZInt64 inline Parcel::getCapacity() const {
    return stopz64(_buffer.capacity());
}

inline const char* Parcel::getData() const {
    assert(isGood());
    const auto p = std::launder(reinterpret_cast<const char*>(_buffer.data()));
    assert(IS_ALIGNED(p, sizeof(void*)));
    return p;
}

inline char* Parcel::getMutableData() {
    assert(isGood());
    const auto p = std::launder(reinterpret_cast<char*>(_buffer.data()));
    assert(IS_ALIGNED(p, sizeof(void*)));
    return p;
}

inline std::span<const char> Parcel::getSpan() const {
    assert(isGood());
    return {getData(), _buffer.size()};
}

inline std::span<char> Parcel::getMutableSpan() {
    assert(isGood());
    return {getMutableData(), _buffer.size()};
}

inline Parcel::UHOBGOBLIN_InStream Parcel::createInputStream(PZInt64 aPosition) const {
    assert(isGood());
    return UHOBGOBLIN_InStream{getData(), getSize()};
}

inline Parcel::UHOBGOBLIN_OutStream Parcel::createOutputStream(PZInt64 aPosition) {
    assert(isGood());
    return UHOBGOBLIN_OutStream{_buffer, _buffer.begin() + aPosition};
}

inline Parcel::Parcel(UHOBGOBLIN_UnderlyingContainer aBuffer, std::weak_ptr<ParcelPool> aPool)
    : _buffer{std::move(aBuffer)}
    , _pool{std::move(aPool)} //
{
    assert(_buffer.empty() || IS_ALIGNED(_buffer.data(), sizeof(void*)));
    assert(!_pool.expired());
}

#ifndef NDEBUG
#undef IS_ALIGNED
#endif

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UTIL_PARCEL_HPP

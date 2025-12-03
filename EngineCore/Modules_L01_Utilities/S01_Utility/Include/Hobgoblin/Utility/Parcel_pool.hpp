// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UTIL_PARCEL_POOL_HPP
#define UHOBGOBLIN_UTIL_PARCEL_POOL_HPP

#include <Hobgoblin/Common/Positive_or_zero_integer.hpp>
#include <Hobgoblin/Utility/Parcel.hpp>

#include <memory>
#include <mutex>
#include <set>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//! \warning PARCELS AND PARCEL POOL ARE AN UNFINISHED FEATURE -- DO NOT USE!!! //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

//! TODO
class ParcelPool : public std::enable_shared_from_this<ParcelPool> {
public:
    //! TODO
    static std::shared_ptr<ParcelPool> create();

    //! TODO
    Parcel obtainParcel(PZInt64 aCapacity);

    //! TODO
    Parcel obtainParcel(PZInt64 aCapacity, PZInt64 aSize, char aFill = 0);

private:
    friend class Parcel;

    ParcelPool() = default;

    struct BufferCapacityCmp {
        using BufferType     = Parcel::UHOBGOBLIN_UnderlyingContainer;
        using is_transparent = void;

        bool operator()(const BufferType& aLhs, const BufferType& aRhs) const {
            return aLhs.capacity() < aRhs.capacity();
        }

        bool operator()(const BufferType& aLhs, PZInt64 aRhs) const {
            return aLhs.capacity() < pz64tos(aRhs);
        }

        bool operator()(PZInt64 aLhs, const BufferType& aRhs) const {
            return pz64tos(aLhs) < aRhs.capacity();
        }
    };

    std::mutex _mutex;

    std::multiset<Parcel::UHOBGOBLIN_UnderlyingContainer, BufferCapacityCmp> _buffers;

    void _returnBuffer(Parcel::UHOBGOBLIN_UnderlyingContainer aBuffer);
};

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UTIL_PARCEL_POOL_HPP

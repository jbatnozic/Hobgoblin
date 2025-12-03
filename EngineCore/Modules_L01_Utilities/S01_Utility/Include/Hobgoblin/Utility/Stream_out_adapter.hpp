// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UTIL_STREAM_OUT_ADAPTER_HPP
#define UHOBGOBLIN_UTIL_STREAM_OUT_ADAPTER_HPP

#include <Hobgoblin/Utility/Stream_input.hpp>
#include <Hobgoblin/Utility/Stream_nothrow.hpp>
#include <Hobgoblin/Utility/Stream_output.hpp>

#include <Hobgoblin/Math/Core.hpp>

#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

//! Wraps an arbitrary container `taContainer` so that binary data can be written to it using the
//! `OutputStream` interface.
//! \note `taContainer` doesn't *have* to be a random access container using a contiguous block of
//!       memory (this adapter can even append bytes one at a time to a std::list, for example), but
//!       this is strongly encouraged for compatibility with `ViewStream` so that you can later
//!       extract the written data.
template <class taContainer>
// `taContainer` must declare a type alias `value_type` and that type an integral of size 1
    requires(requires { typename taContainer::value_type; } &&
             (sizeof(typename taContainer::value_type) == sizeof(char)) &&
             (alignof(typename taContainer::value_type) == alignof(char)))
class OutputStreamContainerAdapter : public OutputStream {
public:
    //! When this constructor is used, binary data will be written to the end of the `aContainer`
    //! (before its end iterator).
    //! \warning any operation that invalidates iterators of `aContainer` will also invalidate
    //!          this adapter object!
    OutputStreamContainerAdapter(taContainer& aContainer)
        : _container{&aContainer}
        , _iter{std::end(aContainer)} {}

    //! When this constructor is used, binary data will be written to `aContainer`
    //! before the passed iterator.
    //! \warning any operation that invalidates iterators of `aContainer` will also invalidate
    //!          this adapter object!
    OutputStreamContainerAdapter(taContainer& aContainer, typename taContainer::const_iterator aIter)
        : _container{&aContainer}
        , _iter{aIter} {}

    //! \brief Copy constructor.
    OutputStreamContainerAdapter(const OutputStreamContainerAdapter& aOther) = default;

    //! \brief Move constructor.
    OutputStreamContainerAdapter(OutputStreamContainerAdapter&& aOther) = default;

    //! \brief Copy assignment operator.
    OutputStreamContainerAdapter& operator=(const OutputStreamContainerAdapter& aOther) = default;

    //! \brief Move assignment operator.
    OutputStreamContainerAdapter& operator=(OutputStreamContainerAdapter&& aOther) = default;

    //! \brief Destructor.
    ~OutputStreamContainerAdapter() override = default;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: STATE CHECKING                                                  //
    ///////////////////////////////////////////////////////////////////////////

    HG_NODISCARD bool hasWriteError() const override {
        return false;
    }

    HG_NODISCARD bool clearWriteError() override {
        return true;
    }

    HG_NODISCARD bool isGood() const override {
        return true;
    }

private:
    using value_type     = typename taContainer::value_type;
    using const_iterator = typename taContainer::const_iterator;

    taContainer*   _container;
    const_iterator _iter;

    std::int64_t _write(NeverNull<const void*> aData,
                        std::int64_t           aByteCount,
                        bool /*aAllowPartal*/) override {
        if (aData && aByteCount > 0) {
            const auto firstInserted =
                _container->insert(_iter,
                                   static_cast<const value_type*>(aData.get()),
                                   static_cast<const value_type*>(aData.get()) + aByteCount);
            _iter = firstInserted + aByteCount;
        }
        return aByteCount;
    }

    void _setWriteError() override {}

    void _setNotGood() override {}
};

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UTIL_STREAM_OUT_ADAPTER_HPP

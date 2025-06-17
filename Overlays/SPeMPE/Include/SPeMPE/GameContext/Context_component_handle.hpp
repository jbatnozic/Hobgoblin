// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_GAME_CONTEXT_CONTEXT_COMPONENT_HANDLE_HPP
#define SPEMPE_GAME_CONTEXT_CONTEXT_COMPONENT_HANDLE_HPP

#include <SPeMPE/GameContext/Context_components.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO.hpp>

#include <memory>
#include <type_traits>
#include <variant>

namespace jbatnozic {
namespace spempe {

namespace hg = ::jbatnozic::hobgoblin;

//! An owning handle for any object derived from `ContextComponent`.
//! Works pretty much like a `std::unique_ptr` in that it manages the held object's lifetime,
//! and can be constructed from either a `std::unique_ptr` or a `QAO_GenericHandle`, for the common
//! case where a context component is derived from `QAO_Base` and thus has to be managed by a
//! QAO handle object.
template <class taCComp = ContextComponent>
class ContextComponentHandle {
public:
    static_assert(
        std::is_base_of_v<ContextComponent, taCComp>,
        "ContextComponentHandle can only be used with classes derived from ContextComponents.");

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CONSTRUCTION                                                    //
    ///////////////////////////////////////////////////////////////////////////

    //! Default constructor; makes an empty null handle.
    ContextComponentHandle()
        : _ccompPointer{nullptr}
        , _lifetimeManager{std::unique_ptr<taCComp>(nullptr)} {}

    //! Construction from nullptr is the same as default construction.
    ContextComponentHandle(std::nullptr_t)
        : ContextComponentHandle{} {}

    //! Construct from a unique_ptr to a ContextComponent.
    ContextComponentHandle(std::unique_ptr<taCComp> aUPtr)
        : _ccompPointer{aUPtr.get()}
        , _lifetimeManager{std::move(aUPtr)} {}

    //! Construct from a QAO_Handle to a ContextComponent that's also a QAO object.
    //! The passed must be owning (or null), otherwise `InvalidArgumentError` will be thrown.
    // clang-format off
    template <class U,
              typename std::enable_if_t<std::is_base_of_v<hg::QAO_Base,     U> &&
                                        std::is_base_of_v<ContextComponent, U>, bool> = true>
    // clang-format on
    ContextComponentHandle(hg::QAO_Handle<U> aHandle)
        : _ccompPointer{aHandle.ptr()}
        , _lifetimeManager{std::move(aHandle)} //
    {
        HG_VALIDATE_ARGUMENT(_ccompPointer == nullptr ||
                                 std::get<VARIANT_QAOHANDLE>(_lifetimeManager).isOwning(),
                             "ContextComponentHandle must be constructed from an owning QAO handle.");
    }

    //! Prevent copy-construction.
    ContextComponentHandle(const ContextComponentHandle& aOther)            = delete;
    ContextComponentHandle& operator=(const ContextComponentHandle& aOther) = delete;

    //! Move constructor.
    //! \note leaves `aOther` in anempty (null) state.
    ContextComponentHandle(ContextComponentHandle&& aOther) noexcept
        : _ccompPointer{aOther._ccompPointer}
        , _lifetimeManager{std::move(aOther._lifetimeManager)} //
    {
        aOther._ccompPointer    = nullptr;
        aOther._lifetimeManager = {};
    }

    //! Move assignment operator.
    //! \note leaves `aOther` in anempty (null) state.
    ContextComponentHandle& operator=(ContextComponentHandle&& aOther) noexcept {
        if (&aOther != this) {
            _ccompPointer           = aOther._ccompPointer;
            _lifetimeManager        = std::move(aOther._lifetimeManager);
            aOther._ccompPointer    = nullptr;
            aOther._lifetimeManager = {};
        }
        return *this;
    }

    //! Move constructor from a handle to a subclass of `taCComp`.
    // clang-format off
    template <class U,
              typename std::enable_if_t<std::is_convertible_v<U*, taCComp*>, bool> = true>
    // clang-format on
    ContextComponentHandle(ContextComponentHandle<U>&& aOther)
        : _ccompPointer{aOther._ccompPointer}
        , _lifetimeManager{std::move(aOther._lifetimeManager)} {
        aOther._ccompPointer    = nullptr;
        aOther._lifetimeManager = {};
    }

    //! Move assignment from a handle to a subclass of `taCComp`.
    // clang-format off
    template <class U,
              typename std::enable_if_t<std::is_convertible_v<U*, taCComp*>, bool> = true>
    // clang-format on
    ContextComponentHandle& operator=(ContextComponentHandle<U>&& aOther) {
        if ((void*)&aOther != (void*)this) {
            _ccompPointer           = aOther._ccompPointer;
            _lifetimeManager        = std::move(aOther._lifetimeManager);
            aOther._ccompPointer    = nullptr;
            aOther._lifetimeManager = {};
        }
        return *this;
    }

    //! Move-construct and return a handle to `U`, where `U` is a subtype of `taCComp`.
    // clang-format off
    template <class U,
              typename std::enable_if_t<std::is_base_of_v<taCComp, U>, bool> = true>
    // clang-format on
    ContextComponentHandle<U> downcastMove() {
        ContextComponentHandle<U> result;
        result._ccompPointer    = _ccompPointer;
        result._lifetimeManager = std::move(_lifetimeManager);
        _ccompPointer           = nullptr;
        _lifetimeManager        = {};
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: NULL CHECKING                                                   //
    ///////////////////////////////////////////////////////////////////////////

    operator bool() const {
        return _ccompPointer != nullptr;
    }

    bool isNull() const {
        return _ccompPointer == nullptr;
    }

    friend bool operator==(const ContextComponentHandle& aSelf, std::nullptr_t) {
        return aSelf.ptr() == nullptr;
    }

    friend bool operator!=(const ContextComponentHandle& aSelf, std::nullptr_t) {
        return aSelf.ptr() != nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: DEREFERENCING                                                   //
    ///////////////////////////////////////////////////////////////////////////

    ContextComponent* ptr() const {
        return _ccompPointer;
    }

    ContextComponent* operator->() const {
        return _ccompPointer;
    }

    ContextComponent& operator*() const {
        return *_ccompPointer;
    }

private:
    template <class U>
    friend class ContextComponentHandle;

    ContextComponent*                                                                  _ccompPointer;
    std::variant<std::unique_ptr<ContextComponent>, hobgoblin::qao::QAO_GenericHandle> _lifetimeManager;

    static constexpr std::size_t VARIANT_UPOINTER  = 0;
    static constexpr std::size_t VARIANT_QAOHANDLE = 1;
};

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_GAME_CONTEXT_CONTEXT_COMPONENT_HANDLE_HPP

// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_BASE_HPP
#define UHOBGOBLIN_QAO_BASE_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Execon.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Instantiation_guard.hpp>
#include <Hobgoblin/QAO/Name_ref.hpp>
#include <Hobgoblin/QAO/Orderer.hpp>
#include <Hobgoblin/QAO/Room.hpp>
#include <Hobgoblin/Utility/Any_ptr.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include <cstdint>
#include <string>
#include <string_view>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Runtime;

// Forward-declare the create functions:
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
// Forward-declare the destroy function:
void QAO_Destroy(QAO_GenericHandle&& aHandle);

class QAO_Base
    : NO_COPY
    , NO_MOVE {
public:
    QAO_Base() = delete;

    //! \brief Constructor.
    //! \param aInstGuard instantiation guard to make sure the instance is created using `QAO_Create`.
    //! \param aExeconThreshold the EXECON threshold of the instance. Essentially it is the importance
    //!                         score of the instance - events of this instance will only be executed
    //!                         while the EXECON level of the runtime is equal to or higher than this
    //!                         threshold.
    //! \param aExecutionPriority execution priority of the instance. Events of instances with higher
    //!                           priority get executed before those of instances with lower priority.
    //! \param aName name of the instance (it doesn't have to follow any specific pattern because the
    //!              QAO framework doesn't use it except for logging; it's up to the user to choose).
    //!              Max 255 characters. Use `QAO_STATIC_NAME` macro to set a static name and avoid
    //!              dynamic memory allocations (better performance).
    QAO_Base(QAO_InstGuard aInstGuard,
             QAO_ExeCon    aExeconThreshold,
             int           aExecutionPriority,
             QAO_NameRef   aName);

    virtual ~QAO_Base() = 0;

    QAO_Runtime* getRuntime() const noexcept;

    //! for objects not attached to a runtime or not owned by their runtime, returns
    //! `QAO_INVALID_ROOM_ID`.
    QAO_RoomId getRoomId() const;

    void       setExeconThreshold(QAO_ExeCon aExeconThreshold);
    QAO_ExeCon getExeconThreshold() const;

    void setExecutionPriority(int priority);
    int  getExecutionPriority() const noexcept;

    //! \brief set a new name for the instance.
    //! \param aName the name to set (max 255 characters). Use `QAO_STATIC_NAME` to set a static name
    //!              and avoid dynamic memory allocations (better performance).
    void             setName(QAO_NameRef aName);
    std::string_view getName() const;

    QAO_GenericId getId() const noexcept;

    std::string getDebugDescription() const;

protected:
    // Lifecycle callbacks

    //! \brief Called right after the object is created (after the constructor returns).
    //!
    //! The only use case that this function has, and the only advantage it has over the constructor,
    //! is that the dynamic type of the instance is fully resolved and available at the time of calling.
    //!
    //! \warning don't forget to call the `_setUp()` method of the superclass before anything else when
    //!          overriding this method!
    //!
    //! \warning DO NOT attempt to destroy or attach/detach (directly or indirectly) the object in this
    //!          method! It will lead to catastrophic bugs!
    //!
    //! \note if `_setUp()` throws, object creation (the `QAO_Create` call) will fail, and,
    //!       analogously to a throwing constructor, `_tearDown()` will NOT be called during cleanup!
    virtual void _setUp();

    //! \brief Called right after the object is attached to a runtime.
    //!
    //! Use this method to do any setup or initialization that requires the object to have the overall
    //! context that being attached to a runtime provides.
    //!
    //! \warning don't forget to call the `_didAttach()` method of the superclass before anything else
    //!          when overriding this method!
    //!
    //! \warning DO NOT attempt to destroy or attach/detach (directly or indirectly) the object in this
    //!          method! It will lead to catastrophic bugs!
    //!
    //! \note if `_didAttach()` throws, attaching the object will fail, and, analogously to a throwing
    //!       constructor, `_willDetach()` will NOT be called during cleanup!
    virtual void _didAttach(QAO_Runtime& aRuntime);

    //! \brief Called just before the object is detached from its runtime.
    //!
    //! This method's implementation should do minimal work outside of nulling out some fields
    //! or unsubscribing the object from runtime-bound components; in general, it should only prepare
    //! the object to exist without the overall context that being attached to a runtime provides.
    //!
    //! \warning don't forget to call the `_willDetach()` method of the superclass after anything else
    //!          when overriding this method!
    //!
    //! \warning DO NOT attempt to destroy or attach/detach (directly or indirectly) the object in this
    //!          method! It will lead to catastrophic bugs!
    //!
    //! \note the QAO library is set up so that it always swallows and never propagates any exceptions
    //!       that escape from `_willDetach()`. (there is no perfect way to handle this: any serious
    //!       error - including the inability to properly detach an object from its runtime - most
    //!       likely leads the whole runtime into an invalid state, where the only possible recovery is
    //!       to tear it down, so it was decided that exceptions from `_willDetach()` must not prevent
    //!       that. On the bright side, the operations usually done in this method should be
    //!       sufficiently simple so that producing an exception is really unlikely.)
    virtual void _willDetach(QAO_Runtime& aRuntime);

    //! \brief Called during destruction, right before the destructor call.
    //!
    //! The only use case that this function has, and the only advantage it has over the destructor,
    //! is that the dynamic type of the instance is still resolved and available at the time of calling.
    //!
    //! \warning don't forget to call the `_tearDown()` method of the superclass after anything else
    //!          when overriding this method!
    //!
    //! \warning DO NOT attempt to destroy or attach/detach (directly or indirectly) the object in this
    //!          method! It will lead to catastrophic bugs!
    //!
    //! \note exceptions that escape this method are always swallowed (see the description of
    //!       `_willDetach` for the reasoning).
    virtual void _tearDown();

private:
    // Befriend `QAO_Create*` so that it can call `_setUp()` when needed.
    template <class T, class... taArgs>
    friend QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
    template <class T, class... taArgs>
    friend QAO_Handle<T> QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY(QAO_RuntimeRef aRuntimeRef,
                                                                  taArgs&&... aArgs);
    // Befriend `QAO_Destroy` so that it can call `_tearDown()` when needed.
    friend void QAO_Destroy(QAO_GenericHandle&& aHandle);

    struct Context {
        std::int64_t        stepOrdinal = 0;
        QAO_GenericId       id;
        QAO_OrdererIterator ordererIterator;
        QAO_Runtime*        runtime = nullptr;
        QAO_RoomId          roomId  = QAO_INVALID_ROOM_ID;
    };

    const char*   _instanceName = nullptr;
    Context       _context;
    std::int32_t  _executionPriority;
    std::uint16_t _flags           = 0;
    std::uint8_t  _nameLength      = 0;
    QAO_ExeCon    _execonThreshold = QAO_ExeCon::META_EXECUTE_NONE;

    enum Flags : std::uint16_t {
        SET_UP_PROPERLY_BIT    = (1u << 15u),
        TORN_DOWN_PROPERLY_BIT = (1u << 14u),
        ATTACHED_PROPERLY_BIT  = (1u << 13u),
        DETACHED_PROPERLY_BIT  = (1u << 12u),
        NAME_IS_STATIC_BIT     = (1u << 11u),
    };

    using EventHandlerPointer = void(QAO_Base::*)();
    static const EventHandlerPointer EVENT_FUNC_TABLE[QAO_Event::EVENT_COUNT];

    // Update
    virtual void _eventPreUpdate() {}
    virtual void _eventBeginUpdate() {}
    virtual void _eventUpdate1() {}
    virtual void _eventUpdate2() {}
    virtual void _eventEndUpdate() {}
    virtual void _eventPostUpdate() {}

    // Draw
    virtual void _eventPreDraw() {}
    virtual void _eventDraw1() {}
    virtual void _eventDraw2() {}
    virtual void _eventDrawGUI() {}
    virtual void _eventPostDraw() {}

    // Display
    virtual void _eventDisplay() {}

    void _callEvent(QAO_Event::Enum ev);

    friend class QAO_Runtime;
    friend class QAO_GenericId;
};

} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_QAO_BASE_HPP

// clang-format on

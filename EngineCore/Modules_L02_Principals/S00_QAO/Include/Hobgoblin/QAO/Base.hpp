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
#include <Hobgoblin/QAO/Orderer.hpp>
#include <Hobgoblin/Utility/Any_ptr.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include <cstdint>
#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {

class QAO_Runtime;

// Forward-declare the create function:
template <class T, class... taArgs>
QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
// Forward-declare the destroy function:
void QAO_Destroy(QAO_GenericHandle&& aHandle);

class QAO_Base
    : NO_COPY
    , NO_MOVE {
public:
    QAO_Base() = delete;

    //! \brief Constructor.
    //! \param aInstGuard instantiation guard to make sure the instance is created using `QAO_Create`.
    //! \param aExecutionPriority execution priority of the instance. Events of instances with higher
    //!                           priority get executed before those of instances with lower priority.
    //! \param aName name of the instance (it doesn't have to follow any specific pattern because the
    //!              QAO framework doesn't use it except for logging; it's up to the user to choose).
    //!
    //! \note this constructor doesn't take a QAO_ExeCon parameter, so it defaults to `ESSENTIAL`.
    [[deprecated("Use the other constructor which also accepts a QAO_ExeCon parameter.")]]
    QAO_Base(QAO_InstGuard aInstGuard, int aExecutionPriority, std::string aName);

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
    QAO_Base(QAO_InstGuard aInstGuard,
             QAO_ExeCon    aExeconThreshold,
             int           aExecutionPriority,
             std::string   aName);

    virtual ~QAO_Base() = 0;

    QAO_Runtime* getRuntime() const noexcept;

    void       setExeconThreshold(QAO_ExeCon aExeconThreshold);
    QAO_ExeCon getExeconThreshold() const;

    void setExecutionPriority(int priority);
    int  getExecutionPriority() const noexcept;

    void        setName(std::string newName);
    std::string getName() const;

    QAO_GenericId getId() const noexcept;

protected:
    // Lifecycle callbacks
    virtual void _setUp();
    virtual void _tearDown();
    virtual void _didAttach(QAO_Runtime& aRuntime);
    virtual void _willDetach(QAO_Runtime& aRuntime);

private:
    // Befriend `QAO_Create` so that it can call `_setUp()` when needed.
    template <class T, class... taArgs>
    friend QAO_Handle<T> QAO_Create(QAO_RuntimeRef aRuntimeRef, taArgs&&... aArgs);
    // Befriend `QAO_Destroy` so that it can call `_tearDown()` when needed.
    friend void QAO_Destroy(QAO_GenericHandle&& aHandle);

    struct Context {
        std::int64_t        stepOrdinal = 0;
        QAO_GenericId       id;
        QAO_OrdererIterator ordererIterator;
        QAO_Runtime*        runtime = nullptr;
    };

    std::string   _instanceName;
    Context       _context;
    std::int32_t  _executionPriority;
    std::uint32_t _flags = 0;

    enum Flags : std::uint32_t {
        SET_UP_PROPERLY_BIT    = (1u << 31u),
        TORN_DOWN_PROPERLY_BIT = (1u << 30u),
        ATTACHED_PROPERLY_BIT  = (1u << 29u),
        DETACHED_PROPERLY_BIT  = (1u << 28u),

        FLAGS_EXECON_BYTE_OFFSET = 2 //!< Execon threshold is stored in bits 16..23
    };

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

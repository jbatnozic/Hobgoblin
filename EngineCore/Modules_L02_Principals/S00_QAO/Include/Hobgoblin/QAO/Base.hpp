// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_QAO_BASE_HPP
#define UHOBGOBLIN_QAO_BASE_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Config.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Id.hpp>
#include <Hobgoblin/QAO/Instantiation_key.hpp>
#include <Hobgoblin/QAO/Orderer.hpp>
#include <Hobgoblin/Utility/Any_ptr.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>
#include <Hobgoblin/Utility/Packet.hpp>

#include <string>
#include <typeinfo>

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

    QAO_Base(QAO_IKey              aInstantiationKey,
             const std::type_info& aTypeInfo,
             int                   aExecutionPriority,
             std::string           aName);

    virtual ~QAO_Base() = 0;

    QAO_Runtime* getRuntime() const noexcept;

    void setExecutionPriority(int priority);
    int  getExecutionPriority() const noexcept;

    void        setName(std::string newName);
    std::string getName() const;

    QAO_GenericId getId() const noexcept;

    const std::type_info& getTypeInfo() const;

    virtual bool message(int tag, util::AnyPtr context);

    friend util::OutputStream& operator<<(util::OutputStreamExtender& ostream, const QAO_Base& self);

protected:
    // Lifecycle callbacks
    virtual void _setUp() {}
    virtual void _tearDown() {}
    virtual void _didAttach(QAO_Runtime& aRuntime) {}
    virtual void _willDetach(QAO_Runtime& aRuntime) {}

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

    std::string           _instanceName;
    Context               _context;
    const std::type_info& _typeInfo;
    int                   _execution_priority;

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

#ifndef UHOBGOBLIN_UTIL_ANY_PTR_HPP
#define UHOBGOBLIN_UTIL_ANY_PTR_HPP

#include <Hobgoblin/Common.hpp>

#include <cassert>
#include <type_traits>
#include <typeinfo>
#include <sstream>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

class AnyPtr {
public:
    AnyPtr() {
        reset();
    }

    AnyPtr(std::nullptr_t) {
        reset(nullptr);
    }

    template <class T>
    AnyPtr(T* value) {
        reset(value);
    }

    void reset() {
        _value = nullptr;
        _valueType = &typeid(std::nullptr_t);
        _valueIsConst = false;
    }

    void reset(std::nullptr_t) {
        _value = nullptr;
        _valueType = &typeid(std::nullptr_t);
        _valueIsConst = false;
    }

    template <class T>
    void reset(T* value) {
        _value = value;
        _valueType = &typeid(T);
        _valueIsConst = std::is_const<T>::value;
    }

    template <class T>
    T* get() const {
        assert(typeid(T) == *_valueType);
        assert(!_valueIsConst || std::is_const<T>::value);
        return const_cast<T*>(static_cast<const T*>(_value));
    }

    template <class T>
    T* getOrThrow() const {
        if (typeid(T) != *_valueType) {
            std::stringstream ss;
            ss << "AnyPtr type mismatch (held: " << _valueType->name() << "; requested: " << typeid(T).name() << ")";
            throw TracedLogicError(ss.str());
        }
        else if (_valueIsConst && !std::is_const<T>::value) {
            std::stringstream ss;
            ss << "AnyPtr const qualifier mismatch";
            throw TracedLogicError(ss.str());
        }
        return const_cast<T*>(static_cast<const T*>(_value));
    }

    bool operator==(const AnyPtr& other) {
        return _value == other._value && 
               *_valueType == *other._valueType && 
               _valueIsConst == other._valueIsConst;
    }

    bool operator!=(const AnyPtr& other) {
        return !(SELF == other);
    }

    bool operator==(const std::nullptr_t) {
        return _value == nullptr;
    }

    bool operator!=(const std::nullptr_t) {
        return _value != nullptr;
    }

private:
    const void* _value;
    const std::type_info* _valueType;
    bool _valueIsConst;
};

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UTIL_ANY_PTR_HPP
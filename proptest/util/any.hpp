#pragma once

#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/type.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/exception.hpp"

namespace proptest
{

struct PROPTEST_API AnyHolder {
    virtual const type_info& type() const = 0;
    virtual ~AnyHolder() {}

    template <typename T>
    const T& getRef() const {
        if constexpr(is_lvalue_reference_v<T>) {
            using RawT = decay_t<T>;
            return *const_cast<RawT*>(static_cast<const RawT*>(rawPtr()));
        }
        else
            return *static_cast<const T*>(rawPtr());
    }

    template <typename T>
    T& getMutableRef() const {
        if constexpr(is_lvalue_reference_v<T>) {
            using RawT = decay_t<T>;
            return *const_cast<RawT*>(static_cast<const RawT*>(rawPtr()));
        }
        else
            return *const_cast<T*>(static_cast<const T*>(rawPtr()));
    }

    virtual shared_ptr<AnyHolder> clone() const = 0;

protected:
    // for cast()
    virtual const void* rawPtr() const = 0;
};

template <typename T> concept equality_check_available = requires(T a, T b) {
    a == b;
};

template <typename T>
struct PROPTEST_API AnyVal : AnyHolder {
    const type_info& type() const override { return typeid(T); }
    virtual ~AnyVal() {}

    AnyVal(const T& val) : value(val) {
    }

    const void* rawPtr() const override {
        return &value;
    }

    bool operator==(const T& other) {
        if constexpr(equality_check_available<T>) {
            return value == other;
        }
        else
            return false;
    }

    virtual shared_ptr<AnyHolder> clone() const override {
        return util::make_shared<AnyVal<T>>(value);
    }

    T value;
};

template <typename T>
struct PROPTEST_API AnyLValRef : AnyHolder {
    const type_info& type() const override { return typeid(T); }
    virtual ~AnyLValRef() {}

    AnyLValRef(const T& val) : value(val) {
    }

    const void* rawPtr() const override {
        return &value;
    }

    bool operator==(const T& other) {
        return &value == &other;
    }

    virtual shared_ptr<AnyHolder> clone() const override {
        return util::make_shared<AnyLValRef<T>>(value); // note: inherently, lvalue reference is not a copy
    }

    const T& value;
};


template <typename T>
struct PROPTEST_API AnyRef : AnyHolder {
    const type_info& type() const override { return typeid(T); }
    virtual ~AnyRef() {}

    AnyRef(const T& t) : ptr(static_pointer_cast<void>(util::make_shared<T>(t))) {
    }

    AnyRef(const shared_ptr<T>& tptr) : ptr(static_pointer_cast<void>(tptr)) {
    }

    const void* rawPtr() const override {
        return ptr.get();
    }

    bool operator==(const T& other) {
        if constexpr(equality_check_available<T>) {
            return static_pointer_cast<T>(ptr)->operator==(*static_pointer_cast<T>(other.ptr));
        }
        else
            return false;
    }

    virtual shared_ptr<AnyHolder> clone() const override {
        if constexpr(copy_constructible<T>)
            return util::make_shared<AnyRef<T>>(*static_pointer_cast<T>(ptr));
        else
            throw runtime_error(__FILE__, __LINE__, "cannot clone AnyRef of a type with no copy constructor: " + string(type().name()));
    }

private:
    shared_ptr<void> ptr;
};

struct PROPTEST_API Any {
    static const Any empty;

    Any() = default;
    virtual ~Any() {}

    Any(const Any& other);

    template <typename T>
    Any(const T& t) {
        if constexpr(is_lvalue_reference_v<T>) {
            ptr = util::make_shared<AnyLValRef<decay_t<T>>>(t);
        }
        else if constexpr (is_fundamental_v<T>) {
            ptr = util::make_shared<AnyVal<T>>(t);
        }
        else {
            ptr = util::make_shared<AnyRef<T>>(util::make_shared<T>(t));
        }
    }

    template <typename T>
    Any(const shared_ptr<AnyLValRef<T>>& holderPtr) : ptr(holderPtr){
    }

    template <typename T>
    Any(const shared_ptr<AnyRef<T>>& holderPtr) : ptr(holderPtr){
    }

    template <typename T>
    Any(const shared_ptr<AnyVal<T>>& holderPtr) : ptr(holderPtr){
    }

    Any(const shared_ptr<AnyHolder>& holderPtr);

    Any clone() const;

    Any& operator=(const Any& other);

    // bool operator==(const Any& other);

    template <typename T>
    const T& getRef(bool skipCheck = false) const {
        if constexpr(is_same_v<decay_t<T>, Any>)
            return *this;
        else {
            if(!ptr)
                throw invalid_cast_error(__FILE__, __LINE__, "no value in an empty Any");
            if(!skipCheck && type() != typeid(T)) {
                throw invalid_cast_error(__FILE__, __LINE__, "cannot cast from " + string(type().name()) + " to " + string(typeid(T).name()));
            }
            return ptr->getRef<T>();
        }
    }

    template <typename T>
    T& getMutableRef(bool skipCheck = false) {
        if constexpr(is_same_v<decay_t<T>, Any>)
            return *this;
        else {
            if(!ptr)
                throw invalid_cast_error(__FILE__, __LINE__, "no value in an empty Any");
            if(!skipCheck && type() != typeid(T)) {
                throw invalid_cast_error(__FILE__, __LINE__, "cannot cast from " + string(type().name()) + " to " + string(typeid(T).name()));
            }
            return ptr->getMutableRef<T>();
        }
    }

    template <typename T>
    T getValue() const {
        return getRef<T>();
    }

    const type_info& type() const;

    bool isEmpty() const;

private:
    shared_ptr<AnyHolder> ptr;
};

namespace util {

template <typename T>
AnyVal<T> make_anyval(T value)
{
    return AnyVal<T>{value};
}

template <typename T, typename... Args>
Any make_any(Args&&... args)
{
    if constexpr (is_same_v<Any, decay_t<T>>) {
        static_assert(sizeof...(Args) == 1, "a value must be provided as argument");
        return Any{args...};
    }
    else if constexpr(is_lvalue_reference_v<T>) {
        static_assert(sizeof...(Args) == 1, "an l-value reference must be provided as argument");
        return Any{util::make_shared<AnyLValRef<decay_t<T>>>(args...)};
    }
    else if constexpr (is_fundamental_v<T>) {
        return Any{util::make_shared<AnyVal<T>>(T{args...})};
    }
    else {
        return Any(util::make_shared<AnyRef<T>>(util::make_shared<T>(args...)));
    }
}

template <typename T>
shared_ptr<Any> make_shared_any(const shared_ptr<T>& ptr)
{
    auto anyPtr = util::make_shared<Any>(ptr);
    return anyPtr;
}

}

template <typename T>
using AnyT = Any;

}  // namespace proptest

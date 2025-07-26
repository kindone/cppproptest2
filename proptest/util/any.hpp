#pragma once

#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/type.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/util/define.hpp"

namespace proptest
{

struct PROPTEST_API AnyHolder {
    virtual const type_info& type() const = 0;
    virtual ~AnyHolder() {}

    template <typename T>
    const T& getRef() const {
        return *static_cast<const decay_t<T>*>(rawPtr());
    }

    template <typename T>
        requires (is_fundamental_v<T> || is_pointer_v<T> || is_reference_v<T>)
    decltype(auto) getRef() const {
        return *static_cast<const decay_t<T>*>(rawPtr());
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

    AnyVal(T&& val) : value(util::move(val)) {
    }

    AnyVal(const T& val) : value(val) {
    }

    const void* rawPtr() const override {
        return &value;
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

    virtual shared_ptr<AnyHolder> clone() const override {
        return util::make_shared<AnyLValRef<T>>(value); // note: inherently, lvalue reference is not a copy
    }

    const T& value;
};


template <typename T>
struct PROPTEST_API AnyRef : AnyHolder {
    const type_info& type() const override { return typeid(T); }
    virtual ~AnyRef() {}

    AnyRef(T&& t) : ptr(util::make_unique<T>(util::move(t))) {
    }

    AnyRef(const T& t) : ptr(util::make_unique<T>(t)) {
    }

    AnyRef(unique_ptr<T>&& tptr) : ptr(util::move(tptr)) {
    }

    const void* rawPtr() const override {
        return ptr.get();
    }

    virtual shared_ptr<AnyHolder> clone() const override {
        if constexpr(copy_constructible<T>)
            return util::make_shared<AnyRef<T>>(*ptr);
        else
            throw runtime_error(__FILE__, __LINE__, "cannot clone AnyRef of a type with no copy constructor: " + string(type().name()));
    }

private:
    unique_ptr<T> ptr;
};

struct PROPTEST_API Any {
    static const Any empty;

    Any() = default;
    virtual ~Any();

    Any(const Any& other);
    Any(Any&& other);

    template <typename T>
        requires (!is_lvalue_reference_v<T>)
    Any(T&& t) {
        if constexpr(is_fundamental_v<T>) {
            ptr = util::make_shared<AnyVal<T>>(util::move(t));
        }
        else if constexpr(is_copy_constructible_v<T>) {
            ptr = util::make_shared<AnyVal<T>>(t);
        }
        else if constexpr(is_move_constructible_v<T>) {
            ptr = util::make_shared<AnyRef<T>>(util::move(t));
        }
        else {
            throw runtime_error(__FILE__, __LINE__, "Any cannot be constructed from a type that is neither copy-constructible nor move-constructible: " + string(typeid(T).name()));
        }
    }

    template <typename T>
    Any(const T& t) {
        static_assert(is_same_v<decay_t<T>, Any> || is_move_constructible_v<T> || is_copy_constructible_v<T>);
        if constexpr(is_lvalue_reference_v<T>) {
            ptr = util::make_shared<AnyLValRef<decay_t<T>>>(t);
        }
        else if constexpr(is_fundamental_v<T> || is_copy_constructible_v<T>) {
            ptr = util::make_shared<AnyVal<T>>(t);
        }
        else {
            ptr = util::make_shared<AnyRef<T>>(util::make_shared<T>(t));
        }
    }

    template <typename T>
    Any(const shared_ptr<AnyLValRef<T>>& holderPtr) : ptr(holderPtr){}
    template <typename T>
    Any(shared_ptr<AnyLValRef<T>>&& holderPtr) : ptr(holderPtr){}

    template <typename T>
    Any(const shared_ptr<AnyRef<T>>& holderPtr) : ptr(holderPtr){}
    template <typename T>
    Any(shared_ptr<AnyRef<T>>&& holderPtr) : ptr(holderPtr){}

    template <typename T>
    Any(const shared_ptr<AnyVal<T>>& holderPtr) : ptr(holderPtr){}

    template <typename T>
    Any(shared_ptr<AnyVal<T>>&& holderPtr) : ptr(holderPtr){}

    Any(const shared_ptr<AnyHolder>& holderPtr);
    Any(shared_ptr<AnyHolder>&& holderPtr);

    Any clone() const;

    Any& operator=(const Any& other);

    // bool operator==(const Any& other);

    template <typename T>
    decltype(auto) getRef(bool skipCheck = false) const {
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
    // static_assert((!is_same_v<decay_t<Args, shared_ptr<T>> && ...), "shared_ptr<T> must not be passed as argument");

    if constexpr (is_same_v<Any, decay_t<T>>) {
        static_assert(sizeof...(Args) == 1, "a value must be provided as argument");
        return Any{util::forward<Args>(args)...};
    }
    else if constexpr(sizeof...(Args) == 1 && (is_same_v<Any, decay_t<Args>> && ...)) {
        return Any{util::forward<Args>(args)...};
    }
    else if constexpr(is_lvalue_reference_v<T>) {
        static_assert(sizeof...(Args) == 1, "an l-value reference must be provided as argument");
        return Any{util::make_shared<AnyLValRef<decay_t<T>>>(util::forward<Args>(args)...)};
    }
    else if constexpr (sizeof...(Args) == 1 && (is_same_v<decay_t<Args>, unique_ptr<T>> && ...)) {
        return Any(util::make_shared<AnyRef<T>>(util::forward<Args>(args)...));
    }
    else if constexpr (is_fundamental_v<decay_t<T>> || is_copy_constructible_v<decay_t<T>>){
        return Any{util::make_shared<AnyVal<T>>(util::forward<T>(T(util::forward<Args>(args)...)))};
    }
    else {
        return Any(util::make_shared<AnyRef<T>>(util::make_unique<T>(util::forward<Args>(args)...)));
    }
}

}

template <typename T>
using AnyT = Any;

}  // namespace proptest

#ifdef PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

#define EXTERN_DECLARE_ANYVAL(TYPE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::AnyVal, TYPE)
#define EXTERN_DECLARE_ANYREF(TYPE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::AnyRef, TYPE)

DEFINE_FOR_ALL_BASIC_TYPES(EXTERN_DECLARE_ANYVAL);
DEFINE_FOR_ALL_STRINGTYPES(EXTERN_DECLARE_ANYREF);

#endif // PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

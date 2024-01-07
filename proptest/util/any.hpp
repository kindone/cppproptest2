#pragma once

#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/type.hpp"
#include "proptest/std/exception.hpp"

namespace proptest
{

struct PROPTEST_API AnyHolder {
    virtual const type_info& type() const = 0;
    virtual ~AnyHolder() {}

    template <typename T>
    const T& getRef() const {
        return *static_cast<const T*>(rawPtr());
    }

protected:
    // for cast()
    virtual const void* rawPtr() const = 0;
};

template <typename T>
struct PROPTEST_API AnyVal : AnyHolder {
    const type_info& type() const override { return typeid(T); }

    AnyVal(const T& val) : value(val) {
    }

    const void* rawPtr() const override {
        return &value;
    }

    T value;
};


template <typename T>
struct PROPTEST_API AnyRef : AnyHolder {
    const type_info& type() const override { return typeid(T); }

    AnyRef(const T& t) : ptr(static_pointer_cast<void>(util::make_shared<T>(t))) {
    }

    AnyRef(const shared_ptr<T>& tptr) : ptr(static_pointer_cast<void>(tptr)) {
    }

    const void* rawPtr() const override {
        return ptr.get();
    }

private:
    shared_ptr<void> ptr;
};

struct PROPTEST_API Any {
    static const Any empty;

    Any() = default;

    template <typename T>
    Any(const T& t) {
        if constexpr (is_fundamental_v<T>) {
            ptr = util::make_shared<AnyVal<T>>(t);
        }
        else {
            ptr = util::make_shared<AnyRef<T>>(util::make_shared<T>(t));
        }
    }

    template <typename T>
    Any(const shared_ptr<AnyRef<T>>& holderPtr) : ptr(holderPtr){
    }

    template <typename T>
    Any(const shared_ptr<AnyVal<T>>& holderPtr) : ptr(holderPtr){
    }

    Any& operator=(const Any& other);

    template <typename T>
    const T& getRef() const {
        if(!ptr)
            throw invalid_cast_error("cannot getRef from an empty Any");
         if(ptr->type() != typeid(T)) {
            throw invalid_cast_error("cannot getRef from " + string(ptr->type().name()) + " to " + string(typeid(T).name()));
        }
        return ptr->getRef<T>();
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
    if constexpr (is_fundamental_v<T>) {
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


}  // namespace proptest

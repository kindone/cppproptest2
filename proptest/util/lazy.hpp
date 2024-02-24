#pragma once

#include "proptest/std/optional.hpp"
#include "proptest/util/anyfunction.hpp"

namespace proptest {

template <typename T>
struct Lazy {
    Lazy(const T& _value) : value(_value) {}
    Lazy(Function<T()> _eval) : eval(_eval) {}

    T& getRef() const {
        if(!value.has_value())
        {
            value = (*eval)();
        }
        return *value;
    }

    T& operator*() const {
        return getRef();
    }

    const T* operator->() const {
        return &getRef();
    }

private:
    mutable optional<T> value;
    optional<Function<T()>> eval;
};

}
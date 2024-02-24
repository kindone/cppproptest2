#pragma once

#include "proptest/util/anyfunction.hpp"

namespace proptest {

template <typename T>
struct Lazy {
    Lazy(const T& _value) : value(util::make_shared<T>(_value)) {}
    Lazy(Function<T()> _eval) : eval(_eval) {}

    T& getRef() const {
        if(!value)
        {
            value = util::make_shared<T>(eval());
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
    mutable shared_ptr<T> value;
    Function<T()> eval;
};

}
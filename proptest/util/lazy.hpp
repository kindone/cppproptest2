#pragma once

#include "proptest/std/optional.hpp"
#include "proptest/util/anyfunction.hpp"

namespace proptest {

template <typename T>
struct Lazy {

    struct LazyBody {
        LazyBody(const T& _value) : value(_value) {}
        LazyBody(Function<T()> _eval) : eval(_eval) {}

        T& getRef() const {
            if(!value.has_value())
            {
                value = eval();
            }
            return *value;
        }

        T& operator*() const {
            return getRef();
        }

        const T* operator->() const {
            return &getRef();
        }

        mutable optional<T> value;
        Function<T()> eval;
    };

    Lazy(const T& _value) : body(util::make_shared<LazyBody>(_value)) {}
    Lazy(Function<T()> _eval) : body(util::make_shared<LazyBody>(_eval)) {}

    T& getRef() const {
        return body->getRef();
    }

    T& operator*() const {
        return body->operator*();
    }

    const T* operator->() const {
        return body->operator->();
    }

private:
    shared_ptr<LazyBody> body;
};

}

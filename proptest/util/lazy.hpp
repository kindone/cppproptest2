#pragma once

#include "proptest/std/optional.hpp"
#include "proptest/util/function.hpp"
#include "proptest/std/variant.hpp"

namespace proptest {

template <typename T>
struct Lazy {

    struct LazyBody {
        LazyBody(const T& _value) : data(_value) {}
        LazyBody(Function<T()> _eval) : data(_eval) {}

        T& getRef() const {
            if(!std::holds_alternative<T>(data))
            {
                data = std::get<Function<T()>>(data)();
            }
            return std::get<T>(data);
        }

        T& operator*() const {
            return getRef();
        }

        const T* operator->() const {
            return &getRef();
        }

        mutable std::variant<T, Function<T()>> data;
        // mutable optional<T> value;
        // Function<T()> eval;
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

#pragma once
#include "proptest/std/vector.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/any.hpp"


namespace proptest {

namespace util {

template<typename... ARGS, size_t... Is>
tuple<ARGS...> vectorToTupleImpl(const vector<Any>& vec, index_sequence<Is...>) {
    if (vec.size() != sizeof...(ARGS)) {
        throw std::invalid_argument("vector size does not match the number of tuple elements");
    }
    return util::make_tuple(vec[Is].getRef<ARGS>()...);
}

template<typename... ARGS>
tuple<ARGS...> vectorToTuple(const vector<Any>& v) {
    return vectorToTupleImpl<ARGS...>(v, std::index_sequence_for<ARGS...>{});
}

template <typename Callable, typename... ARGS, size_t... Is>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithVectorImpl(const Callable&& callable, const vector<Any>& vec, index_sequence<Is...>) {
    if (vec.size() != sizeof...(ARGS)) {
        throw std::invalid_argument("number of arguments does not match the number of function parameters");
    }
    return callable(vec[Is].getRef<decay_t<ARGS>>()...);
}

template<typename Callable, typename... ARGS>
decltype(auto) invokeWithVector(const Callable&& callable, const vector<Any>& vec) {
    return invokeWithVectorImpl<Callable, ARGS...>(util::forward<const Callable>(callable), vec, std::index_sequence_for<ARGS...>{});
}

} // namespace util


struct TupleOrVectorHolder {
    virtual ~TupleOrVectorHolder() {}

    virtual vector<Any> toAnyVector() const =  0;

    template <typename ...ARGS>
    tuple<ARGS...> toTuple() const {
        return util::vectorToTuple<ARGS...>(this->toAnyVector());
    }
};

template <typename T>
struct VectorHolder : public TupleOrVectorHolder {
    VectorHolder(vector<T>&& vec) : value(util::move(vec)) {}
    VectorHolder(const vector<T>& vec) : value(vec) {}

    virtual vector<Any> toAnyVector() const override {
        vector<Any> anyVector;
        for (const auto& elem : value)
            anyVector.push_back(Any(elem));
        return anyVector;
    };

    vector<T> value;
};

template <typename...ARGS>
struct TupleHolder : public TupleOrVectorHolder {
    using TUP = tuple<ARGS...>;
    static constexpr size_t TUP_SIZE = sizeof...(ARGS);

    TupleHolder(TUP&& tup) : value(util::move(tup)) {}
    TupleHolder(const TUP& tup) : value(tup) {}

    virtual vector<Any> toAnyVector() const override {
        vector<Any> anyVector;

        util::For([&] (auto index_sequence) {
            anyVector.push_back(Any(proptest::get<index_sequence.value>(value)));
        }, make_index_sequence<TUP_SIZE>{});

        return anyVector;
    };

    template <size_t N>
    auto get() const {
        return proptest::get<N>(value);
    }

    TUP value;
};

} // namespace proptest
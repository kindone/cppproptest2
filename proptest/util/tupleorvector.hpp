#pragma once
#include "proptest/std/vector.hpp"
#include "proptest/std/initializer_list.hpp"
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
        throw invalid_argument(__FILE__, __LINE__, "vector size does not match the number of tuple elements");
    }
    return util::make_tuple(vec[Is].getRef<ARGS>()...);
}

template<typename... ARGS>
tuple<ARGS...> vectorToTuple(const vector<Any>& v) {
    return vectorToTupleImpl<ARGS...>(v, std::index_sequence_for<ARGS...>{});
}

template <typename... ARGS>
vector<Any> tupleToAnyVector(const tuple<ARGS...>& tup) {
    vector<Any> anyVector;
    util::For([&] (auto index_sequence) {
        anyVector.push_back(Any(proptest::get<index_sequence.value>(tup)));
    }, make_index_sequence<sizeof...(ARGS)>{});
    return anyVector;
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

struct TupleOrVector {
    template <typename... ARGS>
        requires (sizeof...(ARGS) > 1)
    TupleOrVector(ARGS&&... args) : holder(util::make_shared<TupleHolder<ARGS...>>(util::forward<ARGS>(args)...)) {}

    template <typename T>
    TupleOrVector(vector<T>&& vec) : holder(util::make_shared<VectorHolder<T>>(util::move(vec))) {}

    template <typename T>
    TupleOrVector(const vector<T>& vec) : holder(util::make_shared<VectorHolder<T>>(vec)) {}

    template <typename... ARGS>
    TupleOrVector(tuple<ARGS...>&& tup) : holder(util::make_shared<TupleHolder<ARGS...>>(util::move(tup))) {}

    template <typename... ARGS>
    TupleOrVector(const tuple<ARGS...>& tup) : holder(util::make_shared<TupleHolder<ARGS...>>(tup)) {}

    template <typename... ARGS>
    tuple<ARGS...> toTuple() const {
        return holder->toTuple<ARGS...>();
    }

    template <typename T>
    vector<T> toVector() const {
        vector<T> vec;
        for (const auto& elem : holder->toAnyVector())
            vec.push_back(elem.getRef<T>());
        return vec;
    }

    vector<Any> toAnyVector() const {
        return holder->toAnyVector();
    }

    shared_ptr<TupleOrVectorHolder> holder;
};

} // namespace proptest

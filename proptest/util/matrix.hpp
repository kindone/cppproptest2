#pragma once

#include "proptest/std/tuple.hpp"
#include "proptest/util/tupleorvector.hpp"

namespace proptest {
namespace util {

template <typename...ARGS>
struct Matrix {

};

template <typename...ARGS>
decltype(auto) cartesianProduct(initializer_list<ARGS>&&... lists) {
    using TUP = tuple<ARGS...>;
    constexpr size_t Size = sizeof...(ARGS);

    vector<vector<Any>> vecs(Size);
    auto vecTuple = util::make_tuple(vector<ARGS>(lists)...);
    for(size_t i = 0; i < Size; i++) {
        //vecs.push_back
        util::Map([&] (auto index_sequence) {
            auto vec = vecTuple.template get<index_sequence.value>();
            // convert to Any by copying
            vecs[index_sequence.value].insert(vec.begin(), vec.end());
        }, Size);
    }
}


} // namespace util
} // namespace proptest
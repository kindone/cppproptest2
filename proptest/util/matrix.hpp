#pragma once

#include "proptest/std/tuple.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/function.hpp"

namespace proptest {
namespace util {

template <typename RET, typename...ARGS>
decltype(auto) cartesianProduct(Function<RET(ARGS...)> func, initializer_list<ARGS>&&... lists) {
    using TUP = tuple<ARGS...>;
    constexpr int Size = sizeof...(ARGS);

    // prepare vecs
    vector<vector<Any>> vecs{vector<Any>(lists.begin(), lists.end())...};
    // i per args
    size_t is[Size] = {0};

    // progress rear elements first
    auto progress = [&]() {
        for(int j = Size-1; j >= 0; j--) {
            if(is[j] < vecs[j].size()) {
                for(int k = j+1; k < Size; k++)
                    is[k] = 0;
                is[j]++;
                if(is[j] < vecs[j].size())
                    return true;
            }
        }
        return false;
    };

    bool result = true;
    do {
        vector<Any> outVec;
        outVec.reserve(Size);
        for(size_t j = 0; j < Size; j++) {
            outVec.push_back(vecs[j][is[j]]);
        }
        bool success = Call<Size>(func, [&](auto index_sequence) {
            return outVec[index_sequence.value].template getRef<tuple_element_t<index_sequence.value, TUP>>();
        });

        if(!success)
            result = false;
    } while(progress());

    return result;
}


} // namespace util
} // namespace proptest

#pragma once

#include "proptest/std/tuple.hpp"
#include "proptest/util/tupleorvector.hpp"
#include "proptest/util/anyfunction.hpp"

namespace proptest {
namespace util {

template <typename RET, typename...ARGS>
decltype(auto) cartesianProduct(Function<RET(ARGS...)> func, initializer_list<ARGS>&&... lists) {
    using TUP = tuple<ARGS...>;
    constexpr int Size = sizeof...(ARGS);

    auto vecTuple = util::make_tuple(vector<ARGS>(lists)...);
    // prepare vecs
    vector<vector<Any>> vecs(Size);
    util::For<Size>([&] (auto index_sequence) {
        auto vec = get<index_sequence.value>(vecTuple);
        // convert to Any by copying
        vecs[index_sequence.value].insert(vecs[index_sequence.value].end(), vec.begin(), vec.end());
    });

    // i per args
    size_t is[Size] = {0};
    // progress condition
    /*
    // progress front elements first
    auto progressFirst = [&]() {
        for(size_t j = 0; j < Size; j++) {
            if(is[j] < vecs[j].size()) {
                for(size_t k = 0;k < j;k++)
                    is[k] = 0;
                is[j]++;
                if(is[j] < vecs[j].size())
                    return true;
            }
        }
        return false;
    };
    */

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

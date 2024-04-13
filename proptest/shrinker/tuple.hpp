#pragma once

#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/tupleorvector.hpp"
#include "proptest/std/algorithm.hpp"
#include "proptest/std/map.hpp"

namespace proptest {

//extern template struct Shrinkable<vector<ShrinkableAny>>;

namespace util {
PROPTEST_API Shrinkable<vector<ShrinkableBase>> shrinkTupleUsingVector(Shrinkable<vector<ShrinkableBase>> vectorAnyShr);
} // namespace util

template <typename... ARGS>
PROPTEST_API Shrinkable<tuple<ARGS...>> shrinkTuple(const Shrinkable<tuple<Shrinkable<ARGS>...>>& shrinkable)
{
    constexpr size_t NumArgs = sizeof...(ARGS);
    Shrinkable<vector<ShrinkableBase>> vectorAnyShr = shrinkable.template map<vector<ShrinkableBase>>(+[](const tuple<Shrinkable<ARGS>...>& tuple) {
        vector<ShrinkableBase> anyVector;
        anyVector.reserve(NumArgs);
        util::For<NumArgs>([&] (auto index_sequence) {
            anyVector.push_back(get<index_sequence.value>(tuple));
        });
        return anyVector;
    });

    vectorAnyShr = util::shrinkTupleUsingVector(vectorAnyShr);
    return vectorAnyShr.map<tuple<ARGS...>>([](const vector<ShrinkableBase>& shrAnyVec) {
        vector<Any> anyVec;
        anyVec.reserve(shrAnyVec.size());
        util::transform(shrAnyVec.begin(), shrAnyVec.end(), util::inserter(anyVec, anyVec.begin()), [](const ShrinkableBase& shr) -> Any {
            return shr.getAny();
        });
        return util::vectorToTuple<ARGS...>(anyVec);
    });
}

} // namespace proptest

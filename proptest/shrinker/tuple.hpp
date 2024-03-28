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
PROPTEST_API Shrinkable<vector<ShrinkableAny>> shrinkTupleUsingVector(Shrinkable<vector<ShrinkableAny>> vectorAnyShr);
} // namespace util

template <typename... ARGS>
PROPTEST_API Shrinkable<tuple<ARGS...>> shrinkTuple(const Shrinkable<tuple<Shrinkable<ARGS>...>>& shrinkable)
{
    Shrinkable<vector<ShrinkableAny>> vectorAnyShr = shrinkable.template map<vector<ShrinkableAny>, tuple<Shrinkable<ARGS>...>>(+[](const tuple<Shrinkable<ARGS>...>& tuple) {
        vector<ShrinkableAny> anyVector;
        anyVector.reserve(sizeof...(ARGS));
        util::For([&] (auto index_sequence) {
            anyVector.push_back(ShrinkableAny(get<index_sequence.value>(tuple)));
        }, make_index_sequence<sizeof...(ARGS)>{});
        return anyVector;
    });

    vectorAnyShr = util::shrinkTupleUsingVector(vectorAnyShr);
    return vectorAnyShr.map<tuple<ARGS...>, vector<ShrinkableAny>>([](const vector<ShrinkableAny>& shrAnyVec) {
        vector<Any> anyVec;
        anyVec.reserve(shrAnyVec.size());
        util::transform(shrAnyVec.begin(), shrAnyVec.end(), util::inserter(anyVec, anyVec.begin()), [](const ShrinkableAny& shr) -> Any {
            return shr.getAny();
        });
        return util::vectorToTuple<ARGS...>(anyVec);
    });
}

} // namespace proptest

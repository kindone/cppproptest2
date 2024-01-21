#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/tupleorvector.hpp"

namespace proptest {

extern template struct Shrinkable<vector<ShrinkableAny>>;

namespace util {
Shrinkable<vector<ShrinkableAny>> shrinkTupleUsingVector(Shrinkable<vector<ShrinkableAny>> vectorAnyShr);
} // namespace util

template <typename... ARGS>
Shrinkable<tuple<ARGS...>> shrinkTuple(const Shrinkable<tuple<Shrinkable<ARGS>...>>& shrinkable)
{
    Shrinkable<vector<ShrinkableAny>> vectorAnyShr = shrinkable.map<vector<ShrinkableAny>>(+[](const tuple<Shrinkable<ARGS>...>& tuple) {
        vector<ShrinkableAny> anyVector;
        util::For([&] (auto index_sequence) {
            anyVector.push_back(ShrinkableAny(get<index_sequence.value>(tuple)));
        }, make_index_sequence<sizeof...(ARGS)>{});
        return anyVector;
    });

    vectorAnyShr = util::shrinkTupleUsingVector(vectorAnyShr);
    return vectorAnyShr.map<tuple<ARGS...>>([](const vector<ShrinkableAny>& shrAnyVec) {
        vector<Any> anyVec;
        util::transform(shrAnyVec.begin(), shrAnyVec.end(), util::inserter(anyVec, anyVec.begin()), [](const ShrinkableAny& shr) -> Any {
            return shr.getAny();
        });
        return util::vectorToTuple<ARGS...>(anyVec);
    });
}

} // namespace proptest

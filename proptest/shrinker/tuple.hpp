#pragma once

#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/tupleorvector.hpp"
#include "proptest/std/algorithm.hpp"
#include "proptest/std/map.hpp"
#include "proptest/util/typelist.hpp"

namespace proptest {

namespace util {
PROPTEST_API Shrinkable<vector<ShrinkableBase>> shrinkTupleUsingVector(Shrinkable<vector<ShrinkableBase>> vectorAnyShr);
} // namespace util

template <typename... ARGS>
PROPTEST_API Shrinkable<tuple<ARGS...>> shrinkTuple(const Shrinkable<vector<ShrinkableBase>>& shrinkable)
{
    return util::shrinkTupleUsingVector(shrinkable).map<tuple<ARGS...>>([](const vector<ShrinkableBase>& shrAnyVec) {
        vector<Any> anyVec;
        anyVec.reserve(shrAnyVec.size());
        util::transform(shrAnyVec.begin(), shrAnyVec.end(), util::inserter(anyVec, anyVec.begin()), [](const ShrinkableBase& shr) -> Any {
            return shr.getAny();
        });
        return util::vectorToTuple<ARGS...>(anyVec);
    });
}

template <typename... ARGS>
PROPTEST_API Shrinkable<tuple<ARGS...>> shrinkTuple(util::TypeList<ARGS...>, const Shrinkable<vector<ShrinkableBase>>& shrinkable)
{
    return shrinkTuple<ARGS...>(shrinkable);
}


} // namespace proptest

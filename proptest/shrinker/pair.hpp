#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

namespace util {

PROPTEST_API ShrinkableBase shrinkPairImpl(const ShrinkableBase& shrinkable);

} // namespace util


template <typename ARG1, typename ARG2>
Shrinkable<pair<ARG1,ARG2>> shrinkPair(const Shrinkable<ARG1>& firstShr, const Shrinkable<ARG2>& secondShr)
{
    auto pairShr = ShrinkableBase(util::make_pair(ShrinkableBase(firstShr), ShrinkableBase(secondShr)));
    return util::shrinkPairImpl(pairShr).map(+[](const Any& anyPair) -> Any {
        const auto& shrPair = anyPair.getRef<pair<ShrinkableBase, ShrinkableBase>>();
        return util::make_pair(shrPair.first.getRef<ARG1>(), shrPair.second.getRef<ARG2>());
    });
}

} // namespace proptest

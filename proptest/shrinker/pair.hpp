#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

namespace util {

class PROPTEST_API PairShrinker {
    using ARG1 = Any;
    using ARG2 = Any;
    using out_pair_t = pair<ARG1, ARG2>;
    using pair_t = pair<Shrinkable<ARG1>, Shrinkable<ARG2>>;
    using shrinkable_t = Shrinkable<pair_t>;
    using stream_t = Stream<shrinkable_t>;

private:
    static Function<stream_t(const shrinkable_t&)> shrinkFirst();

    static Function<stream_t(const shrinkable_t&)> shrinkSecond();

public:
    static Shrinkable<out_pair_t> shrink(const shrinkable_t& shrinkable);
};

} // namespace util

#ifndef PROPTEST_UNTYPED_SHRINKABLE
namespace typed {
extern template struct PROPTEST_API Shrinkable<pair<Any, Any>>;
}
#endif

#ifndef PROPTEST_UNTYPED_STREAM
namespace typed {
extern template struct PROPTEST_API Stream<Shrinkable<pair<Any, Any>>>;
}
#endif

template <typename ARG1, typename ARG2>
Shrinkable<pair<ARG1, ARG2>> shrinkPair(
    const Shrinkable<ARG1>& firstShr, const Shrinkable<ARG2>& secondShr)
{
    // type-erase ARG1 and ARG2 with ShrinkableAny
    ShrinkableAny firstShrAny(firstShr), secondShrAny(secondShr);
    auto elemPair = util::make_pair(firstShrAny, secondShrAny);
    auto shrinkable = make_shrinkable<decltype(elemPair)>(elemPair);
    return util::PairShrinker::shrink(shrinkable).map<pair<ARG1, ARG2>, pair<Any, Any>>(+[](const pair<Any, Any>& anyPair) {
        return util::make_pair(anyPair.first.getRef<ARG1>(), anyPair.second.getRef<ARG2>());
    });
}

} // namespace proptest

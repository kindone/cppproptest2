#include "proptest/shrinker/pair.hpp"

namespace proptest {

namespace util {

Function<PairShrinker::stream_t(const PairShrinker::shrinkable_t&)> PairShrinker::shrinkFirst()
{
    using e_shrinkable_t = Shrinkable<ARG1>;
    using element_t = typename e_shrinkable_t::type;

    return +[](const shrinkable_t& parent) -> stream_t {
        const e_shrinkable_t& elem = parent.getRef().first;
        shrinkable_t pairWithElems = elem.template flatMap<pair_t>([parent](const element_t& val) {
            auto copy = parent.get();
            copy.first = make_shrinkable<element_t>(val);
            return make_shrinkable<pair_t>(copy);
        });
        return pairWithElems.getShrinks();
    };
}

Function<PairShrinker::stream_t(const PairShrinker::shrinkable_t&)> PairShrinker::shrinkSecond()
{
    using e_shrinkable_t = Shrinkable<ARG2>;
    using element_t = typename e_shrinkable_t::type;

    return +[](const shrinkable_t& parent) -> stream_t {
        const e_shrinkable_t& elem = parent.getRef().second;
        shrinkable_t pairWithElems = elem.template flatMap<pair_t>([parent](const element_t& val) {
            auto copy = parent.get();
            copy.second = make_shrinkable<element_t>(val);
            return make_shrinkable<pair_t>(copy);
        });
        return pairWithElems.getShrinks();
    };
}

Shrinkable<PairShrinker::out_pair_t> PairShrinker::shrink(const PairShrinker::shrinkable_t& shrinkable)
{
    auto concatenated = shrinkable.concat(shrinkFirst()).concat(shrinkSecond());
    return concatenated.template flatMap<out_pair_t>(+[](const pair_t& pair) {
        return make_shrinkable<out_pair_t>(util::make_pair(pair.first.get(), pair.second.get()));
    });
}

} // namespace util

template struct Shrinkable<pair<Any, Any>>;
#ifndef PROPTEST_UNTYPED_STREAM
template struct Stream<Shrinkable<pair<Any, Any>>>;
#endif

} // namespace proptest
#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

namespace util {

template <typename ARG1, typename ARG2>
class PairShrinker {
    using out_pair_t = pair<ARG1, ARG2>;
    using pair_t = pair<Shrinkable<ARG1>, Shrinkable<ARG2>>;
    using shrinkable_t = Shrinkable<pair_t>;
    using stream_t = TypedStream<shrinkable_t>;

private:
    static Function<stream_t(const shrinkable_t&)> shrinkFirst()
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

    static Function<stream_t(const shrinkable_t&)> shrinkSecond()
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

public:
    static Shrinkable<out_pair_t> shrink(const shrinkable_t& shrinkable)
    {
        auto concatenated = shrinkable.concat(shrinkFirst()).concat(shrinkSecond());
        return concatenated.template flatMap<out_pair_t>(+[](const pair_t& pair) {
            return make_shrinkable<out_pair_t>(util::make_pair(pair.first.get(), pair.second.get()));
        });
    }
};

} // namespace util

template <typename ARG1, typename ARG2>
Shrinkable<pair<ARG1, ARG2>> shrinkPair(
    const Shrinkable<ARG1>& firstShr, const Shrinkable<ARG2>& secondShr)
{
    auto elemPair = util::make_pair(firstShr, secondShr);
    auto shrinkable = make_shrinkable<decltype(elemPair)>(elemPair);
    return util::PairShrinker<ARG1, ARG2>::shrink(shrinkable);
}

} // namespace proptest

#include "proptest/shrinker/pair.hpp"

namespace proptest {

namespace util {

Function<ShrinkableBase::StreamType(const ShrinkableBase&)> shrinkFirst()
{
    return +[](const ShrinkableBase& parent) -> ShrinkableBase::StreamType {
        const ShrinkableBase& elem = parent.getRef<pair<ShrinkableBase, ShrinkableBase>>().first;
        ShrinkableBase pairWithElems = elem.flatMap([parent](const Any& val) -> ShrinkableBase {
            const auto& parentPair = parent.getRef<pair<ShrinkableBase, ShrinkableBase>>();
            return make_shrinkable<pair<ShrinkableBase,ShrinkableBase>>(ShrinkableBase(val), parentPair.second);
        });
        return pairWithElems.getShrinks();
    };
}

Function<ShrinkableBase::StreamType(const ShrinkableBase&)> shrinkSecond()
{
    return +[](const ShrinkableBase& parent) -> ShrinkableBase::StreamType {
        const auto& shrPair = parent.getRef<pair<ShrinkableBase, ShrinkableBase>>();
        const ShrinkableBase& elem = shrPair.second;
        ShrinkableBase pairWithElems = elem.flatMap([parent](const Any& val) {
            const auto& parentPair = parent.getRef<pair<ShrinkableBase, ShrinkableBase>>();
            return make_shrinkable<pair<ShrinkableBase,ShrinkableBase>>(parentPair.first, ShrinkableBase(val));
        });
        return pairWithElems.getShrinks();
    };
}

ShrinkableBase shrinkPairImpl(const ShrinkableBase& shrinkable)
{
    return shrinkable.concat(shrinkFirst()).concat(shrinkSecond());
}

} // namespace util

} // namespace proptest

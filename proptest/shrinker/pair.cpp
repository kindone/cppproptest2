#include "proptest/shrinker/pair.hpp"

namespace proptest {

namespace util {

Function<ShrinkableBase::StreamType(const ShrinkableBase&)> shrinkFirst()
{
    return +[](const ShrinkableBase& parent) -> ShrinkableBase::StreamType {
        const ShrinkableBase& elem = parent.getRef<pair<ShrinkableBase, ShrinkableBase>>().first;
        ShrinkableBase pairWithElems = elem.flatMap([parent](const Any& val) -> ShrinkableBase {
            auto shrCopy = make_shrinkable<pair<ShrinkableBase,ShrinkableBase>>(parent.getRef<pair<ShrinkableBase, ShrinkableBase>>());
            shrCopy.getMutableRef().first = ShrinkableBase(val);
            return shrCopy;
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
            auto shrCopy = make_shrinkable<pair<ShrinkableBase,ShrinkableBase>>(parent.getRef<pair<ShrinkableBase, ShrinkableBase>>());
            shrCopy.getMutableRef().second = ShrinkableBase(val);
            return shrCopy;
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

#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/std/map.hpp"

namespace proptest {


template <typename Key, typename Value>
Shrinkable<map<Key, Value>> shrinkMap(const Shrinkable<vector<ShrinkableBase>>& inPairShrVecShr, size_t minSize) {
    Shrinkable<vector<ShrinkableBase>> pairShrVecShr = shrinkAnyVector(inPairShrVecShr, minSize, /*elementwise*/true, /*membershipwise*/true);
    return pairShrVecShr.template flatMap<map<Key, Value>>([](const vector<ShrinkableBase>& anyVec) {
        auto mapPtr = util::make_shared<map<Key, Value>>();
        for(const auto& any : anyVec) {
            const auto& thePair = any.getRef<pair<Key,Value>>();
            mapPtr->insert(thePair);
        }
        return Shrinkable<map<Key, Value>>(util::make_any<map<Key, Value>>(mapPtr));
    });
}

template <typename Key, typename Value>
Shrinkable<map<Key, Value>> shrinkMap(const shared_ptr<vector<Shrinkable<Key>>> keyShrVec, const shared_ptr<vector<Shrinkable<Value>>>& valShrVec, size_t minSize) {
    // 1. for each valShr in valShrVec, map it to pairShr with keyShr (assuming keyShr has no shrinks) and put into a new vector, get vector<pair<Shrinkable<Key>, Shrinkable<Value>>>
    // 2. membershipwise shrink this vector
    // 3. convert this to Shrinkable<map<Key, Value>>
    auto pairShrVecShr = make_shrinkable<vector<ShrinkableBase>>();
    vector<ShrinkableBase>& pairShrVec = pairShrVecShr.getMutableRef();
    pairShrVec.reserve(valShrVec->size());
    for(size_t i = 0; i < valShrVec->size(); i++) {
        ShrinkableBase pairShr = valShrVec->at(i).template map<pair<Key, Value>>([keyShrVec, i](const Value& val) -> pair<Key, Value> {
            return util::make_pair(keyShrVec->at(i).getRef(), val);
        });
        pairShrVec.push_back(pairShr);
    }

    return shrinkMap<Key,Value>(pairShrVecShr, minSize);
}

} // namespace proptes

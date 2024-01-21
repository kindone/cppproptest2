#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

template <typename Key, typename Value>
Shrinkable<map<Key, Value>> shrinkMap(const shared_ptr<vector<Shrinkable<Key>>> keyShrVec, const shared_ptr<vector<Shrinkable<Value>>>& valShrVec, size_t minSize) {
    // 1. for each valShr in valShrVec, map it to pairShr with keyShr (assuming keyShr has no shrinks) and put into a new vector, get vector<pair<Shrinkable<Key>, Shrinkable<Value>>>
    // 2. membershipwise shrink this vector
    // 3. convert this to Shrinkable<map<Key, Value>>

    vector<Shrinkable<pair<Key, Value>>> pairShrVec;
    for(size_t i = 0; i < valShrVec->size(); i++) {
        Shrinkable<pair<Key, Value>> pairShr = valShrVec->at(i).template map<pair<Key, Value>>([keyShrVec, i](const Value& val) -> pair<Key, Value> {
            return util::make_pair(keyShrVec->at(i).getRef(), val);
        });
        pairShrVec.push_back(pairShr);
    }

    Shrinkable<vector<pair<Key,Value>>> pairShrVecShr = shrinkContainer<vector, pair<Key, Value>>(make_shrinkable<decltype(pairShrVec)>(pairShrVec), minSize, /*elementwise*/true, /*membershipwise*/true);
    return pairShrVecShr.template map<map<Key, Value>>([](const vector<pair<Key, Value>>& vec) -> map<Key, Value> {
        map<Key, Value> m;
        for(auto& pair : vec)
            m.insert(pair);
        return m;
    });
}

} // namespace proptes

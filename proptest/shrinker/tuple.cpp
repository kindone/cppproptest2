#include "proptest/shrinker/tuple.hpp"

namespace proptest {
namespace util {

Shrinkable<vector<ShrinkableAny>> shrinkTupleUsingVector(Shrinkable<vector<ShrinkableAny>> vectorAnyShr) {
    const size_t Size = vectorAnyShr.getRef().size();
    for(size_t N = 0; N < Size; N++) {
        vectorAnyShr = vectorAnyShr.concat([N](const Shrinkable<vector<ShrinkableAny>>& parent) -> Shrinkable<vector<ShrinkableAny>>::StreamType {
            const ShrinkableAny& elemShr = parent.getRef()[N];
            // need a mutable clone
            const auto& parentVec = parent.getRef();
            shared_ptr<vector<ShrinkableAny>> parentVecCopy = util::make_shared<vector<ShrinkableAny>>();
            util::transform(parentVec.begin(), parentVec.end(), util::inserter(*parentVecCopy, parentVecCopy->begin()), +[](const ShrinkableAny& shr) -> ShrinkableAny {
                return shr.clone();
            });

            // rebuild full vector from an element
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrunk from 2 is also transformed to [x,x,x,1]
            // ShrinkableAny -> Shrinkable<vector<ShrinkableAny>>
            Shrinkable<vector<ShrinkableAny>> vecWithElems = elemShr.template map<vector<ShrinkableAny>>([N,parentVecCopy](const Any& val) -> vector<ShrinkableAny> {
                // create a copy
                (*parentVecCopy)[N] = make_shrinkable<Any>(val); // replace parent copy with val at tuple position N
                return *parentVecCopy;
            });
            return vecWithElems.getShrinks();
        });
    }

    return vectorAnyShr;
}

} // namespace util

#ifndef PROPTEST_UNTYPED_SHRINKABLE
namespace typed {
template struct Shrinkable<vector<ShrinkableAny>>;
}
#endif // PROPTEST_UNTYPED_SHRINKABLE

} // namespace proptest
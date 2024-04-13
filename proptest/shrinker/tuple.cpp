#include "proptest/shrinker/tuple.hpp"

namespace proptest {
namespace util {

Shrinkable<vector<ShrinkableBase>> shrinkTupleUsingVector(Shrinkable<vector<ShrinkableBase>> vectorAnyShr) {
    const size_t Size = vectorAnyShr.getRef().size();
    for(size_t N = 0; N < Size; N++) {
        vectorAnyShr = vectorAnyShr.concat([N](const Shrinkable<vector<ShrinkableBase>>& parent) -> Shrinkable<vector<ShrinkableBase>>::StreamType {
            const ShrinkableBase& elemShr = parent.getRef()[N];
            // need a mutable clone
            const auto& parentVec = parent.getRef();
            shared_ptr<vector<ShrinkableBase>> parentVecCopy = util::make_shared<vector<ShrinkableBase>>();
            util::transform(parentVec.begin(), parentVec.end(), util::inserter(*parentVecCopy, parentVecCopy->begin()), +[](const ShrinkableBase& shr) -> ShrinkableBase {
                return shr.clone();
            });

            // rebuild full vector from an element
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrunk from 2 is also transformed to [x,x,x,1]
            // ShrinkableBase -> Shrinkable<vector<ShrinkableBase>>
            Shrinkable<vector<ShrinkableBase>> vecWithElems = elemShr.map([N,parentVecCopy](const Any& val) -> Any {
                // create a copy
                (*parentVecCopy)[N] = ShrinkableBase(val); // replace parent copy with val at tuple position N
                return *parentVecCopy;
            });
            return vecWithElems.getShrinks();
        });
    }

    return vectorAnyShr;
}

} // namespace util

} // namespace proptest

#include "proptest/combinator/aggregate.hpp"


namespace proptest {

namespace util {

Generator<vector<Any>> aggregateImplAny(GenFunction<Any> gen1, Function<GenFunction<Any>(Any&)> gen2gen, size_t minSize,
                                    size_t maxSize)
{
    return interval<uint64_t>(minSize, maxSize).flatMap<vector<Any>>([gen1, gen2gen, minSize](const uint64_t& size) {
        if (size == 0)
            return Generator<vector<Any>>([](Random&) { return make_shrinkable<vector<Any>>(); });
        return Generator<vector<Any>>([gen1, gen2gen, size, minSize](Random& rand) {
            Shrinkable<Any> shr = gen1(rand);
            auto shrVec = make_shrinkable<vector<Shrinkable<Any>>>();
            auto& vec = shrVec.getMutableRef();
            vec.reserve(size);
            vec.push_back(shr);
            for (size_t i = 1; i < size; i++) {
                shr = gen2gen(shr.getAny())(rand);
                vec.push_back(shr);
            }
            return shrinkListLikeLength<vector, Any>(shrVec, minSize)
                .andThen([](const Shrinkable<vector<Shrinkable<Any>>>::StreamElementType& parent) {
                    const vector<Shrinkable<Any>>& shrVec_ = Shrinkable<vector<Shrinkable<Any>>>(parent).getRef();
                    if (shrVec_.size() == 0)
                        return Shrinkable<vector<Shrinkable<Any>>>::StreamType::empty();
                    const Shrinkable<Any>& lastElemShr = shrVec_.back();
                    Shrinkable<Any>::StreamType elemShrinks = lastElemShr.getShrinks();
                    if (elemShrinks.isEmpty())
                        return Shrinkable<vector<Shrinkable<Any>>>::StreamType::empty();
                    return elemShrinks.template transform<Shrinkable<vector<Shrinkable<Any>>>::StreamElementType, Shrinkable<Any>::StreamElementType>(
                        [copy = shrVec_](const Shrinkable<Any>::StreamElementType& elem) mutable -> Shrinkable<vector<Shrinkable<Any>>>::StreamElementType {
                            copy[copy.size() - 1] = Shrinkable<Any>(elem);
                            return make_shrinkable<vector<Shrinkable<Any>>>(copy);
                        });
                })
                .template map<vector<Any>>([](const vector<Shrinkable<Any>>& shrVec) {
                    vector<Any> valVec;
                    valVec.reserve(shrVec.size());
                    util::transform(shrVec.begin(), shrVec.end(), util::back_inserter(valVec),
                                    [](const Shrinkable<Any>& shr) { return shr.getAny(); });
                    return valVec;
                });
        });
    });
}

} // namespace util

} // namespace proptest


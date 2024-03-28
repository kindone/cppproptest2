#include "proptest/combinator/accumulate.hpp"


namespace proptest {

namespace util {

Generator<vector<Any>> accumulateImplAny(GenFunction<Any> gen1, Function<GenFunction<Any>(const Any&)> gen2gen, size_t minSize,
                                    size_t maxSize)
{
    return interval<uint64_t>(minSize, maxSize).flatMap<vector<Any>>([gen1, gen2gen, minSize](const uint64_t& size) {
        if (size == 0)
            return Generator<vector<Any>>([](Random&) { return make_shrinkable<vector<Any>>(); });
        return Generator<vector<Any>>([gen1, gen2gen, size, minSize](Random& rand) {
            Shrinkable<Any> shr = gen1(rand);
            auto shrVec = make_shrinkable<vector<Shrinkable<Any>>>();
            auto& vec = shrVec.getMutableRef<vector<Shrinkable<Any>>>();
            vec.reserve(size);
            vec.push_back(shr);
            for (size_t i = 1; i < size; i++) {
                shr = gen2gen(shr.getAny())(rand);
                vec.push_back(shr);
            }
            return shrinkListLikeLength<vector, Any>(shrVec, minSize)
                .andThen([](const Shrinkable<vector<Shrinkable<Any>>>& parent) {
                    const vector<Shrinkable<Any>>& shrVec_ = parent.getRef<vector<Shrinkable<Any>>>();
                    if (shrVec_.size() == 0)
                        return Stream<Shrinkable<vector<Shrinkable<Any>>>>::empty();
                    const Shrinkable<Any>& lastElemShr = shrVec_.back();
                    Stream<Shrinkable<Any>> elemShrinks = lastElemShr.getShrinks();
                    if (elemShrinks.isEmpty())
                        return Stream<Shrinkable<vector<Shrinkable<Any>>>>::empty();
                    return elemShrinks.template transform<Shrinkable<vector<Shrinkable<Any>>>,Shrinkable<Any>>(
                        [copy = shrVec_](const Shrinkable<Any>& elem) mutable -> Shrinkable<vector<Shrinkable<Any>>> {
                            copy[copy.size() - 1] = Shrinkable<Any>(elem);
                            return make_shrinkable<vector<Shrinkable<Any>>>(copy);
                        });
                })
                .template map<vector<Any>, vector<Shrinkable<Any>>>([](const vector<Shrinkable<Any>>& shrVec) {
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


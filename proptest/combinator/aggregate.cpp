#include "proptest/combinator/aggregate.hpp"


namespace proptest {

namespace util {

GeneratorCommon aggregateImpl(Function1 gen1, Function1 gen2gen, size_t minSize, size_t maxSize)
{
    auto intervalGen = interval<uint64_t>(minSize, maxSize);

    auto deriveAggregate = [gen1, gen2gen, minSize](const uint64_t& size) {
        if (size == 0)
            return Function1([](Random&) { return make_shrinkable<vector<ShrinkableBase>>(); });
        return Function1([gen1, gen2gen, size, minSize](Random& rand) {
            ShrinkableBase shr = gen1(util::make_any<Random&>(rand)).getRef<ShrinkableBase>(true);
            auto shrVec = make_shrinkable<vector<ShrinkableBase>>();
            auto& vec = shrVec.getMutableRef();
            vec.reserve(size);
            vec.push_back(shr);
            for (size_t i = 1; i < size; i++) {
                shr = gen2gen(shr.getAny()).getRef<Function1>()(util::make_any<Random&>(rand)).getRef<ShrinkableBase>(true);
                vec.push_back(shr);
            }
            return shrinkVectorLength(shrVec, minSize) // -> Shrinkable<vector<ShrinkableBase>>
                .andThen([](const ShrinkableBase& parent) {
                    const vector<ShrinkableBase>& shrVec_ = parent.getRef<vector<ShrinkableBase>>();
                    if (shrVec_.size() == 0)
                        return ShrinkableBase::StreamType::empty();
                    const ShrinkableBase& lastElemShr = shrVec_.back();
                    ShrinkableBase::StreamType elemShrinks = lastElemShr.getShrinks();
                    if (elemShrinks.isEmpty())
                        return ShrinkableBase::StreamType::empty();
                    return elemShrinks.template transform<ShrinkableBase, ShrinkableBase>(
                        [copy = shrVec_](const ShrinkableBase& elem) mutable -> ShrinkableBase {
                            copy[copy.size() - 1] = elem;
                            return make_shrinkable<vector<ShrinkableBase>>(copy);
                        });
                });
        });
    };

    return deriveImpl(intervalGen, [deriveAggregate](const Any& sizeAny) -> Function1 { return deriveAggregate(sizeAny.getRef<uint64_t>()); });
}

} // namespace util

} // namespace proptest


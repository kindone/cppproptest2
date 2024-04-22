#include "proptest/combinator/dependency.hpp"

namespace proptest {

namespace util {

GeneratorCommon dependencyImpl(Function1<ShrinkableBase> gen1, Function1<Function1<ShrinkableBase>> gen2gen)
{
    return GeneratorCommon([gen1, gen2gen](Random& rand) -> ShrinkableBase {
        // generate T
        ShrinkableBase shrinkableT = gen1.callDirect(rand);
        using Intermediate = pair<Any, ShrinkableBase>;

        // shrink strategy 1: expand Shrinkable<T>
        ShrinkableBase intermediate =
            shrinkableT.flatMap([&rand, gen2gen](const Any& t) mutable -> ShrinkableBase {
                // generate U
                auto gen2 = gen2gen(t);
                ShrinkableBase shrinkableU = gen2.callDirect(rand);
                return make_shrinkable<Intermediate>(t, shrinkableU);
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const ShrinkableBase& interShr) -> ShrinkableBase::StreamType {
                // assume interShr has no shrinks
                const Intermediate& interpair = interShr.get<Intermediate>();
                const ShrinkableBase& shrinkableU = interpair.second;
                ShrinkableBase newShrinkableU =
                    shrinkableU.flatMap([first = interShr.get<Intermediate>().first](const Any& u) mutable -> ShrinkableBase {
                        return make_shrinkable<Intermediate>(first, ShrinkableBase(u));
                    });
                return newShrinkableU.getShrinks();
            });

        // to be reformated to pair<T, Shrinkable<U>> to pair<T, U>
        return intermediate;
    });
}

} // namespace util

} // namespace proptest

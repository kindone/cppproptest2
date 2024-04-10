#include "proptest/combinator/derive.hpp"
#include "proptest/Generator.hpp"

namespace proptest {

namespace util {

GeneratorCommon deriveImpl(Function1 gen1, Function1 gen2gen)
{
    Function1 genU = [gen1, gen2gen](Random& rand) {
        // generate T
        ShrinkableBase shrinkableT = gen1(util::make_any<Random&>(rand)).getRef<ShrinkableBase>(true);
        using Intermediate = pair<Any, ShrinkableBase>;
        // shrink strategy 1: expand Shrinkable<T>
        ShrinkableBase intermediate =
            shrinkableT.flatMap([&rand, gen2gen](const Any& t) mutable {
                // generate U
                Function1 gen2 = gen2gen(t).getRef<Function1>();
                ShrinkableBase shrinkableU = gen2(util::make_any<Random&>(rand)).getRef<ShrinkableBase>(true);
                return ShrinkableBase(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const ShrinkableBase& interShr) -> ShrinkableBase::StreamType {
                // assume interShr has no shrinks
                const ShrinkableBase& shrinkableU = interShr.getRef<Intermediate>().second;
                ShrinkableBase newShrinkableU =
                    shrinkableU.flatMap([interShr](const Any& u) mutable {
                        return ShrinkableBase(util::make_pair(interShr.getRef<Intermediate>().first, ShrinkableBase(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<T, Shrinkable<U>> to U
        return intermediate.flatMap(
            +[](const Intermediate& interpair) -> ShrinkableBase { return interpair.second; });
    };

    return genU;
}

} // namespace util

} // namespace proptest

#include "proptest/combinator/dependency.hpp"

namespace proptest {

GeneratorCommon dependency(Function1 gen1, Function1 gen2gen)
{
    return GeneratorCommon([gen1, gen2gen](Random& rand) -> ShrinkableBase {
        // generate T
        ShrinkableBase shrinkableT = gen1(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>();
        using Intermediate = pair<Any, ShrinkableBase>;

        // shrink strategy 1: expand Shrinkable<T>
        ShrinkableBase intermediate =
            shrinkableT.flatMap([&rand, gen2gen](const Any& t) mutable {
                // generate U
                auto gen2 = gen2gen(t).template getRef<Function1>();
                ShrinkableBase shrinkableU = gen2(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>();
                return ShrinkableBase(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const ShrinkableBase& interShr) -> ShrinkableBase::StreamType {
                // assume interShr has no shrinks
                const Intermediate& interpair = interShr.get<Intermediate>();
                const ShrinkableBase& shrinkableU = interpair.second;
                ShrinkableBase newShrinkableU =
                    shrinkableU.flatMap([interShr](const Any& u) mutable {
                        return ShrinkableBase(
                            util::make_pair(interShr.get<Intermediate>().first, ShrinkableBase(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<T, Shrinkable<U>> to pair<T, U>
        return intermediate.flatMap(+[](const Intermediate& interpair) -> ShrinkableBase {
            return ShrinkableBase(util::make_pair(interpair.first, interpair.second.getAny()));
        });
    });
}

} // namespace proptest

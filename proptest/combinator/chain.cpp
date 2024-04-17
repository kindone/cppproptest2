#include "proptest/combinator/chain.hpp"
#include "proptest/util/tupleorvector.hpp"

namespace proptest {
namespace util {

GeneratorCommon chainImpl1(Function1 gen1, Function1 gen2gen)
{
    return Function1([gen1, gen2gen](Random& rand) -> ShrinkableBase {
        // generate T
        ShrinkableBase shrinkableTs = gen1.callDirect(rand).template getRef<ShrinkableBase>(true);
        using Intermediate = pair<Any, ShrinkableBase>;

        // shrink strategy 1: expand Shrinkable<T>
        ShrinkableBase intermediate =
            shrinkableTs.flatMap([&rand, gen2gen](const Any& t) mutable {
                // generate U
                auto gen2 = gen2gen(t).template getRef<Function1>();
                ShrinkableBase shrinkableU = gen2.callDirect(rand).template getRef<ShrinkableBase>(true);
                return ShrinkableBase(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Any& interShrAny) mutable -> ShrinkableBase::StreamType {
                const ShrinkableBase& interShr = interShrAny.template getRef<ShrinkableBase>(true);
                // assume interShr has no shrinks
                const Intermediate& interpair = interShr.template getRef<Intermediate>();
                const ShrinkableBase& shrinkableU = interpair.second;
                ShrinkableBase newShrinkableU =
                    shrinkableU.flatMap([interShr](const Any& u) mutable {
                        const Any& t = interShr.getRef<Intermediate>().first;
                        return ShrinkableBase(util::make_pair(t, ShrinkableBase(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<T, Shrinkable<U>> to Chain<T, U>
        return intermediate;
    });
}

GeneratorCommon chainImplN(Function1 gen1, Function1 gen2gen)
{
    return Function1([gen1, gen2gen](Random& rand) -> ShrinkableBase {
        // generate T
        ShrinkableBase shrinkableTs = gen1.callDirect(rand).getRef<ShrinkableBase>(true);
        using Intermediate = pair<Any, ShrinkableBase>;

        // shrink strategy 1: expand Shrinkable<tuple<Ts...>>
        ShrinkableBase intermediate =
            shrinkableTs.flatMap(
                [&rand, gen2gen](const Any& ts) {
                    // generate U
                    auto gen2 = gen2gen(ts).getRef<Function1>();
                    ShrinkableBase shrinkableU = gen2.callDirect(rand).getRef<ShrinkableBase>(true);
                    return ShrinkableBase(util::make_pair(ts, shrinkableU));
                });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Any& interShrAny) -> ShrinkableBase::StreamType {
                ShrinkableBase interShr = interShrAny.getRef<ShrinkableBase>(true);
                // assume interShr has no shrinks
                const ShrinkableBase& shrinkableU = interShr.template getRef<Intermediate>().second;
                ShrinkableBase newShrinkableU =
                    shrinkableU.flatMap([interShr](const Any& u) mutable {
                        const Any& t = interShr.getRef<Intermediate>().first;
                        return ShrinkableBase(util::make_pair(t, ShrinkableBase(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<Chain<T0, T1, Ts...>, Shrinkable<U>> to Chain<T0, T1, Ts..., U>
        return intermediate;
    });
}



} // namespace util
} // namespace proptest

#include "proptest/combinator/chain.hpp"
#include "proptest/util/tupleorvector.hpp"

namespace proptest {
namespace util {

GeneratorCommon chainImpl(Function1 gen1, Function1 gen2gen)
{
    return Function1([gen1, gen2gen](Random& rand) -> ShrinkableBase {
        // generate T
        ShrinkableBase shrinkableTs = gen1(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>();
        using Intermediate = pair<Any, ShrinkableBase>;

        // shrink strategy 1: expand Shrinkable<T>
        ShrinkableBase intermediate =
            shrinkableTs.flatMap([&rand, gen2gen](const Any& t) mutable {
                // generate U
                auto gen2 = gen2gen(t).template getRef<Function1>();
                ShrinkableBase shrinkableU = gen2(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>();
                return ShrinkableBase(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const ShrinkableBase& interShr) mutable -> ShrinkableBase::StreamType {
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
        return intermediate.flatMap(
            +[](const Any& interpairAny) -> Shrinkable<tuple<T, U>> {
                const Intermediate& interpair = interpairAny.getRef<Intermediate>();
                const Any& t = interpair.first;
                return ShrinkableBase(
                    tuple_cat(tuple<T>(t), util::make_tuple(interpair.second.getAny())));
            });
    });
}

GeneratorCommon chainImpl(Function1 gen1, Function1 gen2gen)
{
    using ChainType = Chain<T0, T1, Ts...>;
    using NextChainType = Chain<T0, T1, Ts..., U>;

    auto genTuple = [gen1, gen2gen](Random& rand) -> ShrinkableBase {
        // generate T
        ShrinkableBase shrinkableTs = gen1(util::make_any<Random&>(rand)).getRef<ShrinkableBase>();
        using Intermediate = pair<ChainType, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<tuple<Ts...>>
        Shrinkable<pair<ChainType, Shrinkable<U>>> intermediate =
            shrinkableTs.template flatMap<Intermediate>(
                [&rand, gen2gen](const ChainType& ts) {
                    // generate U
                    auto gen2 = gen2gen(ts);
                    Shrinkable<U> shrinkableU = gen2(rand);
                    return make_shrinkable<Intermediate>(util::make_pair(ts, shrinkableU));
                });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Shrinkable<Intermediate>::StreamType {
                // assume interShr has no shrinks
                const Shrinkable<U>& shrinkableU = interShr.getRef().second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate>([interShr](const U& u) mutable {
                        return make_shrinkable<Intermediate>(
                            util::make_pair(interShr.getRef().first, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<Chain<T0, T1, Ts...>, Shrinkable<U>> to Chain<T0, T1, Ts..., U>
        return intermediate.template flatMap<TupleOrVector>(
            +[](const Intermediate& interpair) -> Shrinkable<NextChainType> {
                const ChainType& ts = interpair.first;
                return make_shrinkable<NextChainType>(tuple_cat(ts, tuple<U>(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}



} // namespace util
} // namespace proptest

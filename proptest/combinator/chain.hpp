#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/std/tuple.hpp"

/**
 * @file chain.hpp
 * @brief Generator combinator for chaining two generators to generate a tuple of values, where the second generator
 * depends on generated value from the first generator
 */

namespace proptest {

template <class... Ts>
using Chain = tuple<Ts...>;

namespace util {

template <typename U, typename T>
Generator<Chain<T, U>> chainImpl(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
{
    return generator([gen1, gen2gen](Random& rand) -> Shrinkable<Chain<T, U>> {
        // generate T
        Shrinkable<T> shrinkableTs = gen1(rand);
        using Intermediate = pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<pair<T, Shrinkable<U>>> intermediate =
            shrinkableTs.template flatMap<pair<T, Shrinkable<U>>>([&rand, gen2gen](const T& t) mutable {
                // generate U
                auto gen2 = gen2gen(t);
                Shrinkable<U> shrinkableU = gen2(rand);
                return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) mutable -> Shrinkable<Intermediate>::StreamType {
                // assume interShr has no shrinks
                const Intermediate& interpair = interShr.getRef();
                const Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate>([interShr](const U& u) mutable {
                        const T& t = interShr.getRef().first;
                        return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<T, Shrinkable<U>> to Chain<T, U>
        return intermediate.template flatMap<Chain<T, U>>(
            +[](const Intermediate& interpair) -> Shrinkable<tuple<T, U>> {
                const T& t = interpair.first;
                return make_shrinkable<Chain<T, U>>(
                    tuple_cat(tuple<T>(t), util::make_tuple(interpair.second.getRef())));
            });
    });
}

template <typename U, typename T0, typename T1, typename... Ts>
Generator<Chain<T0, T1, Ts..., U>> chainImpl(GenFunction<Chain<T0, T1, Ts...>> gen1,
                                             Function<GenFunction<U>(Chain<T0, T1, Ts...>&)> gen2gen)
{
    using ChainType = Chain<T0, T1, Ts...>;
    using NextChainType = Chain<T0, T1, Ts..., U>;

    auto genTuple = [gen1, gen2gen](Random& rand) -> Shrinkable<NextChainType> {
        // generate T
        Shrinkable<ChainType> shrinkableTs = gen1(rand);
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
        return intermediate.template flatMap<NextChainType>(
            +[](const Intermediate& interpair) -> Shrinkable<NextChainType> {
                const ChainType& ts = interpair.first;
                return make_shrinkable<NextChainType>(tuple_cat(ts, tuple<U>(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}

GeneratorCommon chainImpl(Function1 gen1, Function1 gen2gen);

}  // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for chaining two generators to generate a tuple of values, where the second generator
 * depends on generated value from the first generator. Serves similar purpose as \ref derive, the only difference is in
 * the chained type (tuple).
 * @details Generates a tuple<T,U> with dependency.  Generator for U is decided by T value
 * @code
 *     GenFunction<tuple<T,U>> tupleGen = chain(intGen, [](int& intVal) {
 *         auto stringGen = Arbi<string>();
 *         stringGen.setMaxSize(intVal); // string size is dependent to intVal generated from intGen
 *         return stringGen;
 *     });
 *     // chain(gen, ...) is equivalent to gen.tupleWith(...), if gen is of Arbitrary or Generator type
 * @endcode
 */
template <GenLike GEN1, GenLikeGen<GEN1> GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen)
{
    using CHAIN = typename function_traits<GEN1>::return_type::type;  // T from shrinkable<T>(Random&)
    using RetType = typename function_traits<GEN2GEN>::return_type;
    static_assert(GenLike<RetType>, "gen2gen a callable of T -> Shrinkable<U>");
    using U = typename invoke_result_t<RetType, Random&>::type;  // U from shrinkable<U>(Random&)

    GenFunction<CHAIN> funcGen1 = gen1;
    Function<GenFunction<U>(CHAIN&)> funcGen2Gen = gen2gen;
    return util::chainImpl(funcGen1, funcGen2Gen);
}

}  // namespace proptest

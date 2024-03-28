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
Generator<Chain<T, U>> chainImpl(GenFunction<T> gen1, Function<GenFunction<U>(const T&)> gen2gen)
{
    return generator([gen1, gen2gen](Random& rand) -> Shrinkable<Chain<T, U>> {
        // generate T
        Shrinkable<T> shrinkableTs = gen1(rand);
        using Intermediate = pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<pair<T, Shrinkable<U>>> intermediate =
            shrinkableTs.template flatMap<pair<T, Shrinkable<U>>, T>([&rand, gen2gen](const T& t) mutable {
                // generate U
                auto gen2 = gen2gen(t);
                Shrinkable<U> shrinkableU = gen2(rand);
                return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) mutable -> Stream<Shrinkable<Intermediate>> {
                // assume interShr has no shrinks
                const Intermediate& interpair = interShr.template getRef<Intermediate>();
                const Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate, U>([interShr](const U& u) mutable {
                        const T& t = interShr.template getRef<Intermediate>().first;
                        return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<T, Shrinkable<U>> to Chain<T, U>
        return intermediate.template flatMap<Chain<T, U>, Intermediate>(
            +[](const Intermediate& interpair) -> Shrinkable<tuple<T, U>> {
                const T& t = interpair.first;
                return make_shrinkable<Chain<T, U>>(
                    tuple_cat(tuple<T>(t), util::make_tuple(interpair.second.template getRef<U>())));
            });
    });
}

template <typename U, typename T0, typename T1, typename... Ts>
Generator<Chain<T0, T1, Ts..., U>> chainImpl(GenFunction<Chain<T0, T1, Ts...>> gen1,
                                             Function<GenFunction<U>(const Chain<T0, T1, Ts...>&)> gen2gen)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    Function<GenFunction<U>(const Chain<T0, T1, Ts...>&)> gen2genFunc =
        [gen2gen](const Chain<T0, T1, Ts...>& ts) { return gen2gen(const_cast<Chain<T0, T1, Ts...>&>(ts)); };

    auto genTuple = [gen1Ptr, gen2genFunc](Random& rand) -> Shrinkable<Chain<T0, T1, Ts..., U>> {
        // generate T
        Shrinkable<Chain<T0, T1, Ts...>> shrinkableTs = (*gen1Ptr)(rand);
        using Intermediate = pair<Chain<T0, T1, Ts...>, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<tuple<Ts...>>
        Shrinkable<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>> intermediate =
            shrinkableTs.template flatMap<pair<Chain<T0, T1, Ts...>,Shrinkable<U>>, Chain<T0, T1, Ts...>>(
                [&rand, gen2genFunc](const Chain<T0, T1, Ts...>& ts) {
                    // generate U
                    auto gen2 = gen2genFunc(ts);
                    Shrinkable<U> shrinkableU = gen2(rand);
                    return make_shrinkable<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(util::make_pair(ts, shrinkableU));
                });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Stream<Shrinkable<Intermediate>> {
                // assume interShr has no shrinks
                const Shrinkable<U>& shrinkableU = interShr.template getRef<Intermediate>().second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate, U>([interShr](const U& u) mutable {
                        return make_shrinkable<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(
                            util::make_pair(interShr.template getRef<Intermediate>().first, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.getShrinks();
            });

        // reformat pair<Chain<T0, T1, Ts...>, Shrinkable<U>> to Chain<T0, T1, Ts..., U>
        return intermediate.template flatMap<Chain<T0, T1, Ts..., U>, Intermediate>(
            +[](const Intermediate& interpair) -> Shrinkable<tuple<T0, T1, Ts..., U>> {
                const Chain<T0, T1, Ts...>& ts = interpair.first;
                return make_shrinkable<Chain<T0, T1, Ts..., U>>(tuple_cat(ts, tuple<U>(interpair.second.template getRef<U>())));
            });
    };

    return generator(genTuple);
}

}  // namespace util


template <typename F, typename GEN, typename T = typename invoke_result_t<GEN, Random&>::type>
concept GenLikeGen = GenLike<GEN> && requires(F f, T& t) {
    { f(t) }
    -> GenLike;
};

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
    Function<GenFunction<U>(const CHAIN&)> funcGen2Gen = gen2gen;
    return util::chainImpl(funcGen1, funcGen2Gen);
}

}  // namespace proptest

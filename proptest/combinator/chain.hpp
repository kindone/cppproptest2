#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/std/tuple.hpp"
#include "proptest/combinator/combinatorimpl.hpp"

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
Generator<tuple<T, U>> chainImpl(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
{
    using Intermediate = pair<Any, ShrinkableBase>;
    Generator<Intermediate> intermediateGen = util::chainImpl1(gen1, [gen2gen](T& t) -> Function1<ShrinkableBase> { return gen2gen(t); });
    return intermediateGen.map(+[](const Intermediate& interpair) -> tuple<T, U> {
        return tuple<T, U>(interpair.first.getRef<T>(), interpair.second.getRef<U>());
    });
}

template <typename U, typename T0, typename T1, typename...Ts>
Generator<Chain<T0, T1, Ts..., U>> chainImpl(GenFunction<Chain<T0, T1, Ts...>> gen1,
                                             Function<GenFunction<U>(Chain<T0, T1, Ts...>&)> gen2gen)
{
    using ChainType = Chain<T0, T1, Ts...>;
    using NextChainType = Chain<T0, T1, Ts..., U>;
    using Intermediate = pair<Any, ShrinkableBase>;
    Generator<Intermediate> intermediateGen = util::chainImplN(gen1, [gen2gen](ChainType& c) -> Function1<ShrinkableBase> { return gen2gen(c); });
    return intermediateGen.map(+[](const Intermediate& interpair) -> NextChainType {
        const ChainType& ts = interpair.first.getRef<ChainType>();
        return tuple_cat(ts, tuple<U>(interpair.second.getRef<U>()));
    });
}

}  // namespace util

namespace gen {

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
 *     // chain(gen, ...) is equivalent to gen.tupleWith(...), if gen is an Arbitrary or a Generator type
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

} // namespace gen

}  // namespace proptest

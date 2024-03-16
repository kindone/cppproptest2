#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/generator/integral.hpp"

/**
 * @file aggregate.hpp
 * @brief Generator combinator for aggregating sequentially generated values into a value with a generator generator
 */

namespace proptest {

namespace util {

template <typename T>
Generator<T> aggregateImpl(GenFunction<T> gen1, Function<GenFunction<T>(const T&)> gen2gen, size_t minSize, size_t maxSize)
{
    return interval<uint64_t>(minSize, maxSize).flatMap<T>([gen1, gen2gen](const uint64_t& size) {
        return Generator<T>([gen1, gen2gen, size](Random& rand) {
            Shrinkable<T> shr = gen1(rand);
            for (size_t i = 0; i < size; i++)
                shr = gen2gen(shr.getRef())(rand);
            return shr;
        });
    });
}

}  // namespace util
/**
 * @ingroup Combinators
 * @brief Generator combinator for aggregating a value of type T from a generator generator
 * @tparam GEN1 Generator type of (Random&) -> Shrinkable<T>
 * @tparam GEN2GEN (T&) -> ((Random&) -> Shrinkable<T>) (Generator for T)
 * @param gen1 base generator for type T
 * @param gen2gen function that returns a generator for type T based on previously generated value of the same type
 * @param minSize minimum size of the aggregate steps
 * @param maxSize maximum size of the aggregate steps
 * @return last generated value of T throughout the aggregation
 */
template <GenLike GEN1, typename GEN2GEN>
    requires FunctionLike<GEN2GEN, invoke_result_t<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type&>, typename invoke_result_t<GEN1, Random&>::type&>
decltype(auto) aggregate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    using RetType = invoke_result_t<GEN2GEN, T&>;             // GEN2GEN's return type
    GenFunction<T> funcGen1 = gen1;
    Function<RetType(const T&)> funcGen2Gen = gen2gen;
    return util::aggregateImpl<T>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest

#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/generator/integral.hpp"

/**
 * @file accumulate.hpp
 * @brief Generator combinator for aggregating sequentially generated values into a value with a generator generator
 */

namespace proptest {

namespace util {

PROPTEST_API GeneratorCommon accumulateImpl(Function1 gen1, Function1 gen2gen, size_t minSize, size_t maxSize);

}  // namespace util
/**
 * @ingroup Combinators
 * @brief Generator combinator for accumulating a value of type T from a generator generator
 * @tparam GEN1 Generator type of (Random&) -> Shrinkable<T>
 * @tparam GEN2GEN (T&) -> ((Random&) -> Shrinkable<T>) (Generator for T)
 * @param gen1 base generator for type T
 * @param gen2gen function that returns a generator for type T based on previously generated value of the same type
 * @param minSize minimum size of the accumulate steps
 * @param maxSize maximum size of the accumulate steps
 * @return last generated value of T throughout the accumulation
 */
template <GenLike GEN1, typename GEN2GEN>
    requires FunctionLike<GEN2GEN, invoke_result_t<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type&>, typename invoke_result_t<GEN1, Random&>::type&>
decltype(auto) accumulate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    Function1 func1Gen1 = gen1;
    Function1 func1Gen2Gen([gen2gen](const Any& t) -> Function1 { return gen2gen(t.getRef<T>()); });
    return Generator<T>(util::accumulateImpl(func1Gen1, func1Gen2Gen, minSize, maxSize));
}

}  // namespace proptest

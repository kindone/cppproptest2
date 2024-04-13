#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/shrinker/listlike.hpp"

/**
 * @file aggregate.hpp
 * @brief Generator combinator for aggregating values into a vector where each value is generated from a given
 * generator generator
 */

namespace proptest {

namespace util {

template <typename T>
Generator<vector<T>> aggregateHelper(GenFunction<T> gen1, Function<GenFunction<T>(const T&)> gen2gen, size_t minSize,
                                    size_t maxSize)
{
    // Call aggregateImplAny with Any type
    Generator<vector<ShrinkableBase>> anyVecGen = aggregateImpl(gen1, [gen2gen](const Any& t) -> Function1 { return gen2gen(t.getRef<T>()); }, minSize, maxSize);

    // Convert the generated vector<Any> back to vector<T>
    return anyVecGen.template map<vector<T>>([](const vector<ShrinkableBase>& shrBaseVec) -> vector<T> {
        vector<T> tVec;
        tVec.reserve(shrBaseVec.size());
        for (const ShrinkableBase& shrBase : shrBaseVec) {
            tVec.push_back(shrBase.getAny().getRef<T>()); // Assuming Any has a get<T>() method
        }
        return tVec;
    });
}


}  // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for aggregating values into a vector where each value is generated from a given
 * generator generator
 * @tparam GEN1 Generator type of (Random&) -> Shrinkable<T>
 * @tparam GEN2GEN (T&) -> ((Random&) -> Shrinkable<T>) (Generator for T)
 * @param gen1 base generator for type T
 * @param gen2gen function that returns a generator for type T based on previously generated value of the same type
 * @param minSize minimum size of the aggregate
 * @param maxSize maximum size of the aggregate
 * @return vector of generated values of type T
 */
template <GenLike GEN1, typename GEN2GEN>
    requires FunctionLike<GEN2GEN, invoke_result_t<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type&>, typename invoke_result_t<GEN1, Random&>::type&>
decltype(auto) aggregate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    using RetType = invoke_result_t<GEN2GEN, T&>;
    GenFunction<T> funcGen1 = gen1;
    Function<RetType(T&)> funcGen2Gen = gen2gen;
    return util::aggregateHelper<T>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest

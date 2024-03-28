#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/shrinker/listlike.hpp"

/**
 * @file accumulate.hpp
 * @brief Generator combinator for accumulating values into a vector where each value is generated from a given
 * generator generator
 */

namespace proptest {

namespace util {

PROPTEST_API Generator<vector<Any>> accumulateImplAny(GenFunction<Any> gen1, Function<GenFunction<Any>(const Any&)> gen2gen, size_t minSize,
                                    size_t maxSize);

template <typename T>
Generator<vector<T>> accumulateImpl(GenFunction<T> gen1, Function<GenFunction<T>(const T&)> gen2gen, size_t minSize,
                                    size_t maxSize)
{
    // Convert gen1 to work with Any by wrapping its output
    GenFunction<Any> anyGen1 = [gen1](Random& rand) -> Shrinkable<Any> {
        return gen1(rand).template map<Any,T>([](const T& value) { return Any(value); });
    };

    // Convert gen2gen to work with Any by adapting its input and output
    Function<GenFunction<Any>(const Any&)> anyGen2Gen = [gen2gen](const Any& any) -> GenFunction<Any> {
        return [any, gen2gen](Random& rand) -> Shrinkable<Any> {
            return gen2gen(any.getRef<T>())(rand).template map<Any, T>([](const T& value) { return Any(value); });
        };
    };

    // Call accumulateImplAny with Any type
    auto anyVecGen = accumulateImplAny(anyGen1, anyGen2Gen, minSize, maxSize);

    // Convert the generated vector<Any> back to vector<T>
    return anyVecGen.template map<vector<T>>([](const vector<Any>& anyVec) -> vector<T> {
        vector<T> tVec;
        tVec.reserve(anyVec.size());
        for (const Any& any : anyVec) {
            tVec.push_back(any.getRef<T>()); // Assuming Any has a get<T>() method
        }
        return tVec;
    });
}

}  // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for accumulating values into a vector where each value is generated from a given
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
decltype(auto) accumulate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    using RetType = invoke_result_t<GEN2GEN, T&>;
    GenFunction<T> funcGen1 = gen1;
    Function<RetType(const T&)> funcGen2Gen = gen2gen;
    return util::accumulateImpl<T>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest

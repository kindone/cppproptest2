#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/std/type.hpp"
#include "proptest/combinator/combinatorimpl.hpp"

/**
 * @file transform.hpp
 * @brief Generator combinator for generating a type U from a generator for type T by applying transformation on
 * generated value of type T
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;
struct GeneratorCommon;

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a type U from a generator for type T by applying transformation on the
 * generated value of type T
 * @param gen generator for type T
 * @param transformer transformation function T& -> U
 */
template <typename T, typename U>
Generator<U> transform(Function<Shrinkable<T>(Random&)> gen, Function<U(T&)> transformer)
{
    return util::transformImpl(gen, transformer);
}

}  // namespace proptest

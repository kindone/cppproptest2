#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/GenType.hpp"
#include "proptest/combinator/combinatorimpl.hpp"

/**
 * @file derive.hpp
 * @brief Generator combinator for chaining two generators to generate a pair of values, where the second generator
 * depends on generated value from the first generator
 */


namespace proptest {

template <typename T> struct Generator;
struct GeneratorCommon;

/**
 * @ingroup Combinators
 * @brief Generator combinator for chaining two generators to generate a pair of values, where the second generator
 * depends on generated value from the first generator. Serves similar purpose as \ref chain() and the only difference
 * is in the chained type (pair vs. tuple).
 * @details Generates a tuple<T,U> with dependency.  Generator for U is decided by T value
 * @code
 *     GenFunction<pair<T,U>> pairGen = derive(intGen, [](const int& intVal) {
 *         auto stringGen = Arbi<string>();
 *         stringGen.setMaxSize(intVal); // string size is dependent to intVal generated from intGen
 *         return intVal;
 *     });
 *     // derive(gen, ...) is equivalent to gen.pairWith(...), if gen is of Arbitrary or Generator type
 * @endcode
 */
template <typename T, typename U>
Generator<U> derive(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
{
    return util::deriveImpl(gen1, [gen2gen](T& t) -> Function1 { return gen2gen(t); });
}

}  // namespace proptest

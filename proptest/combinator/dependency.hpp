#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/combinator/combinatorimpl.hpp"

/**
 * @file dependency.hpp
 * @brief Generator combinator for generating values with dependency or relation to a base generator
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating values with dependencies or relation to a base generator
 * @param gen1 base generator
 * @param gen2gen generator generator that generates a value based on generated value from the base generator
 * @return template <typename T, typename U>
 */
template <typename T, typename U>
Generator<pair<T, U>> dependency(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
{
    using Intermediate = pair<Any, ShrinkableBase>;
    Generator<Intermediate> intermediateGen = util::dependencyImpl(gen1, [gen2gen](T& t) -> Function1<ShrinkableBase> { return gen2gen(t); });
    return intermediateGen.map(+[](const Intermediate& interpair) {
        return util::make_pair(interpair.first.getRef<T>(), interpair.second.getRef<U>());
    });
}

}  // namespace proptest

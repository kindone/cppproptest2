#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/std/pair.hpp"

/**
 * @file dependency.hpp
 * @brief Generator combinator for generating values with dependency or relation to a base generator
 */

namespace proptest {

namespace util {
GeneratorCommon dependencyImpl(Function1 gen1, Function1 gen2gen);
} // namespace util

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
    Generator<Intermediate> intermediateGen = util::dependencyImpl(gen1, [gen2gen](T& t) -> Function1 { return gen2gen(t); });
    return intermediateGen.map(+[](const Intermediate& interpair) {
        return util::make_pair(interpair.first.getRef<T>(), interpair.second.getRef<U>());
    });
}


// returns a shrinkable pair of <T,U> where U depends on T
// template <typename T, typename U>
// Generator<pair<T, U>> dependency(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
// {
//     return generator([gen1, gen2gen](Random& rand) -> Shrinkable<pair<T, U>> {
//         // generate T
//         Shrinkable<T> shrinkableT = gen1(rand);
//         using Intermediate = pair<T, Shrinkable<U>>;

//         // shrink strategy 1: expand Shrinkable<T>
//         Shrinkable<pair<T, Shrinkable<U>>> intermediate =
//             shrinkableT.template flatMap<pair<T, Shrinkable<U>>>([&rand, gen2gen](const T& t) mutable {
//                 // generate U
//                 auto gen2 = gen2gen(t);
//                 Shrinkable<U> shrinkableU = gen2(rand);
//                 return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, shrinkableU));
//             });

//         // shrink strategy 2: expand Shrinkable<U>
//         intermediate =
//             intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Shrinkable<Intermediate>::StreamType {
//                 // assume interShr has no shrinks
//                 const Intermediate& interpair = interShr.get();
//                 const Shrinkable<U>& shrinkableU = interpair.second;
//                 Shrinkable<Intermediate> newShrinkableU =
//                     shrinkableU.template flatMap<Intermediate>([interShr](const U& u) mutable {
//                         return make_shrinkable<pair<T, Shrinkable<U>>>(
//                             util::make_pair(interShr.get().first, make_shrinkable<U>(u)));
//                     });
//                 return newShrinkableU.getShrinks();
//             });

//         // reformat pair<T, Shrinkable<U>> to pair<T, U>
//         return intermediate.template flatMap<pair<T, U>>(+[](const Intermediate& interpair) -> Shrinkable<pair<T, U>> {
//             return make_shrinkable<pair<T, U>>(util::make_pair(interpair.first, interpair.second.getRef()));
//         });
//     });
// }

}  // namespace proptest

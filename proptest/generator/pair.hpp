#pragma once

#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/pair.hpp"

/**
 * @file pair.hpp
 * @brief Arbitrary for pair<ARG1,ARG2> and utility function pairOf(gen1, gen2)
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Generator combinator for pair<T1, T2>
 * @details shrinking is done by one parameter and then continues to the next
 */
template <GenLike GEN1, GenLike GEN2>
decltype(auto) pairOf(GEN1&& gen1, GEN2&& gen2)
{
    auto genPairPtr =
        util::make_shared<pair<decay_t<GEN1>, decay_t<GEN2>>>(util::forward<GEN1>(gen1), util::forward<GEN2>(gen2));
    // generator
    return generator(
        [genPairPtr](Random& rand) mutable { return shrinkPair(genPairPtr->first(rand), genPairPtr->second(rand)); });
}

/**
 * @ingroup Generators
 * @brief Arbitrary for pair<T1, T2> based on Arbitrary<T1> and Arbitrary<T2>
 */
template <typename ARG1, typename ARG2>
class PROPTEST_API Arbi<pair<ARG1, ARG2>> final : public ArbiBase<pair<ARG1, ARG2>> {
public:

    using Pair = pair<ARG1, ARG2>;

    Arbi() : arg1Gen(Arbi<ARG1>()), arg2Gen(Arbi<ARG2>()) {}
    Arbi(GenFunction<ARG1> _arg1Gen, GenFunction<ARG2> _arg2Gen) : arg1Gen(_arg1Gen), arg2Gen(_arg2Gen) {}

    Shrinkable<pair<ARG1, ARG2>> operator()(Random& rand) const override { return pairOf(arg1Gen, arg2Gen)(rand); }

    shared_ptr<GeneratorBase<Pair>> clone() const override {
        return util::make_shared<Arbi>(arg1Gen, arg2Gen);
    }

private:
    GenFunction<ARG1> arg1Gen;
    GenFunction<ARG2> arg2Gen;
};

}  // namespace proptest

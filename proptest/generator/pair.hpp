#pragma once

#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/pair.hpp"

/**
 * @file pair.hpp
 * @brief Arbitrary for pair<ARG1,ARG2> and utility function pair(gen1, gen2)
 */

namespace proptest {

namespace gen {

/**
 * @ingroup Combinators
 * @brief Generator combinator for pair<T1, T2>
 * @details shrinking is done by one parameter and then continues to the next
 */
template <typename T1, typename T2>
Generator<proptest::pair<T1,T2>> pairOf(const GenFunction<T1>& gen1, const GenFunction<T2>& gen2)
{
    // generator
    return generator(
        [gen1, gen2](Random& rand) mutable -> Shrinkable<::proptest::pair<T1,T2>>{
            return shrinkPair<T1,T2>(gen1(rand), gen2(rand));
        });
}

template <GenLike GEN1, GenLike GEN2>
decltype(auto) pair(GEN1&& gen1, GEN2&& gen2)
{
    using T1 = typename function_traits<GEN1>::return_type::type;
    using T2 = typename function_traits<GEN2>::return_type::type;
    return pairOf<T1,T2>(util::forward<GEN1>(gen1), util::forward<GEN2>(gen2));
}

} // namespace gen

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

    Shrinkable<pair<ARG1, ARG2>> operator()(Random& rand) const override {
        return shrinkPair<ARG1, ARG2>(arg1Gen(rand), arg2Gen(rand));
    }

private:
    GenFunction<ARG1> arg1Gen;
    GenFunction<ARG2> arg2Gen;
};

}  // namespace proptest

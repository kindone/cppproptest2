#pragma once
#include "proptest/Random.hpp"
#include "proptest/GenType.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/combinator/combinatorimpl.hpp"

/**
 * @file oneof.hpp
 * @brief Generator combinator for generating a type by choosing one of given generators with some probability
 */

namespace proptest {

namespace util {
template <typename T>
struct Weighted;
}  // namespace util

namespace gen {

template <typename T>
util::Weighted<T> weightedGen(GenFunction<T> gen, double weight);

template <GenLike GEN>
auto weightedGen(GEN&& gen, double weight) -> util::Weighted<typename invoke_result_t<GEN, Random&>::type>;

} // namespace gen

namespace util {

struct WeightedBase
{
    WeightedBase(const Function1<ShrinkableBase>& _func, double _weight) : func(_func), weight(_weight) {}
    template <typename T>
    WeightedBase(const Weighted<T>& weighted) : func(weighted.func), weight(weighted.weight) {}

    Function1<ShrinkableBase> func;
    double weight;
};


template <typename T>
struct Weighted : WeightedBase
{
    using type = T;

    Weighted(const WeightedBase& base) : WeightedBase(base) {}
    Weighted(const Function1<ShrinkableBase>& _func, double _weight) : WeightedBase(_func, _weight) {}
};

template <typename T, GenLike<T> GEN>
Weighted<T> GenToWeighted(GEN&& gen)
{
    return gen::weightedGen<T>(util::forward<GEN>(gen), 0.0);
}

template <typename T>
Weighted<T> GenToWeighted(const Weighted<T>& weighted)
{
    return weighted;
}

}  // namespace util


namespace gen {

template <typename T>
util::Weighted<T> weightedGen(GenFunction<T> gen, double weight)
{
    return util::Weighted<T>(gen, weight);
}

/**
 * @ingroup Combinators
 * @brief Decorator function to pair a generator GEN with desired probability for use in `oneOf` combinator
 * @tparam GEN (optional) generator (function of Random& -> Shrinkable<T>)
 */
template <GenLike GEN>
auto weightedGen(GEN&& gen, double weight) -> util::Weighted<typename invoke_result_t<GEN, Random&>::type>
{
    using T = typename invoke_result_t<GEN, Random&>::type;
    return weightedGen<T>(util::forward<GEN>(gen), weight);
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a type by choosing one of given generators with some probability
 * @details You can combine generators into a single generator that can generate one of them with some probability. This
 * can be considered as taking a union of generators. It can generate a type T from multiple generators for type T, by
 * choosing one of the generators randomly, with even probability, or weighted probability. a GEN can be a generator or
 * a weightedGen(generator, weight) decorator with the weight between 0 and 1 (exclusive). Unweighted generators take rest of
 * unweighted probability evenly.
 */
template <typename T, typename... GENS>
    requires ((GenLike<GENS, T> || is_same_v<decay_t<GENS>, util::Weighted<T>>) && ...)
Generator<T> oneOf(GENS&&... gens)
{
    using WeightedVec = vector<util::WeightedBase>;
    shared_ptr<WeightedVec> genVecPtr(new WeightedVec{util::WeightedBase(util::GenToWeighted<T>(util::forward<GENS>(gens)))...});

    return util::oneOfImpl(genVecPtr);
}

/**
 * @ingroup Combinators
 * @brief Alias for \ref oneOf combinator
 */
template <typename T, typename... GENS>
decltype(auto) unionOf(GENS&&... gens)
{
    return ::proptest::gen::oneOf<T>(util::forward<GENS>(gens)...);
}

} // namespace gen

}  // namespace proptest

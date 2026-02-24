#pragma once

#include "proptest/Random.hpp"
#include "proptest/std/type.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/combinator/weighted.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/just.hpp"
#include "proptest/std/algorithm.hpp"
#include "proptest/combinator/combinatorimpl.hpp"

/**
 * @file elementof.hpp
 * @brief Generator combinator for generating a type by choosing one of given values with some probability
 */

namespace proptest {

namespace gen {

template <typename Impl, typename T = Impl>
util::WeightedValue<T> weightedVal(Impl&& value, double weight);

/**
 * @ingroup Combinators
 * @brief Unified weighted decorator for both elementOf and oneOf.
 * @details For values: returns WeightedValue<T> (use with elementOf or oneOf).
 *          For generators: returns Weighted<T> (use with oneOf only).
 *          Works with or without explicit T: weighted(value, prob), weighted<T>(value, prob),
 *          weighted(gen, prob), weighted<T>(gen, prob).
 */
template <typename Impl, typename T = decay_t<Impl>>
    requires(!invocable<Impl, Random&>) && (convertible_to<Impl, T> || same_as<decay_t<Impl>, T>)
util::WeightedValue<T> weighted(Impl&& value, double prob);

template <typename T = void, GenLike GEN>
    requires(same_as<T, void> || same_as<T, typename invoke_result_t<GEN, Random&>::type>)
auto weighted(GEN&& gen, double prob)
    -> util::Weighted<conditional_t<same_as<T, void>, typename invoke_result_t<GEN, Random&>::type, T>>;

} // namespace gen

namespace util {

template <typename T>
    requires(!is_same_v<decay_t<T>, WeightedValue<T>>)
WeightedValue<T> ValueToWeighted(T&& value)
{
    return gen::weightedVal<T>(util::forward<T>(value), 0.0);
}

template <typename T>
WeightedValue<T> ValueToWeighted(const WeightedValue<T>& weighted)
{
    return weighted;
}

// Reject gen::weighted(gen, prob) in elementOf — generators not allowed; provide clear error
template <typename>
struct dependent_false : false_type {};
template <typename T>
WeightedValue<T> ValueToWeighted(const util::Weighted<T>&)
{
    static_assert(dependent_false<T>::value,
        "elementOf accepts values only, not generators. "
        "Use gen::weighted(value, prob) or gen::weightedVal(value, prob) for values. "
        "For generators, use gen::oneOf with gen::weighted(gen, prob).");
}

}  // namespace util

namespace gen {

template <typename Impl, typename T>
util::WeightedValue<T> weightedVal(Impl&& value, double weight)
{
    return util::WeightedValue<T>(Any(util::forward<Impl>(value)), weight);
}

template <typename Impl, typename T>
    requires(!invocable<Impl, Random&>) && (convertible_to<Impl, T> || same_as<decay_t<Impl>, T>)
util::WeightedValue<T> weighted(Impl&& value, double prob)
{
    return weightedVal<T>(util::forward<Impl>(value), prob);
}

template <typename T, GenLike GEN>
    requires(same_as<T, void> || same_as<T, typename invoke_result_t<GEN, Random&>::type>)
auto weighted(GEN&& gen, double prob)
    -> util::Weighted<conditional_t<same_as<T, void>, typename invoke_result_t<GEN, Random&>::type, T>>
{
    return gen::weightedGen(util::forward<GEN>(gen), prob);
}

// a value can be a raw Impl or a weightedVal(Impl, weight)
/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a type by choosing one of given values with some probability
 * @details It can generate a type T from given values of type T, by choosing one of the values randomly, with even
 * probability, or weighted probability.
 * @tparam T result type
 * @tparam Impl an Impl can be a value of type T or a weightedValue(value of type T, weight) with the weight
 * between 0 and 1 (exclusive). Unweighted values take rest of unweighted probability evenly.
 */
template <typename T, typename... Impl>
decltype(auto) elementOf(Impl&&... values)
{
    using WeightedValueVec = vector<util::WeightedValue<T>>;
    using WeightedVec = vector<util::WeightedBase>;
    WeightedValueVec wvaluevec{util::ValueToWeighted<T>(util::forward<Impl>(values))...};

    auto genVecPtr = util::make_shared<WeightedVec>();

    util::transform(
        wvaluevec.begin(), wvaluevec.end(), util::back_inserter(*genVecPtr),
        +[](const util::WeightedValue<T>& wvalue) -> util::WeightedBase { return weightedGen<T>(gen::just<T>(wvalue.value), wvalue.weight); });

    return Generator<T>(util::oneOfImpl(genVecPtr));
}

} // namespace gen

}  // namespace proptest

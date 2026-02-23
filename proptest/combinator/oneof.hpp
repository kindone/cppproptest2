#pragma once
#include "proptest/Random.hpp"
#include "proptest/GenType.hpp"
#include "proptest/Generator.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/combinator/combinatorimpl.hpp"
#include "proptest/combinator/weighted.hpp"
#include "proptest/Shrinkable.hpp"

/**
 * @file oneof.hpp
 * @brief Generator combinator for generating a type by choosing one of given generators with some probability
 */

namespace proptest {

namespace gen {

template <typename T>
util::Weighted<T> weightedGen(GenFunction<T> gen, double weight);

template <GenLike GEN>
auto weightedGen(GEN&& gen, double weight) -> util::Weighted<typename invoke_result_t<GEN, Random&>::type>;

// Raw value: treat as gen::just<T>(value) — avoids weightedGen<T>(gen::just(value), weight) and lvalue deduction issues
template <typename T, typename Impl>
    requires(!GenLike<Impl, T>) && (convertible_to<Impl, T> || same_as<decay_t<Impl>, T>)
util::Weighted<T> weightedGen(Impl&& value, double weight);

} // namespace gen

namespace util {

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

// Raw value: treat as gen::just<T>(value) - inline to avoid circular include with just.hpp
template <typename T, typename Impl>
    requires (!GenLike<Impl, T>) && (!is_same_v<decay_t<Impl>, Weighted<T>>) &&
             (!is_same_v<decay_t<Impl>, WeightedValue<T>>) &&
             (convertible_to<Impl, T> || same_as<decay_t<Impl>, T>)
Weighted<T> GenToWeighted(Impl&& value)
{
    auto any = util::make_any<T>(util::forward<Impl>(value));
    return Weighted<T>(Function1<ShrinkableBase>([any](Random&) -> ShrinkableBase { return Shrinkable<T>(any); }), 0.0);
}

// gen::weighted(value, prob) -> WeightedValue<T>; convert to Weighted<T> for oneOf
template <typename T>
Weighted<T> GenToWeighted(const WeightedValue<T>& wval)
{
    return Weighted<T>(
        Function1<ShrinkableBase>([val = wval.value](Random&) -> ShrinkableBase { return Shrinkable<T>(val); }),
        wval.weight);
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
 * @brief Decorator to pair a raw value with a weight for use in `oneOf`. Treats value as gen::just(value).
 * @details Avoids weightedGen<T>(gen::just(value), weight) and lvalue deduction issues. Backward compatible.
 */
template <typename T, typename Impl>
    requires(!GenLike<Impl, T>) && (convertible_to<Impl, T> || same_as<decay_t<Impl>, T>)
util::Weighted<T> weightedGen(Impl&& value, double weight)
{
    auto any = util::make_any<T>(util::forward<Impl>(value));
    return util::Weighted<T>(
        Function1<ShrinkableBase>([any](Random&) -> ShrinkableBase { return Shrinkable<T>(any); }), weight);
}

// Concept: generator, Weighted<T>, WeightedValue<T>, or raw value convertible to T
template <typename T, typename Impl>
concept OneOfArg = GenLike<Impl, T> || is_same_v<decay_t<Impl>, util::Weighted<T>> ||
    is_same_v<decay_t<Impl>, util::WeightedValue<T>> ||
    ((convertible_to<Impl, T> || same_as<decay_t<Impl>, T>) && !GenLike<Impl, T> &&
     !is_same_v<decay_t<Impl>, util::Weighted<T>> && !is_same_v<decay_t<Impl>, util::WeightedValue<T>>);

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a type by choosing one of given generators with some probability
 * @details You can combine generators into a single generator that can generate one of them with some probability. This
 * can be considered as taking a union of generators. It can generate a type T from multiple generators for type T, by
 * choosing one of the generators randomly, with even probability, or weighted probability. Each argument can be:
 * - a generator (GenLike)
 * - weightedGen(generator, weight) with weight between 0 and 1 (exclusive)
 * - a raw value of type T (treated as gen::just(value))
 * Unweighted generators/values take rest of unweighted probability evenly.
 */
template <typename T, typename... GENS>
    requires (OneOfArg<T, GENS> && ...)
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

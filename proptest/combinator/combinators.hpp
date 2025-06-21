#pragma once

#include "proptest/combinator/just.hpp"
#include "proptest/combinator/lazy.hpp"
#include "proptest/combinator/reference.hpp"
#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/chain.hpp"
#include "proptest/combinator/elementof.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/intervals.hpp"
#include "proptest/combinator/aggregate.hpp"
#include "proptest/combinator/accumulate.hpp"
#include "proptest/combinator/construct.hpp"

// ============================================================================
// BACKWARD COMPATIBILITY ALIASES
// ============================================================================

namespace proptest {

// Combinator aliases for backward compatibility
// These functions are now implemented in proptest::gen namespace

template <typename T>
Generator<T> just(T&& value)
{
    return gen::just<T>(util::forward<T>(value));
}

template <typename T>
Generator<T> just(const T& value)
{
    return gen::just<T>(value);
}

template <typename T>
Generator<T> just(const Any& any)
{
    return gen::just<T>(any);
}

template <typename T, typename... Impl>
decltype(auto) elementOf(Impl&&... values)
{
    return gen::elementOf<T>(util::forward<Impl>(values)...);
}

template <typename T, typename... GENS>
    requires ((GenLike<GENS, T> || is_same_v<decay_t<GENS>, util::Weighted<T>>) && ...)
Generator<T> oneOf(GENS&&... gens)
{
    return gen::oneOf<T>(util::forward<GENS>(gens)...);
}

template <typename T, typename... GENS>
decltype(auto) unionOf(GENS&&... gens)
{
    return gen::unionOf<T>(util::forward<GENS>(gens)...);
}

template <typename CLASS, typename... ARGS>
Generator<CLASS> construct()
{
    return gen::construct<CLASS, ARGS...>();
}

template <typename CLASS, typename... ARGS, GenLike ExplicitGen0, GenLike...ExplicitGens>
Generator<CLASS> construct(ExplicitGen0&& gen0, ExplicitGens&&... gens)
{
    return gen::construct<CLASS, ARGS...>(util::forward<ExplicitGen0>(gen0), util::forward<ExplicitGens>(gens)...);
}

template <typename T, typename U>
Generator<U> transform(Function<Shrinkable<T>(Random&)> gen, Function<U(T&)> transformer)
{
    return gen::transform<T, U>(gen, transformer);
}

template <typename T, typename U>
Generator<U> derive(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
{
    return gen::derive<T, U>(gen1, gen2gen);
}

template <typename T>
Generator<T> filter(const Generator<T>& gen, Function<bool(T&)> criteria)
{
    return gen::filter<T>(gen, criteria);
}

template <typename T, GenLike GEN, typename Criteria>
decltype(auto) suchThat(GEN&& gen, Criteria&& criteria)
{
    return gen::suchThat<T>(util::forward<GEN>(gen), util::forward<Criteria>(criteria));
}

template <typename T, typename U>
Generator<pair<T, U>> dependency(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen)
{
    return gen::dependency<T, U>(gen1, gen2gen);
}

template <GenLike GEN1, GenLikeGen<GEN1> GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen)
{
    return gen::chain(util::forward<GEN1>(gen1), util::forward<GEN2GEN>(gen2gen));
}

template <GenLike GEN1, typename GEN2GEN>
    requires FunctionLike<GEN2GEN, invoke_result_t<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type&>, typename invoke_result_t<GEN1, Random&>::type&>
decltype(auto) aggregate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    return gen::aggregate(util::forward<GEN1>(gen1), util::forward<GEN2GEN>(gen2gen), minSize, maxSize);
}

template <GenLike GEN1, typename GEN2GEN>
    requires FunctionLike<GEN2GEN, invoke_result_t<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type&>, typename invoke_result_t<GEN1, Random&>::type&>
decltype(auto) accumulate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    return gen::accumulate(util::forward<GEN1>(gen1), util::forward<GEN2GEN>(gen2gen), minSize, maxSize);
}

template <typename T, typename LazyEval>
    requires(is_convertible_v<LazyEval&&, function<T()>>)
Generator<T> lazy(LazyEval&& lazyEval)
{
    return gen::lazy<T>(util::forward<LazyEval>(lazyEval));
}

template <typename LazyEval>
auto lazy(LazyEval&& lazyEval) -> Generator<invoke_result_t<LazyEval>>
{
    return gen::lazy(util::forward<LazyEval>(lazyEval));
}

template <GenLike GEN>
auto reference(GEN&& gen) -> Generator<typename invoke_result_t<GEN, Random&>::type>
{
    return gen::reference(util::forward<GEN>(gen));
}

inline Generator<int64_t> intervals(initializer_list<Interval> interval_list)
{
    return gen::intervals(interval_list);
}

inline Generator<uint64_t> uintervals(initializer_list<UInterval> interval_list)
{
    return gen::uintervals(interval_list);
}

} // namespace proptest


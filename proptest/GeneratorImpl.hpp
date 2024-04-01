#pragma once

#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/chain.hpp"

namespace proptest {

/* fwd-declaration of combinators */
template <typename T, typename U>
Generator<U> transform(Function<Shrinkable<T>(Random&)> gen, Function<U(T&)> transformer);

template <typename T, GenLike<T> GEN, typename Criteria>
Generator<T> filter(GEN&& gen, Criteria&& criteria);

template <typename T, typename U>
Generator<pair<T, U>> dependency(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen);

template <GenLike GEN1, GenLikeGen<GEN1> GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen);

template <typename T, typename U>
Generator<U> derive(GenFunction<T> gen1, Function<GenFunction<U>(T&)> gen2gen);

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::map(Function<U(T&)> mapper)
{
    return Generator<U>(proptest::transform<T, U>(this->asGenFunction(), mapper));
}

template <typename T>
template <typename Criteria>
Generator<T> GeneratorBase<T>::filter(Criteria&& criteria)
{
    return Generator<T>(proptest::filter<T>(this->asGenFunction(), util::forward<Criteria>(criteria)));
}

template <typename T>
template <typename U>
Generator<pair<T, U>> GeneratorBase<T>::pairWith(Function<GenFunction<U>(T&)> genFactory)
{
    return proptest::dependency<T, U>(this->asGenFunction(), genFactory);
}

template <typename T>
template <typename U>
decltype(auto) GeneratorBase<T>::tupleWith(Function<GenFunction<U>(T&)> genFactory)
{
    return proptest::chain(this->asGenFunction(), genFactory);
}

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::flatMap(Function<GenFunction<U>(T&)> genFactory)
{
    return Generator<U>(proptest::derive<T, U>(this->asGenFunction(), genFactory));
}

}  // namespace proptest

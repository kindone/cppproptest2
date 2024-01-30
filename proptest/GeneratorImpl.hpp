#pragma once

#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/chain.hpp"

namespace proptest {

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::map(Function<U(const T&)> mapper)
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
Generator<pair<T, U>> GeneratorBase<T>::pairWith(Function<GenFunction<U>(const T&)> genFactory)
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
Generator<U> GeneratorBase<T>::flatmap(Function<U(T&)> genFactory)
{
    return Generator<U>(proptest::derive<T, U>(this->asGenFunction(), genFactory));
}

}  // namespace proptest
#pragma once

#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/chain.hpp"

namespace proptest {

/* fwd-declaration of combinators */

namespace util {
PROPTEST_API GeneratorCommon filterImpl(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon dependencyImpl(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon transformImpl(Function1 gen, Function1 criteria);
} // namespace util

template <GenLike GEN1, GenLikeGen<GEN1> GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen);

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::map(Function<U(T&)> mapper)
{
    // return Generator<U>(proptest::transform<T, U>(this->asGenFunction(), mapper));
    return util::transformImpl(asGenFunction1(), mapper);
}

template <typename T>
template <typename Criteria>
Generator<T> GeneratorBase<T>::filter(Criteria&& criteria)
{
    // return Generator<T>(proptest::filter<T>(this->asGenFunction(), util::forward<Criteria>(criteria)));
    return util::filterImpl(asGenFunction1(), util::forward<Criteria>(criteria));
}

template <typename T>
template <typename U>
Generator<pair<T, U>> GeneratorBase<T>::pairWith(Function<GenFunction<U>(T&)> genFactory)
{
    // return proptest::dependency<T, U>(this->asGenFunction(), genFactory);
    // return proptest::dependency(asGenFunction1(), genFactory);
    using Intermediate = pair<Any, ShrinkableBase>;
    Generator<Intermediate> intermediateGen = util::dependencyImpl(asGenFunction1(), [=](T& t) { return Function1(genFactory(t));} );
    return intermediateGen.map(+[](const Intermediate& interpair) {
        return util::make_pair(interpair.first.getRef<T>(), interpair.second.getRef<U>());
    });
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
    // return Generator<U>(proptest::derive<T, U>(this->asGenFunction(), genFactory));
    return util::deriveImpl(asGenFunction1(), [=](T& t) { return Function1(genFactory(t));} );
}

}  // namespace proptest

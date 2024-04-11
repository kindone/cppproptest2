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
PROPTEST_API GeneratorCommon chainImpl1(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon chainImplN(Function1 gen, Function1 criteria);
} // namespace util

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::map(Function<U(T&)> mapper)
{
    return util::transformImpl(asGenFunction1(), mapper);
}

template <typename T>
template <typename Criteria>
Generator<T> GeneratorBase<T>::filter(Criteria&& criteria)
{
    return util::filterImpl(asGenFunction1(), util::forward<Criteria>(criteria));
}

template <typename T>
template <typename U>
Generator<pair<T, U>> GeneratorBase<T>::pairWith(Function<GenFunction<U>(T&)> genFactory)
{
    using Intermediate = pair<Any, ShrinkableBase>;
    Generator<Intermediate> intermediateGen = util::dependencyImpl(asGenFunction1(), [=](T& t) { return Function1(genFactory(t));} );
    return intermediateGen.map(+[](const Intermediate& interpair) {
        return util::make_pair(interpair.first.getRef<T>(), interpair.second.getRef<U>());
    });
}

namespace util {

template<typename T>
concept TupleLike =
requires (T a) {
    tuple_size<T>::value;
    get<0>(a);
};

} // namespace util

template <typename T>
template <typename U>
decltype(auto) GeneratorBase<T>::tupleWith(Function<GenFunction<U>(T&)> genFactory)
{
    if constexpr (util::TupleLike<T>) {
        using Intermediate = pair<Any, ShrinkableBase>;
        Generator<Intermediate> intermediateGen = util::chainImplN(asGenFunction1(), [genFactory](T& c) { return Function1(genFactory(c)); });
        return intermediateGen.map(+[](const Intermediate& interpair) {
            const T& ts = interpair.first.getRef<T>();
            return tuple_cat(ts, tuple<U>(interpair.second.getRef<U>()));
        });
    } else {
        using Intermediate = pair<Any, ShrinkableBase>;
        Generator<Intermediate> intermediateGen = util::chainImpl1(asGenFunction1(), [genFactory](T& t) { return Function1(genFactory(t)); });
        return intermediateGen.map(+[](const Intermediate& interpair) -> tuple<T, U> {
            return tuple<T, U>(interpair.first.getRef<T>(), interpair.second.getRef<U>());
        });
    }
}

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::flatMap(Function<GenFunction<U>(T&)> genFactory)
{
    // return Generator<U>(proptest::derive<T, U>(this->asGenFunction(), genFactory));
    return util::deriveImpl(asGenFunction1(), [=](T& t) { return Function1(genFactory(t));} );
}

}  // namespace proptest

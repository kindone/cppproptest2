#pragma once

#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/chain.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/combinatorimpl.hpp"



namespace proptest {

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::map(Function<U(T&)> mapper)
{
    return util::transformImpl(asGenFunction1(), mapper);
}

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::map(Function<Any(T&)> mapper)
{
    return util::transformImpl(asGenFunction1(), mapper);
}

// template <typename T>
// template <typename U>
// Generator<U> GeneratorBase<T>::map(Function<unique_ptr<U>(T&)> mapper)
// {
//     return util::transformImpl(asGenFunction1(), mapper);
// }

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
    Generator<Intermediate> intermediateGen = util::dependencyImpl(asGenFunction1(), [=](T& t) { return Function1<ShrinkableBase>(genFactory(t));} );
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
        Generator<Intermediate> intermediateGen = util::chainImplN(asGenFunction1(), [genFactory](T& c) { return Function1<ShrinkableBase>(genFactory(c)); });
        return intermediateGen.map(+[](const Intermediate& interpair) {
            const T& ts = interpair.first.getRef<T>();
            return tuple_cat(ts, tuple<U>(interpair.second.getRef<U>()));
        });
    } else {
        using Intermediate = pair<Any, ShrinkableBase>;
        Generator<Intermediate> intermediateGen = util::chainImpl1(asGenFunction1(), [genFactory](T& t) { return Function1<ShrinkableBase>(genFactory(t)); });
        return intermediateGen.map(+[](const Intermediate& interpair) -> tuple<T, U> {
            return tuple<T, U>(interpair.first.getRef<T>(), interpair.second.getRef<U>());
        });
    }
}

template <typename T>
template <typename U>
Generator<U> GeneratorBase<T>::flatMap(Function<GenFunction<U>(T&)> genFactory)
{
    return util::deriveImpl(asGenFunction1(), [=](T& t) { return Function1<ShrinkableBase>(genFactory(t));} );
}

}  // namespace proptest

#ifdef PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

#define EXTERN_DECLARE_GENERATORBASE(TYPE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::GeneratorBase, TYPE)
#define EXTERN_DECLARE_GENERATOR(TYPE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::Generator, TYPE)

DEFINE_FOR_ALL_BASIC_TYPES(EXTERN_DECLARE_GENERATORBASE);
DEFINE_FOR_ALL_BASIC_TYPES(EXTERN_DECLARE_GENERATOR);

#endif // PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"

/**
 * @file filter.hpp
 * @brief Generator combinator for generating a type with an additional constraint
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief You can add a filtering condition to a generator to restrict the generated values to have certain constraint
 * @tparam T generated type
 * @tparam GEN base generator for type T
 * @tparam Criteria a callable with signature T& -> bool
 * @details returns a generator for type T that satisfies criteria predicate (criteria(t) returns true)
 * @code
 * // returns generator for even numbers only out of values generated by intGen
 * filter<int>(intGen, [](int& num) { return num % 2 == 0; });
 * @endcode
 */
template <typename T, GenLike<T> GEN, typename Criteria>
Generator<T> filter(GEN&& gen, Criteria&& criteria)
{
    static_assert(is_convertible_v<Criteria&&, Function<bool(T&)>>, "criteria must be a callable of T& -> bool");
    static_assert(is_convertible_v<GEN&&, Function<Shrinkable<T>(Random&)>>,
                  "Gen must be a GenFunction<T> or a callable of Random& -> Shrinkable<T>");
    Function<Shrinkable<T>(Random&)> genFunc = gen;
    Function<bool(const T&)> criteriaFunc = criteria;
    return Generator<T>([genFunc, criteriaFunc](Random& rand) {
        // TODO: add some configurable termination criteria (e.g. maximum no. of attempts)
        while (true) {
            Shrinkable<T> shrinkable = genFunc(rand);
            if (criteriaFunc(shrinkable.getRef<T>())) {
                return shrinkable.filter(criteriaFunc, 1);  // 1: tolerance
            }
        }
    });
}

/**
 * @brief You can add a filtering condition to a generator to restrict the generated values to have certain constraint
 * @tparam T generated type (deduced)
 * @tparam GEN base generator for type T
 * @tparam Criteria a callable with signature T& -> bool
 * @details returns a generator for type T that satisfies criteria predicate (criteria(t) returns true)
 * e.g. filter<int>(intGen, [](int& num) { return num % 2 == 0; }); // returns generator for even numbers only out of
 * values generated by intGen
 */
template <GenLike GEN, typename Criteria>
Generator<typename invoke_result_t<GEN, Random&>::type> filter(GEN&& gen, Criteria&& criteria)
{
    using T = typename invoke_result_t<GEN, Random&>::type;
    static_assert(is_convertible_v<Criteria&&, function<bool(T&)>>, "criteria must be a callable of T& -> bool");
    static_assert(is_convertible_v<GEN&&, Function<Shrinkable<T>(Random&)>>,
                  "Gen must be a GenFunction<T> or a callable of Random& -> Shrinkable<T>");

    return filter<T, GEN, Criteria>(util::forward<GEN>(gen), util::forward<Criteria>(criteria));
}

/**
 * @ingroup Combinators
 * @brief alias for \ref filter combinator
 */
template <typename T, GenLike GEN, typename Criteria>
decltype(auto) suchThat(GEN&& gen, Criteria&& criteria)
{
    return filter<T, GEN, Criteria>(util::forward<GEN>(gen), util::forward<Criteria>(criteria));
}

}  // namespace proptest

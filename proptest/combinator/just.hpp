#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/std/type.hpp"

/**
 * @file just.hpp
 * @brief Generator combinator for generating just a specific value
 */

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating just a specific value
 * @details Will always generate a specific value of type T. e.g. just(1339) will generate 1339
 */
template <typename T>
Generator<T> just(T&& value)
{
    if constexpr (is_trivial<T>::value)
        return generator([value](Random&) { return make_shrinkable<T>(value); });
    else if(!is_trivial<T>::value)
    {
        auto any = util::make_any<T>(util::forward<T>(value));  // requires copy constructor
        return generator([any](Random&) { return Shrinkable<T>(any); });
    }
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating just a specific value
 * @details Will always generate a specific value of type T.
 */
template <typename T>
Generator<T> just(const T& value)
{
    auto any = util::make_any<T>(value);  // requires copy constructor
    return generator([any](Random&) { return Shrinkable<T>(any); });
}

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating just a specific value
 * @details Will always generate a specific value of type T.
 */
template <typename T>
Generator<T> just(const Any& any)
{
    return generator([any](Random&) { return Shrinkable<T>(any); });
}

} // namespace proptest

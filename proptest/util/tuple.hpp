#pragma once
#include "proptest/std/tuple.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/concepts.hpp"

namespace proptest {

namespace util {

template<std::size_t N>
struct Num { static const constexpr size_t value = N; };

template <class F, size_t... Is>
// func is a template lambda that takes (auto index_sequence) as an argument
void For(F func, index_sequence<Is...>)
{
    (func(std::integral_constant<size_t, Is>{}), ...);
}

/* Example:
    util::Map([&] (auto index_sequence) {
        return x; // where x is the desired tuple element
    }, make_index_sequence<N>{}); // N is the number of elements for the tuple
*/
template <class F, size_t... Is>
// func is a template lambda that takes (auto index_sequence) as an argument
decltype(auto) Map(F func, index_sequence<Is...>)
{
    return util::make_tuple(func(std::integral_constant<size_t, Is>{})...);
}

} // namespace util

} // namespace proptest
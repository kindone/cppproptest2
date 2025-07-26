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
void For([[maybe_unused]] F func, index_sequence<Is...>)
{
    (func(std::integral_constant<size_t, Is>{}), ...);
}

template <size_t N, class F>
// func is a template lambda that takes (auto index_sequence) as an argument
void For(F&& func)
{
    For(util::forward<F>(func), make_index_sequence<N>{});
}


/* Example:
    util::Map([&] (auto index_sequence) {
        return x; // where x is the desired tuple element
    }, make_index_sequence<N>{}); // N is the number of elements for the tuple

    or util::Map<N>(callable);
  Result: tuple of xs
*/
template <class F, size_t... Is>
// F is a template lambda that takes (auto index_sequence) as an argument
decltype(auto) Map(F func, index_sequence<Is...>)
{
    return util::make_tuple(func(std::integral_constant<size_t, Is>{})...);
}

template <size_t N, class F>
decltype(auto) Map(F&& func)
{
    return Map(util::forward<F>(func), make_index_sequence<N>{});
}

/* Example:
    util::Call(callable, [&] (auto index_sequence) {
        return x; // where x is the desired tuple element
    }, make_index_sequence<N>{}); // N is the number of elements for the tuple

  Result: instead of creating results as tuple, pass into callable as arguments
*/
template <class Func, class ArgFunc, size_t... Is>
// ArgFunc is a template lambda that takes (auto index_sequence) as an argument
decltype(auto) Call(Func&& func, ArgFunc&& argFunc, index_sequence<Is...>)
{
    // Use std::forward to preserve reference-ness of each argument
    return std::forward<Func>(func)(std::forward<decltype(argFunc(std::integral_constant<size_t, Is>{}))>(argFunc(std::integral_constant<size_t, Is>{}))...);
}

template <size_t N, class Func, class ArgFunc>
decltype(auto) Call(Func&& func, ArgFunc&& argFunc)
{
    return Call(util::forward<Func>(func), util::forward<ArgFunc>(argFunc), make_index_sequence<N>{});
}

template <class Func, class ArgFunc, typename...ARGS, size_t... Is>
// ArgFunc is a template lambda that takes (auto index_sequence) as an argument
decltype(auto) CallWithAny(Func&& func, ArgFunc&& argFunc, index_sequence<Is...>)
{
    return func(argFunc(std::integral_constant<size_t, Is>{}).template getRef<ARGS>()...);
}

template <class Func, class ArgFunc, typename...ARGS>
decltype(auto) CallWithAny(Func&& func, ArgFunc&& argFunc)
{
    constexpr size_t N = sizeof...(ARGS);
    return CallWithAny<Func, ArgFunc, ARGS...>(util::forward<Func>(func), util::forward<ArgFunc>(argFunc), make_index_sequence<N>{});
}



} // namespace util

} // namespace proptest

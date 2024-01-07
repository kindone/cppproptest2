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
    (func(Num<Is>{}), ...);
}

} // namespace util

} // namespace proptest
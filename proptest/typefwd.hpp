#pragma once
#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/typefwd.hpp"


namespace proptest {

template <typename T> class Nullable;
// any.hpp
struct AnyHolder;
template <typename T> struct AnyRef;
template <typename T> struct AnyVal;
struct Any;

// Shrinkable.hpp
template <typename T> struct Shrinkable;

// stateful.hpp
template <typename ObjectType, typename ModelType>
struct Action;

// Random
struct Random;

template <class F, size_t... Is>
void For(F func, index_sequence<Is...>);

} // namespace proptest
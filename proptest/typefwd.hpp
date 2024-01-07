#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"

namespace proptest {

// any.hpp
struct AnyHolder;
template <typename T> struct AnyRef;
template <typename T> struct AnyVal;
struct Any;

struct Random;

template <class F, size_t... Is>
void For(F func, index_sequence<Is...>);

} // namespace proptest
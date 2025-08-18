#pragma once

#include "proptest/std/concepts.hpp"
#include "proptest/std/functional.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

template <typename F, typename RET = typename invoke_result_t<F, Random&>::type, typename...ARGS>
concept FunctionLike = requires(F f, ARGS... args) {
    { f(args...) }
    -> same_as<RET>;
};

template <typename F, typename T = typename invoke_result_t<F, Random&>::type, typename S = T>
concept GenLike = requires(F f, Random& rand) {
    { f(rand) }
    -> same_as<Shrinkable<S>>;
};

template <typename F, typename T>
concept Gen = GenLike<F, T, T>;

template <typename T>
using GenFunction = Function<Shrinkable<T>(Random&)>;

template <typename F, typename GEN, typename T = typename invoke_result_t<GEN, Random&>::type>
concept GenLikeGen = GenLike<GEN> && requires(F f, T& t) {
    { f(t) }
    -> GenLike;
};

// forward-declarations
template <typename T> struct Generator;
template <typename T> struct Arbi;


/* Type-erased type */
struct GeneratorCommon {
    GeneratorCommon(const GeneratorCommon& other) : func(other.func) {}

    GeneratorCommon(const Function1<ShrinkableBase>& _func) : func(_func) {}

    template <typename T>
    GeneratorCommon(const Generator<T>& gen) : func(gen.func) {}

    template <typename T>
    GeneratorCommon(const Arbi<T>& arbi) : func([arbi](Random& rand) -> ShrinkableBase { return arbi(rand); }) {}

    Function1<ShrinkableBase> func;
};

} // namespace proptest

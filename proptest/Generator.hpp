#pragma once

#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/std/tuple.hpp"

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

// forward-declarations
template <typename T> struct Generator;
template <typename T> struct Arbi;

template <typename T>
struct PROPTEST_API GeneratorBase
{
public:
    virtual Shrinkable<T> operator()(Random& rand) const = 0;

    template <typename U>
    Generator<U> map(Function<U(const T&)> mapper);

    template <typename Criteria>
    Generator<T> filter(Criteria&& criteria);

    template <typename U>
    Generator<pair<T, U>> pairWith(Function<GenFunction<U>(const T&)> genFactory);

    template <typename U>
    decltype(auto) tupleWith(Function<GenFunction<U>(T&)> genFactory);

    template <typename U>
    Generator<U> flatmap(Function<U(T&)> genFactory);

    virtual shared_ptr<GeneratorBase> clone() const = 0;

    GenFunction<T> asGenFunction() {
        auto thisClone = clone();
        return [thisClone](Random& rand) -> Shrinkable<T> {
            return thisClone->operator()(rand);
        };
    }
};

template <typename T>
struct PROPTEST_API Generator : public GeneratorBase<T>
{
public:
    Generator(Function<Shrinkable<T>(Random&)> _func) : func(_func) {}

    Shrinkable<T> operator()(Random& rand) const override { return this->func(rand); }

    shared_ptr<GeneratorBase<T>> clone() const override {
        return util::make_shared<Generator>(func);
    }

private:
    Function<Shrinkable<T>(Random&)> func;
};

/**
 * @ingroup Generators
 * @brief Helper function to create \ref Generator<T> from generator functions to provide utility methods.
 */
template <GenLike GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(util::forward<GEN>(gen));
}

template <GenLike GEN>
decltype(auto) asGenFunction(GEN&& gen)
{
    using retType = decltype(gen(declval<Random&>()));
    return static_cast<Function<retType(Random&)>>(gen);
}

struct AnyGenerator
{
    template <typename T>
    AnyGenerator(const Generator<T>& gen) : anyGen(gen)
    {
    }

    template <typename T>
    AnyGenerator(GenFunction<T> gen) : anyGen(gen) {}

    template <typename T>
    AnyGenerator(const Arbi<T>& arbi) : anyGen(arbi) {}

    ShrinkableAny operator()(Random& rand) const;

    template <typename T>
    Shrinkable<T> generate(Random& rand) {
        return anyGen(rand).getRef().getRef<T>();
    }

private:
    Function<ShrinkableAny(Random&)> anyGen;
};

extern template struct Function<ShrinkableAny(Random&)>;

}  // namespace proptest


#include "proptest/GeneratorImpl.hpp"

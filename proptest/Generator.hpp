#pragma once

#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"

namespace proptest {

template <typename F, typename...ARGS, typename RET = typename invoke_result_t<F, Random&>::type>
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

template <typename T>
class PROPTEST_API GeneratorBase
{
public:
    virtual Shrinkable<T> operator()(Random& rand) const = 0;
};

template <typename T>
class PROPTEST_API Generator : public GeneratorBase<T>
{
public:
    Generator(const Function<Shrinkable<T>(Random&)>& _func) : func(_func) {}

    Shrinkable<T> operator()(Random& rand) const override { return this->func(rand); }

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
decltype(auto) callableToGenFunction(GEN&& gen)
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

    ShrinkableAny operator()(Random& rand) {
        return anyGen(rand);
    }

    template <typename T>
    Shrinkable<T> generate(Random& rand) {
        return anyGen(rand).getRef().getRef<T>();
    }

private:
    Function<ShrinkableAny(Random&)> anyGen;
};

}  // namespace proptest
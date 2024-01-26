#pragma once

#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"

namespace proptest {

template <typename F, typename T, typename S = T>
concept GenFunctionLike = requires(F f, Random& rand) {
    { f(rand) }
    -> same_as<Shrinkable<S>>;
};

template <typename F, typename T>
concept GenFunction = GenFunctionLike<F, T, T>;

template <typename T>
class PROPTEST_API GenBase
{
public:
    virtual Shrinkable<T> operator()(Random& rand) const = 0;
};

template <typename T>
class PROPTEST_API Generator : public GenBase<T>
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
template <typename GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(util::forward<GEN>(gen));
}

}  // namespace proptest
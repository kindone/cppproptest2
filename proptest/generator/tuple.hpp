#pragma once

#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/tuple.hpp"

namespace proptest {

/**
 * @file tuple.hpp
 * @brief Arbitrary for tuple<T1,..., Tn> and utility function tupleOf(gen0, ..., gens)
 * @details shrinking is done by one parameter and then continues to the next
 */

/**
 * @ingroup Combinators
 * @brief Generator combinator for tuple<T1, ..., Tn> with given generators for T1, ..., Tn
 */
template <GenLike GEN0, GenLike... GENS>
decltype(auto) tupleOf(GEN0&& gen0, GENS&&... gens)
{
    constexpr auto Size = sizeof...(GENS) + 1;
    auto genTup = util::make_tuple(generator(gen0), generator(gens)...);
    // generator
    return generator([genTup](Random& rand) mutable {
        auto shrTup = util::Map<Size>([&rand, genTup](auto index_sequence) {
            return get<index_sequence.value>(genTup)(rand);
        });
        return shrinkTuple(make_shrinkable<decltype(shrTup)>(shrTup));
    });
}

/**
 * @ingroup Generators
 * @brief Arbitrary for tuple<T1, ..., Tn> based on Arbitraries for T1, ..., Tn
 */
template <typename... ARGS>
class PROPTEST_API Arbi<tuple<ARGS...>> final : public ArbiBase<tuple<ARGS...>> {
public:
    static constexpr auto Size = sizeof...(ARGS);
    using Tuple = tuple<ARGS...>;

    Arbi() : genTup(Arbi<ARGS>()...) {}

    Arbi(GenFunction<ARGS>... gens) : genTup(gens...) {}

    Arbi(const tuple<GenFunction<ARGS>...>& _genTup) : genTup(_genTup) {}

    Shrinkable<tuple<ARGS...>> operator()(Random& rand) const override {
        auto shrTup = util::Map<Size>([&rand, genTup=this->genTup](auto index_sequence) {
            return get<index_sequence.value>(genTup)(rand);
        });
        return shrinkTuple(make_shrinkable<decltype(shrTup)>(shrTup));
    }

    shared_ptr<GeneratorBase<Tuple>> clone() const override {
        return util::make_shared<Arbi>(genTup);
    }

private:
    tuple<GenFunction<ARGS>...> genTup;
};

}  // namespace proptest

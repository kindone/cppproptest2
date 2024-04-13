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
    using ArgTypeList = util::TypeList<typename function_traits<GEN0>::return_type::type, typename function_traits<GENS>::return_type::type...>;
    auto genVec = util::make_shared<vector<AnyGenerator>, initializer_list<AnyGenerator>>({generator(gen0), generator(gens)...});
    // generator
    return generator([genVec](Random& rand) mutable {
        // generate into new vector
        auto shrVecPtr = util::make_shared<vector<ShrinkableBase>>();
        shrVecPtr->reserve(Size);
        for(const auto& gen : *genVec) {
            shrVecPtr->push_back(gen(rand));
        }
        return shrinkTuple(ArgTypeList{}, make_shrinkable<vector<ShrinkableBase>>(shrVecPtr));
    });
}

/**
 * @ingroup Generators
 * @brief Arbitrary for tuple<T1, ..., Tn> based on Arbitraries for T1, ..., Tn
 */
template <typename... ARGS>
    requires (sizeof...(ARGS) > 0)
class PROPTEST_API Arbi<tuple<ARGS...>> final : public ArbiBase<tuple<ARGS...>> {
public:
    static constexpr auto Size = sizeof...(ARGS);
    using Tuple = tuple<ARGS...>;

    Arbi() : genVec{generator(Arbi<ARGS>())...} {}

    Arbi(GenFunction<ARGS>... gens) : genVec{gens...} {}

    // Arbi(const tuple<GenFunction<ARGS>...>& _genTup) : genTup(_genTup) {}

    Shrinkable<tuple<ARGS...>> operator()(Random& rand) const override {
        auto shrVecPtr = util::make_shared<vector<ShrinkableBase>>();
        shrVecPtr->reserve(Size);
        for(auto& gen : genVec) {
            shrVecPtr->push_back(gen(rand));
        }
        return shrinkTuple<ARGS...>(make_shrinkable<vector<ShrinkableBase>>(shrVecPtr));
    }

private:
    vector<AnyGenerator> genVec;
};

}  // namespace proptest

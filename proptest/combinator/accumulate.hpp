#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/shrinker/listlike.hpp"

/**
 * @file accumulate.hpp
 * @brief Generator combinator for accumulating values into a vector where each value is generated from a given
 * generator generator
 */

namespace proptest {

namespace util {

template <typename T>
Generator<vector<T>> accumulateImpl(GenFunction<T> gen1, Function<GenFunction<T>(const T&)> gen2gen, size_t minSize,
                                    size_t maxSize)
{
    return interval<uint64_t>(minSize, maxSize).flatMap<vector<T>>([gen1, gen2gen, minSize](const uint64_t& size) {
        if (size == 0)
            return Generator<vector<T>>([](Random&) { return make_shrinkable<vector<T>>(); });
        return Generator<vector<T>>([gen1, gen2gen, size, minSize](Random& rand) {
            Shrinkable<T> shr = gen1(rand);
            auto shrVec = make_shrinkable<vector<Shrinkable<T>>>();
            auto& vec = shrVec.getMutableRef();
            vec.reserve(size);
            vec.push_back(shr);
            for (size_t i = 1; i < size; i++) {
                shr = gen2gen(shr.get())(rand);
                vec.push_back(shr);
            }
            return shrinkListLikeLength<vector, T>(shrVec, minSize)
                .andThen([](const Shrinkable<vector<Shrinkable<T>>>& parent) {
                    const vector<Shrinkable<T>>& shrVec_ = parent.getRef();
                    if (shrVec_.size() == 0)
                        return Stream<Shrinkable<vector<Shrinkable<T>>>>::empty();
                    const Shrinkable<T>& lastElemShr = shrVec_.back();
                    Stream<Shrinkable<T>> elemShrinks = lastElemShr.getShrinks();
                    if (elemShrinks.isEmpty())
                        return Stream<Shrinkable<vector<Shrinkable<T>>>>::empty();
                    return elemShrinks.template transform<Shrinkable<vector<Shrinkable<T>>>>(
                        [copy = shrVec_](const Shrinkable<T>& elem) mutable -> Shrinkable<vector<Shrinkable<T>>> {
                            copy[copy.size() - 1] = Shrinkable<T>(elem);
                            return make_shrinkable<vector<Shrinkable<T>>>(copy);
                        });
                })
                .template map<vector<T>>([](const vector<Shrinkable<T>>& shrVec) {
                    vector<T> valVec;
                    valVec.reserve(shrVec.size());
                    util::transform(shrVec.begin(), shrVec.end(), util::back_inserter(valVec),
                                    [](const Shrinkable<T>& shr) { return shr.get(); });
                    return valVec;
                });
        });
    });
}

}  // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for accumulating values into a vector where each value is generated from a given
 * generator generator
 * @tparam GEN1 Generator type of (Random&) -> Shrinkable<T>
 * @tparam GEN2GEN (T&) -> ((Random&) -> Shrinkable<T>) (Generator for T)
 * @param gen1 base generator for type T
 * @param gen2gen function that returns a generator for type T based on previously generated value of the same type
 * @param minSize minimum size of the aggregate
 * @param maxSize maximum size of the aggregate
 * @return vector of generated values of type T
 */
template <GenLike GEN1, typename GEN2GEN>
    requires FunctionLike<GEN2GEN, invoke_result_t<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type&>, typename invoke_result_t<GEN1, Random&>::type&>
decltype(auto) accumulate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    using RetType = invoke_result_t<GEN2GEN, T&>;
    GenFunction<T> funcGen1 = gen1;
    Function<RetType(const T&)> funcGen2Gen = gen2gen;
    return util::accumulateImpl<T>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest

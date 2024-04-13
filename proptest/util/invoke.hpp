#pragma once

#include "proptest/Generator.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/Random.hpp"

namespace proptest {
namespace util {

template <typename RET, typename...ARGS, GenLike<ARGS>...GENS>
decltype(auto) invokeExplicit(Function<RET(ARGS...)> func, GENS&&...gens) {
    constexpr size_t NumArgs = sizeof...(ARGS);
    using ArgTuple = tuple<ARGS...>;

    vector<AnyGenerator> genVec{generator(gens)...};
    genVec.reserve(NumArgs);

    Random rand(getCurrentTime());
    auto generateArgs = [&](auto index_sequence) {
        return genVec[index_sequence.value].template generate<tuple_element_t<index_sequence.value, ArgTuple>>(rand).get();
    };

    if constexpr(is_same_v<RET, void>) {
        util::Call<NumArgs>(func, generateArgs);
    }
    else {
        return util::Call<NumArgs>(func, generateArgs);
    }
}

template <typename RET, typename...ARGS, GenLike...GENS>
    requires (sizeof...(ARGS) >= sizeof...(GENS))
decltype(auto) invoke(Function<RET(ARGS...)> func, GENS&&...gens) {
    constexpr size_t NumArgs = sizeof...(ARGS);
    constexpr size_t NumGens = sizeof...(GENS);

    using ArgTuple = tuple<ARGS...>;
    auto genTuple = util::make_tuple(generator(gens)...);

    // prepare genVec
    vector<AnyGenerator> genVec;
    genVec.reserve(NumArgs);

    util::For<NumGens>([&](auto index_sequence) {
        genVec.push_back(get<index_sequence.value>(genTuple));
    });

    util::For<NumArgs-NumGens>([&genVec](auto index_sequence) {
        using T = tuple_element_t<NumGens + index_sequence.value, ArgTuple>;
        genVec.push_back(Arbi<T>());
    });

    // generate args and invoke
    Random rand(getCurrentTime());

    auto generateArgs = [&](auto index_sequence) {
        return genVec[index_sequence.value].template generate<tuple_element_t<index_sequence.value, ArgTuple>>(rand).get();
    };

    if constexpr(is_same_v<RET, void>) {
        util::Call<NumArgs>(func, generateArgs);
    }
    else {
        return util::Call<NumArgs>(func, generateArgs);
    }
}

} // namespace util
} // namespace proptest

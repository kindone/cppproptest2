#pragma once

#include "proptest/Generator.hpp"
#include "proptest/generator/tuple.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/util/function.hpp"

/**
 * @file construct.hpp
 * @brief Generator combinator for generating a type with a constructor
 */

namespace proptest {

namespace util {

template <typename CLASS, typename... ARGS>
Generator<CLASS> constructImpl(const shared_ptr<vector<AnyGenerator>>& genVec)
{
    constexpr size_t NumArgs = sizeof...(ARGS);

    if(genVec->size() != NumArgs)
        throw invalid_argument(__FILE__, __LINE__, "construct: genVec size does not match number of arguments");

    auto tupleGen = tupleOf<decay_t<ARGS>...>(genVec);
    return tupleGen.template map<CLASS>([](const tuple<decay_t<ARGS>...>& tup) {
        return util::Call<NumArgs>(+[](const decay_t<ARGS>&...args) {
            return CLASS(util::toCallableArg<ARGS>(args)...);
        }, [&tup](auto index_sequence) {
            return get<index_sequence.value>(tup);
        });
    });
}

} // namespace util

template <typename CLASS, typename... ARGS>
Generator<CLASS> construct()
{
    constexpr size_t NumArgs = sizeof...(ARGS);
    using ArgTuple = tuple<ARGS...>;

    auto genVec = util::make_shared<vector<AnyGenerator>>();
    genVec->reserve(NumArgs);

    // fill the rest with arbitraries
    util::For<NumArgs>([&genVec](auto index_sequence) {
        using T = decay_t<tuple_element_t<index_sequence.value, ArgTuple>>;
        genVec->push_back(Arbi<T>());
    });

    return util::constructImpl<CLASS, ARGS...>(genVec);
}

/**
 * @ingroup Combinators
 * @brief Generates a CLASS type by specifying target constructor's parameter types and their (optional) generators
 *
 * Usage:
 *
 * @code
 *      struct Point {
 *          Point() : x(0), y(0) {}
 *          Point(int x, int y) : x(x), y(y) {}
 *          int x;
 *          int y;
 *      };
 *      GenFunction<Point> objectGen = construct<Point>(); // calls Point()
 *      GenFunction<Point> objectGen2 = construct<Point, int, int>(nonNegative(), nonNegative()); // Point(int, int)
 *      GenFunction<Point> objectGen3 = construct<Point, int, int>(); // ints are generated using Arbi<int>
 * @endcode
 */
template <typename CLASS, typename... ARGS, GenLike ExplicitGen0, GenLike...ExplicitGens>
Generator<CLASS> construct(ExplicitGen0&& gen0, ExplicitGens&&... gens)
{
    constexpr size_t NumArgs = sizeof...(ARGS);
    constexpr size_t NumGens = 1 + sizeof...(ExplicitGens);
    using ArgTuple = tuple<ARGS...>;

    // check if explicit generators are compatible with the args
    util::For<sizeof...(ExplicitGens)>([](auto index_sequence) {
        using GenTup = tuple<ExplicitGen0, ExplicitGens...>;
        using T = decay_t<tuple_element_t<index_sequence.value, ArgTuple>>;
        using ExplicitGen = decay_t<tuple_element_t<index_sequence.value, GenTup>>;
        static_assert(is_same_v<typename invoke_result_t<ExplicitGen, Random&>::type, T>, "Supplied generator type does not match property argument type");
    });

    // prepare genVec
    auto genVec = util::make_shared<vector<AnyGenerator>, initializer_list<AnyGenerator>>(
        {generator(util::forward<ExplicitGen0>(gen0)), generator(util::forward<ExplicitGens>(gens))...});
    genVec->reserve(NumArgs);

    // fill the rest with arbitraries
    util::For<NumArgs-NumGens>([&genVec](auto index_sequence) {
        using T = decay_t<tuple_element_t<NumGens + index_sequence.value, ArgTuple>>;
        genVec->push_back(Arbi<T>());
    });

    return util::constructImpl<CLASS, ARGS...>(genVec);
}

} // namespace proptest

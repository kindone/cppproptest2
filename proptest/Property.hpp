#pragma once

#include "proptest/api.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/PropertyBase.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/matrix.hpp"

/**
 * @file Property.hpp
 * @brief Core API for `cppproptest` Property-based testing library
 */

namespace proptest {

/**
 * @brief Holder class for properties
 * @details When a property is defined using `proptest::property` or `proptest::forAll`, a `Property` object is created
 * to hold the property.
 */
template <typename... ARGS>
    requires (sizeof...(ARGS) > 0)
class Property {
public:
    static constexpr size_t Arity = sizeof...(ARGS);
    using Func = Function<bool(ARGS...)>;

private:
    using ArgTuple = tuple<decay_t<ARGS>...>;

public:
    Property(const Func& f, vector<AnyGenerator>&& gens) : func(f) {
        base = util::make_unique<PropertyBase>(util::move(gens));
        base->setCallFunction([func=this->func](const vector<Any>& anyVec) {
            return util::Call<Arity>(func, [&](auto index_sequence) {
                return anyVec[index_sequence.value].template getRef<tuple_element_t<index_sequence.value, ArgTuple>>();
            });
        });
        base->setCallFunctionWithShr([func=this->func](const vector<ShrinkableBase>& shrVec) {
            return util::Call<Arity>(func, [&](auto index_sequence) {
                return shrVec[index_sequence.value].getAny().template getRef<tuple_element_t<index_sequence.value, ArgTuple>>();
            });
        });
        base->setCallFunctionFromGen([func=this->func](Random& rand, const vector<AnyGenerator>& genVec) {
            return util::Call<Arity>(func, [&](auto index_sequence) {
                return genVec[index_sequence.value](rand).getAny().template getRef<tuple_element_t<index_sequence.value, ArgTuple>>();
            });
        });
        base->setWriteArgs([func=this->func](ostream& os, const vector<Any>& anyVec) {
            os << "{ " << Show<Any, tuple_element_t<0, ArgTuple>>(anyVec[0]);
            util::For<Arity-1>([&](auto index_sequence) {
                os << ", " << Show<Any, tuple_element_t<index_sequence.value+1, ArgTuple>>(anyVec[index_sequence.value+1]);
            });
            os << " }";
        });
        base->setWriteShrs([func=this->func](ostream& os, const vector<ShrinkableBase>& shrVec) {
            os << "{ " << Show<ShrinkableBase, tuple_element_t<0, ArgTuple>>(shrVec[0]);
            util::For<Arity-1>([&](auto index_sequence) {
                os << ", " << Show<ShrinkableBase, tuple_element_t<index_sequence.value+1, ArgTuple>>(shrVec[index_sequence.value+1]);
            });
            os << " }";
        });
    }

public:
    /**
     * @brief Sets the seed value for deterministic input generation
     *
     * @param s Seed in uint64_t type
     * @return Property& `Property` object itself for chaining
     */
    Property& setSeed(uint64_t s)
    {
        base->setSeed(s);
        return *this;
    }

    /**
     * @brief Sets the number of runs
     *
     * @param runs Number of runs
     * @return Property& `Property` object itself for chaining
     */
    Property& setNumRuns(uint32_t runs)
    {
        base->setNumRuns(runs);
        return *this;
    }

    /**
     * @brief Sets the startup function
     *
     * @param onStartup Invoked in each run before running the property function
     * @return Property& `Property` object itself for chaining
     */
    Property& setOnStartup(Function<void()> _onStartup)
    {
        base->setOnStartup(_onStartup);
        return *this;
    }

    /**
     * @brief Sets the cleanup function
     *
     * @param onCleanup Invoked in each run after running the property function.
     * @return Property& `Property` object itself for chaining
     */
    Property& setOnCleanup(Function<void()> _onCleanup)
    {
        base->setOnCleanup(_onCleanup);
        return *this;
    }

    /**
     * @brief Sets the timeout duration.
     *
     * @param durationMs maximum time to spend in the main loop, in milliseconds. Default is 0 meaning there is no timeout.
     * @return Property& `Property` object itself for chaining
     */
    Property& setMaxDurationMs(uint32_t durationMs)
    {
        base->setMaxDurationMs(durationMs);
        return *this;
    }

    /**
     * @brief Executes randomized tests for given property. If explicit generator arguments are omitted, utilizes
     * default generators (a.k.a. Arbitraries) instead
     *
     * Usage of explicit generators:
     * @code
     *  auto prop = property([](int a, float b) { ... });
     *  prop.forAll();                  // Arbitrary<int>, Arbitrary<float> is used to generate a and b
     *  prop.forAll(intGen);            // intGen, Arbitrary<float> is used to generate a and b
     *  prop.forAll(intGen, floatGen);  // intGen, floatGen is used to generate a and b
     * @endcode
     * @tparam ExplicitGens Explicitly given generator types
     * @param gens Variadic list of optional explicit generators (in same order as in definition of property arguments)
     * @return true if all the cases succeed
     * @return false if any one of the cases fails
     */
    template <typename... ExplicitGens>
    bool forAll(ExplicitGens&&... gens)
    {
        // combine explicit generators and implicit generators into a tuple by overriding implicit generators with explicit generators
        vector<AnyGenerator> curGenVec{generator(gens)...};
        curGenVec.reserve(Arity);
        return base->runForAll(curGenVec);
    }

    /**
     * @brief Executes single example-based test for given property.
     *
     * @param args Variadic list of explicit arguments (in same order as in definition of property arguments)
     * @return true if the case succeeds
     * @return false if the cases fails
     */
    bool example(const ARGS&... args)
    {
        vector<Any> values{args...};
        return base->exampleImpl(values);
    }

    /**
    * @brief Executes all input combinations in the Cartesian product of input lists
    *
    * Usage:
    * @code
        // As property is defined with two arguments: int and float, matrix() requires two arguments: int and float
    lists
        auto prop = property([](int i, float f) {
            // ...
        });

        // examples are auto-generated as Cartesian propduct of the lists:
        //   {1, 0.2f}, {1, 0.3f}, {2, 0.2f}, {2, 0.3f}, {3, 0.2f}, {3, 0.3f}
        prop.matrix({1,2,3}, {0.2f, 0.3f});
    * @endcode
    * @param lists Lists of valid arguments (types must be in same order as in parameters of the callable)
    */
    bool matrix(initializer_list<ARGS>&&... lists)
    {
        Function<bool(ARGS...)> test = [this](ARGS... args) {
            return example(args...);
        };
        return util::cartesianProduct(test, util::forward<initializer_list<ARGS>>(lists)...);
    }

private:

    Func func;
    unique_ptr<PropertyBase> base;
};

namespace util {

template <typename... ARGS>
decltype(auto) createProperty(Function<bool(ARGS...)> func, vector<AnyGenerator>&& genVec)
{
    return Property<ARGS...>(func, util::forward<decltype(genVec)>(genVec));
}

template <typename... ARGS>
decltype(auto) createProperty(Function<void(ARGS...)> func, vector<AnyGenerator>&& genVec)
{
    return Property<ARGS...>([func](const ARGS&...args) { func(args...); return true; }, util::forward<decltype(genVec)>(genVec));
}

}  // namespace util

/**
 * @brief creates a property object that can be used to run various property tests
 * @details @see Property
 * @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
 * `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
 * @tparam ExplicitGens Explicit generator callable types for each `ARG` in `(Random&) -> Shrinkable<ARG>`
 * @param callable passed as any callable such as `std::function`, functor object, function pointer
 * @param gens variadic list of generators for `ARG`s (optional if `Arbitrary<ARG>` is preferred)
 */
template <typename Callable, typename... ExplicitGens>
auto property(Callable&& callable, ExplicitGens&&... gens)
{
    // acquire full tuple of generators
    using FuncType = function_traits<Callable>::template function_type_with_signature<Function>;
    constexpr size_t NumArgs = FuncType::Arity;
    constexpr size_t NumGens = sizeof...(ExplicitGens);
    using ArgTuple = typename FuncType::ArgTuple;

    // prepare genVec
    vector<AnyGenerator> genVec{generator(gens)...};
    genVec.reserve(NumArgs);

    // fill the rest with arbitraries
    util::For<NumArgs-NumGens>([&genVec](auto index_sequence) {
        using T = decay_t<tuple_element_t<NumGens + index_sequence.value, ArgTuple>>;
        genVec.push_back(Arbi<T>());
    });

    // callable to Function
    return util::createProperty(FuncType(callable), util::move(genVec));
}
/**
 * @brief Immediately executes a randomized property test
 *
 * equivalent to `property(...).forAll()`
 *
 * @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
 * `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
 * @tparam ExplicitGens Explicit generator callable types for `ARG` in `(Random&) -> Shrinkable<ARG>`
 * @param callable passed as any callable such as `std::function`, functor object, function pointer
 * @param gens variadic list of generators for `ARG`s (optional if `Arbitrary<ARG>` is preferred)
 * @return true if all the cases succeed
 * @return false if any one of the cases fails
 */
template <typename Callable, typename... ExplicitGens>
bool forAll(Callable&& callable, ExplicitGens&&... gens)
{
    return property(util::forward<Callable>(callable), util::forward<ExplicitGens>(gens)...).forAll();
}

/**
* @brief Immediately executes all input combinations in the Cartesian product of input lists
*
* equivalent to `property(...).forAll()`
*
* Usage:
* @code
    // If property callable is defined with two arguments: int and float,
    // matrix() requires as arguments the callable and two more of types initializer_list<int> and initializer_list<float>.
lists
    // examples are auto-generated as Cartesian propduct of the lists:
    //   {1, 0.2f}, {1, 0.3f}, {2, 0.2f}, {2, 0.3f}, {3, 0.2f}, {3, 0.3f}
    proptest::matrix([](int i, float f) {
        // ...
    }, {1,2,3}, {0.2f, 0.3f});
* @endcode
*
* @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
* `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
* @tparam ARGS variadic types for callable and the initializer_lists
* @param callable passed as any callable such as `std::function`, functor object, function pointer
* @param lists Lists of valid arguments (types must be in same order as in parameters of the callable)
* @return true if all the cases succeed
* @return false if any one of the cases fails
*/

template <typename Callable, typename... ARGS>
bool matrix(Callable&& callable, initializer_list<ARGS>&&... lists)
{
    return property(util::forward<Callable>(callable)).matrix(util::forward<decltype(lists)>(lists)...);
}

#define EXPECT_FOR_ALL(CALLABLE, ...) EXPECT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))
#define ASSERT_FOR_ALL(CALLABLE, ...) ASSERT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))

}  // namespace proptest

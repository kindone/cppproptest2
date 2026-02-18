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
 * @brief Configuration struct for forAll() and Property::setConfig() with optional parameters
 * @details Allows configuration of property tests using designated initializers (C++20).
 * All fields are optional and can be set using designated initializer syntax.
 *
 * Usage:
 * @code
 * forAll([](int x) { ... }, {
 *     .seed = 123,
 *     .numRuns = 100,
 *     .maxDurationMs = 5000
 * });
 *
 * property([](int x) { ... })
 *     .setConfig({
 *         .seed = 123,
 *         .numRuns = 100
 *     })
 *     .forAll();
 * @endcode
 */
struct ForAllConfig {
    optional<uint64_t> seed;
    optional<uint32_t> numRuns;
    optional<uint32_t> maxDurationMs;
    optional<Function<void()>> onStartup;
    optional<Function<void()>> onCleanup;
};

// Forward declaration
template <typename... ARGS>
    requires (sizeof...(ARGS) > 0)
class Property;

namespace util {

/**
 * @brief Helper function to apply configuration to a Property object
 * @tparam ARGS Property argument types
 * @param prop Property object to configure
 * @param config Configuration to apply
 */
template <typename... ARGS>
void applyConfig(Property<ARGS...>& prop, const ForAllConfig& config)
{
    if (config.seed.has_value()) {
        prop.setSeed(config.seed.value());
    }
    if (config.numRuns.has_value()) {
        prop.setNumRuns(config.numRuns.value());
    }
    if (config.maxDurationMs.has_value()) {
        prop.setMaxDurationMs(config.maxDurationMs.value());
    }
    if (config.onStartup.has_value()) {
        prop.setOnStartup(config.onStartup.value());
    }
    if (config.onCleanup.has_value()) {
        prop.setOnCleanup(config.onCleanup.value());
    }
}

} // namespace util

/**
 * @brief Holder class for properties
 * @details When a property is defined using `proptest::property` or `proptest::forAll`, a `Property` object is created
 * to hold the property.
 */
template <typename... ARGS>
    requires (sizeof...(ARGS) > 0)
class Property final : public PropertyBase {
public:
    static constexpr size_t Arity = sizeof...(ARGS);
    using Func = Function<bool(ARGS...)>;

private:
    using ArgTuple = tuple<decay_t<ARGS>...>;
    using OrigArgTuple = tuple<ARGS...>;

public:
    Property(const Func& f, vector<AnyGenerator>&& gens) : PropertyBase(util::move(gens)), func(f) {}

public:
    /**
     * @brief Sets the seed value for deterministic input generation
     *
     * @param s Seed in uint64_t type
     * @return Property& `Property` object itself for chaining
     */
    Property& setSeed(uint64_t s)
    {
        seed = s;
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
        numRuns = runs;
        return *this;
    }

    /**
     * @brief Sets the startup function
     *
     * @param _onStartup Invoked in each run before running the property function
     * @return Property& `Property` object itself for chaining
     */
    Property& setOnStartup(Function<void()> _onStartup)
    {
        onStartup = _onStartup;
        return *this;
    }

    /**
     * @brief Sets the cleanup function
     *
     * @param _onCleanup Invoked in each run after running the property function.
     * @return Property& `Property` object itself for chaining
     */
    Property& setOnCleanup(Function<void()> _onCleanup)
    {
        onCleanup = _onCleanup;
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
        maxDurationMs = durationMs;
        return *this;
    }

    /**
     * @brief Sets multiple configuration options at once using designated initializers (C++20)
     *
     * Allows batch configuration of property tests using designated initializers.
     * Equivalent to chaining multiple setX() calls, but provides a cleaner syntax.
     *
     * Usage:
     * @code
     * property([](int x) { ... })
     *     .setConfig({
     *         .seed = 123,
     *         .numRuns = 100,
     *         .maxDurationMs = 5000
     *     })
     *     .forAll();
     * @endcode
     *
     * @param config Configuration options (using designated initializers)
     * @return Property& `Property` object itself for chaining
     */
    Property& setConfig(const ForAllConfig& config)
    {
        util::applyConfig(*this, config);
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
        constexpr size_t NumExplicitGens = sizeof...(gens);

        // check if explicit generators are compatible with the ARGS
        util::For<NumExplicitGens>([](auto index_sequence) {
            using GenTup = tuple<ExplicitGens...>;
            using T = decay_t<tuple_element_t<index_sequence.value, ArgTuple>>;
            using ExplicitGen = decay_t<tuple_element_t<index_sequence.value, GenTup>>;
            static_assert(is_same_v<typename invoke_result_t<ExplicitGen, Random&>::type, T>, "Supplied generator type does not match property argument type");
        });

        vector<AnyGenerator> curGenVec{generator(gens)...};
        curGenVec.reserve(genVec.size());
        for(size_t i = NumExplicitGens; i < genVec.size(); i++) {
            curGenVec.push_back(genVec[i]);
        }
        return runForAll(curGenVec);
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
        PropertyContext context;
        vector<Any> values{args...};
        return exampleImpl(values);
    }

public:
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

    virtual bool callFunction(const vector<Any>& anyVec) override {
        return util::Call<Arity>(func, [&](auto index_sequence) -> decltype(auto) {
            using ArgT = tuple_element_t<index_sequence.value, ArgTuple>;
            return anyVec[index_sequence.value].template getRef<ArgT>();
        });
    }

    virtual bool callFunction(const vector<ShrinkableBase>& shrVec) override {
        return util::Call<Arity>(func, [&](auto index_sequence) -> decltype(auto) {
            using ArgT = tuple_element_t<index_sequence.value, ArgTuple>;
            return shrVec[index_sequence.value].getAny().template getRef<ArgT>();
        });
    }

    virtual bool callFunctionFromGen(Random& rand, const vector<AnyGenerator>& genVec) override {
        vector<Any> argVec; // make sure generated values are intact when passed as reference to the func throughout the call
        return util::Call<Arity>(func, [&](auto index_sequence) -> decltype(auto) {
            using ArgT = tuple_element_t<index_sequence.value, ArgTuple>;
            Any arg = genVec[index_sequence.value](rand).getAny();
            argVec.push_back(arg);
            return arg.template getRef<ArgT>();
        });
    }

    virtual void writeArgs(ostream& os, const vector<ShrinkableBase>& shrVec) const override
    {
        os << "{ " << Show<ShrinkableBase, tuple_element_t<0, ArgTuple>>(shrVec[0]);
        util::For<Arity-1>([&](auto index_sequence) {
            os << ", " << Show<ShrinkableBase, tuple_element_t<index_sequence.value+1, ArgTuple>>(shrVec[index_sequence.value+1]);
        });
        os << " }";
    }

    virtual void writeArgs(ostream& os, const vector<Any>& anyVec) const override
    {
        os << "{ " << Show<Any, tuple_element_t<0, ArgTuple>>(anyVec[0]);
        util::For<Arity-1>([&](auto index_sequence) {
            os << ", " << Show<Any, tuple_element_t<index_sequence.value+1, ArgTuple>>(anyVec[index_sequence.value+1]);
        });
        os << " }";
    }

    Func func;
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
    using FuncType = typename function_traits<Callable>::template function_type_with_signature<Function>;
    constexpr size_t NumArgs = FuncType::Arity;
    constexpr size_t NumGens = sizeof...(ExplicitGens);
    using ArgTuple = typename FuncType::ArgTuple;

    // check if explicit generators are compatible with the callable
    util::For<sizeof...(ExplicitGens)>([](auto index_sequence) {
        using GenTup = tuple<ExplicitGens...>;
        using T = decay_t<tuple_element_t<index_sequence.value, ArgTuple>>;
        using ExplicitGen = decay_t<tuple_element_t<index_sequence.value, GenTup>>;
        static_assert(is_same_v<typename invoke_result_t<ExplicitGen, Random&>::type, T>, "Supplied generator type does not match property argument type");
    });

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
 * @brief Immediately executes a randomized property test with configuration
 *
 * Allows configuration using designated initializers (C++20).
 * Equivalent to `property(...).setSeed(...).setNumRuns(...).forAll()`
 *
 * Usage:
 * @code
 * forAll([](int x) { ... }, {
 *     .seed = 123,
 *     .numRuns = 100,
 *     .maxDurationMs = 5000
 * }, gen::int32());
 * @endcode
 *
 * @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
 * `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
 * @tparam ExplicitGens Explicit generator callable types for `ARG` in `(Random&) -> Shrinkable<ARG>`
 * @param callable passed as any callable such as `std::function`, functor object, function pointer
 * @param config configuration options (using designated initializers)
 * @param gens variadic list of generators for `ARG`s (optional if `Arbitrary<ARG>` is preferred)
 * @return true if all the cases succeed
 * @return false if any one of the cases fails
 */
template <typename Callable, typename... ExplicitGens>
bool forAll(Callable&& callable, const ForAllConfig& config, ExplicitGens&&... gens)
{
    auto prop = property(util::forward<Callable>(callable), util::forward<ExplicitGens>(gens)...);
    util::applyConfig(prop, config);
    return prop.forAll();
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
template <typename Callable, typename First, typename... Rest>
    requires (!is_same_v<decay_t<First>, ForAllConfig>)
bool forAll(Callable&& callable, First&& first, Rest&&... rest)
{
    return property(util::forward<Callable>(callable), util::forward<First>(first), util::forward<Rest>(rest)...).forAll();
}

/**
 * @brief Immediately executes a randomized property test (no generators)
 *
 * equivalent to `property(...).forAll()`
 *
 * @tparam Callable property callable type
 * @param callable passed as any callable such as `std::function`, functor object, function pointer
 * @return true if all the cases succeed
 * @return false if any one of the cases fails
 */
template <typename Callable>
bool forAll(Callable&& callable)
{
    return property(util::forward<Callable>(callable)).forAll();
}

/**
* @brief Immediately executes all input combinations in the Cartesian product of input lists
*
* equivalent to `property(...).matrix()`
*
* Usage:
* @code
    // If property callable is defined with two arguments: int and float,
    // matrix() requires as arguments the callable and two more of types initializer_list<int> and initializer_list<float>.
lists
    // examples are auto-generated as Cartesian product of the lists:
    //   {1, 0.2f}, {1, 0.3f}, {2, 0.2f}, {2, 0.3f}, {3, 0.2f}, {3, 0.3f}
    proptest::matrix([](int i, float f) {
        // ...
    }, {1,2,3}, {0.2f, 0.3f});
* @endcode
*
* @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
* `(ARGS...) -> void` (fail if exception is thrown, success otherwise)
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

#define EXPECT_FOR_ALL(...) EXPECT_TRUE(proptest::forAll(__VA_ARGS__))
#define ASSERT_FOR_ALL(...) ASSERT_TRUE(proptest::forAll(__VA_ARGS__))

}  // namespace proptest

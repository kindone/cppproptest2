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
#include "proptest/std/chrono.hpp"

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
class Property final : public PropertyBase {
public:
    static constexpr size_t Arity = sizeof...(ARGS);
    using Func = Function<bool(ARGS...)>;
    using GenVec = vector<AnyGenerator>;

private:
    using ArgTuple = tuple<decay_t<ARGS>...>;

public:
    Property(const Func& f, vector<AnyGenerator>&& gens) : func(f), genVec(util::move(gens)) {}

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
     * @param onStartup Invoked in each run before running the property function
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
     * @param onCleanup Invoked in each run after running the property function.
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
        auto valueTup = util::make_tuple(args...);
        try {
            try {
                try {
                    if (onStartup)
                        onStartup();
                    bool result = func(args...);
                    if (onCleanup)
                        onCleanup();
                    return result;
                } catch (const AssertFailed& e) {
                    throw PropertyFailed<tuple<ARGS...>>(e);
                }
            } catch (const Success&) {
                return true;
            } catch (const Discard&) {
                // silently discard combination
                cerr << "Discard is not supported for single run" << endl;
            }
        } catch (const PropertyFailedBase& e) {
            cerr << "example failed: " << e.what() << " (" << e.filename << ":" << e.lineno << ")" << endl;
            cerr << "  with args: " << Show<tuple<ARGS...>>(valueTup) << endl;
            return false;
        } catch (const exception& e) {
            // skip shrinking?
            cerr << "example failed by exception: " << e.what() << endl;
            cerr << "  with args: " << Show<tuple<ARGS...>>(valueTup) << endl;
            return false;
        }
        return false;
    }

private:

    bool runForAll(const GenVec& curGenVec)
    {
        Random rand(seed);
        Random savedRand(seed);
        cout << "random seed: " << seed << endl;
        PropertyContext ctx;
        auto startedTime = steady_clock::now();

        size_t i = 0;
        try {
            for (; i < numRuns; i++) {
                if(maxDurationMs != 0) {
                    auto currentTime = steady_clock::now();
                    if(duration_cast<util::milliseconds>(currentTime - startedTime).count() > maxDurationMs)
                    {
                        cout << "Timed out after " << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms , passed " << i << " tests" << endl;
                        ctx.printSummary();
                        return true;
                    }
                }
                bool pass = true;
                do {
                    pass = true;
                    try {
                        savedRand = rand;
                        if (onStartup)
                            onStartup();
                        // generate values
                        bool result = util::Call<Arity>(func, [&](auto index_sequence) {
                            return curGenVec[index_sequence.value](rand).getAny().template getRef<tuple_element_t<index_sequence.value, ArgTuple>>();
                        });

                        if (onCleanup)
                            onCleanup();
                        stringstream failures = ctx.flushFailures();
                        // failed expectations
                        if (failures.rdbuf()->in_avail()) {
                            cerr << "Falsifiable, after " << (i + 1) << " tests: ";
                            cerr << failures.str();
                            shrink(savedRand, curGenVec);
                            return false;
                        } else if (!result) {
                            cerr << "Falsifiable, after " << (i + 1) << " tests" << endl;
                            shrink(savedRand, curGenVec);
                            return false;
                        }
                        pass = true;
                    } catch (const Success&) {
                        pass = true;
                    } catch (const Discard&) {
                        // silently discard combination
                        pass = false;
                    }
                } while (!pass);
            }
        } catch (const AssertFailed& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                 << ")" << endl;
            // shrink
            shrink(savedRand, curGenVec);
            return false;
        } catch (const PropertyFailedBase& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                 << ")" << endl;
            // shrink
            shrink(savedRand, curGenVec);
            return false;
        } catch (const exception& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests - unhandled exception thrown: " << e.what() << endl;
            // shrink
            shrink(savedRand, curGenVec);
            return false;
        }

        cout << "OK, passed " << numRuns << " tests" << endl;
        ctx.printSummary();
        return true;
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
    bool test(const vector<ShrinkableAny>& curShrVec)
    {
        bool result = false;
        try {
            if (onStartup)
                onStartup();

            result = util::Call<Arity>(func, [&](auto index_sequence) {
                return curShrVec[index_sequence.value].getAny().template getRef<tuple_element_t<index_sequence.value, ArgTuple>>();
            });

            if (onCleanup)
                onCleanup();
        } catch (const AssertFailed&) {
            result = false;
            // cerr << "    assertion failed: " << e.what() << " (" << e.filename << ":"
            //           << e.lineno << ")" << endl;
        } catch (const exception&) {
            result = false;
        }
        return result;
    }

    template <typename Shrinks>
    static void printShrinks(const Shrinks& shrinks)
    {
        auto itr = shrinks.iterator();
        // cout << "    shrinks: " << endl;
        for (int i = 0; i < 4 && itr.hasNext(); i++) {
            auto& value = itr.next();
            cout << "    " << Show<decay_t<decltype(value)>>(value) << endl;
        }
    }

    void shrink(Random& savedRand, const GenVec& curGenVec)
    {
        // regenerate failed value tuple
        vector<ShrinkableAny> shrVec;
        vector<ShrinkableAny::StreamType> shrinksVec;
        shrVec.reserve(Arity);
        shrinksVec.reserve(Arity);
        for(size_t i = 0; i < Arity; i++) {
            auto shr = curGenVec[i](savedRand);
            shrVec.push_back(shr);
            shrinksVec.push_back(shr.getShrinks());
        };

        cout << "  with args: " << ShowShrVec{shrVec} << endl;

        util::For<Arity>([&](auto index_sequence) {
            constexpr size_t N = index_sequence.value;

            auto shrinks = shrinksVec[N];
            while (!shrinks.isEmpty()) {
                // printShrinks(shrinks);
                auto iter = shrinks.iterator<ShrinkableAny::StreamElementType>();
                bool shrinkFound = false;
                PropertyContext context;
                // keep trying until failure is reproduced
                while (iter.hasNext()) {
                    // get shrinkable
                    auto next = iter.next();
                    vector<ShrinkableAny> curShrVec = shrVec;
                    curShrVec[N] = next;
                    if (!test(curShrVec) || context.hasFailures()) {
                        shrinks = next.getShrinks();
                        shrVec[N] = next;
                        shrinkFound = true;
                        break;
                    }
                }
                if (shrinkFound) {
                    cout << "  shrinking found simpler failing arg " << N << ": " << ShowShrVec{shrVec} << endl;
                    if (context.hasFailures())
                        cout << "    by failed expectation: " << context.flushFailures(4).str() << endl;
                } else {
                    break;
                }
            }
        });

        cout << "  simplest args found by shrinking: " << ShowShrVec{shrVec} << endl;
    }

    struct ShowShrVec {
        friend ostream& operator<<(ostream& os, const ShowShrVec& show)
        {
            os << "{ " << Show<ShrinkableAny, tuple_element_t<0, ArgTuple>>(show.shrVec[0]);
            util::For<Arity-1>([&](auto index_sequence) {
                os << ", " << Show<ShrinkableAny, tuple_element_t<index_sequence.value+1, ArgTuple>>(show.shrVec[index_sequence.value+1]);
            });
            os << " }";
            return os;
        }
        const vector<ShrinkableAny>& shrVec;
    };

    Func func;
    vector<AnyGenerator> genVec;
};

namespace util {

template <typename Callable, typename... ARGS>
Function<bool(ARGS...)> functionWithBoolResultHelper(util::TypeList<ARGS...>, Callable&& callable)
{
    Function<void(const ARGS&...)> func = callable;
    return [func](const ARGS&... args) {
        func(args...);
        return true;
    };
}

template <class Callable>
decltype(auto) toFunctionWithBoolResult(Callable&& callable)
{
    using FuncType = function_traits<Callable>::template function_type_with_signature<Function>;
    using RetType = FuncType::RetType;
    if constexpr(is_same_v<RetType, void>) {
        // using FuncTypeBoolRet = function_traits<Callable>::template function_type_with_signature<Function, bool>;
        typename function_traits<Callable>::argument_type_list argument_type_list;
        return functionWithBoolResultHelper(argument_type_list, util::forward<Callable>(callable));
    }
    else
        return FuncType(callable);
}

template <typename... ARGS>
decltype(auto) createProperty(Function<bool(ARGS...)> func, vector<AnyGenerator>&& genVec)
{
    return Property<ARGS...>(func, util::forward<decltype(genVec)>(genVec));
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
    auto genTuple = util::make_tuple(generator(gens)...);
    vector<AnyGenerator> genVec;
    genVec.reserve(NumArgs);
    // fill with gens
    util::For<NumGens>([&](auto index_sequence) {
        genVec.push_back(get<index_sequence.value>(genTuple));
    });
    // fill the rest with arbitraries
    util::For<NumArgs-NumGens>([&genVec](auto index_sequence) {
        using T = decay_t<tuple_element_t<NumGens + index_sequence.value, ArgTuple>>;
        genVec.push_back(Arbi<T>());
    });

    // callable to Function
    auto func = util::toFunctionWithBoolResult(callable);
    return util::createProperty(func, util::move(genVec));
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
    return property(callable, gens...).forAll();
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
    return property(callable).matrix(util::forward<decltype(lists)>(lists)...);
}

#define EXPECT_FOR_ALL(CALLABLE, ...) EXPECT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))
#define ASSERT_FOR_ALL(CALLABLE, ...) ASSERT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))

}  // namespace proptest

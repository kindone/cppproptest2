#pragma once

#include "proptest/util/any.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/std/tuple.hpp"
#include "proptest/GenType.hpp"

namespace proptest {

/* Type-erased type */
struct GeneratorCommon {
    GeneratorCommon(const GeneratorCommon& other) : func(other.func) {}

    GeneratorCommon(Function1 _func) : func(_func) {}

    template <typename T>
    GeneratorCommon(const Generator<T>& gen) : func(gen.func) {}

    template <typename T>
    GeneratorCommon(const Arbi<T>& arbi) : func([arbi](Random& rand) { return arbi(rand); }) {}

    virtual ~GeneratorCommon();

    Function1 func;
};

template <typename T>
struct PROPTEST_API GeneratorBase
{
public:
    virtual ~GeneratorBase() {}

    virtual Shrinkable<T> operator()(Random& rand) const = 0;

    /**
     * @brief Higher-order function that returns an altered Generator for type `U`, based on this Generator's generated
     * value of type `T`
     *
     * Similar to \ref flatMap, whereas the function in `map` returns a `U` but the function in `flatMap` returns a
     * `Generator<U>`. This gives greater simplicity
     *
     * @tparam U Target type
     * @param mapper Function that takes a value of type `T` and returns a value of type `U`
     * @return Generator<U> Generator for type `U`
     */
    template <typename U>
    Generator<U> map(Function<U(T&)> mapper);

    /**
     * @brief Higher-order function that returns an altered Generator for type `U`,  based on this Generator's generated
     * value of type `T`
     *
     * @tparam F Callable type
     * @tparam U Target type
     * @param mapper Function that takes a value of type `T` and returns a value of type `U`
     * @return Generator<U> Generator for type `U`
     */
    template <invocable<T&> F>
    auto map(F&& mapper) -> Generator<invoke_result_t<F, T&>>
    {
        return map<invoke_result_t<F, T&>>(util::forward<F>(mapper));
    }

    /**
     * @brief Higher-order function that returns an altered Generator such that it restricts the values generated with
     * certain criteria
     *
     * @param criteria Function that takes a value `T` and returns whether to accept(true) or reject(false) the value
     * @return Generator<T> New Generator for type `T` that no longer includes the values that falsifies the criteria
     * function
     */
    template <typename Criteria>
    Generator<T> filter(Criteria&& criteria);

    /**
     * @brief Higher-order function that lets you produce a pair of dependent generators, by taking a generated result
     * from this Generator
     *
     * @tparam U Target type, deduced automatically from genFactory
     * @param genFactory Function that takes a value of type `T` that would be generated by this Generator and returns
     * the next Generator for type `U`
     * @return Generator<pair<T, U>> New Generator would generate a pair of `T` and `U` types, where the second argument
     * of `U` type has dependency to the first argument of type `T`
     */
    template <typename U>
    Generator<pair<T, U>> pairWith(Function<GenFunction<U>(T&)> genFactory);

    template <invocable<T&> FACTORY>
    decltype(auto) pairWith(FACTORY&& genFactory)
    {
        using GEN = invoke_result_t<FACTORY, T&>;
        using RetType = typename invoke_result_t<GEN, Random&>::type;
        return pairWith<RetType>(util::forward<FACTORY>(genFactory));
    }

    /**
     * @brief Higher-order function that lets you produce a tuple of dependent generators, by taking a generated result
     * from this Generator
     *
     * @tparam U Next type, deduced automatically from genFactory
     * @param genFactory Function that takes a value of type `T` that would be generated by this Generator and returns
     * the next Generator for type `U`
     * @return Generator<tuple<T, U>> If T is not a tuple, new Generator would generate a tuple of `T` and `U` types,
     * where the second argument of `U` type has dependency to the first argument of type `T`
     * @return Generator<tuple<T1,...Tn, U>> If T is a tuple<T1,...,Tn>, new Generator would generate a tuple of
     * `T1,...,Tn` and `U` types, where the last argument of `U` type has dependency to the second last argument of `Tn`
     * type
     */
    template <typename U>
    decltype(auto) tupleWith(Function<GenFunction<U>(T&)> genFactory);

    template <typename FACTORY>
    decltype(auto) tupleWith(FACTORY&& genFactory)
    {
        using U = typename invoke_result_t<invoke_result_t<FACTORY, T&>, Random&>::type;
        return tupleWith<U>(util::forward<FACTORY>(genFactory));
    }

    /**
     * @brief Higher-order function that transforms the Generator for type `T` into a generator for type `U`
     *
     * Similar to `.map`, whereas the function in `map` returns a `U` but the function in `flatMap` returns a
     * `Generator<U>`. This gives higher freedom
     *
     * @tparam U Target Type
     * @param genFactory Function that takes a value of type `T` and returns a generator for type `U`
     * @return Generator<U> Generator for type `U`
     */
    template <typename U>
    Generator<U> flatMap(Function<GenFunction<U>(T&)> genFactory);

    template <invocable<T&> FACTORY>
    decltype(auto) flatMap(FACTORY&& genFactory)
    {
        using U = typename invoke_result_t<invoke_result_t<FACTORY, T&>, Random&>::type;
        return flatMap<U>(util::forward<FACTORY>(genFactory));
    }

    virtual shared_ptr<GeneratorBase> clone() const = 0;

    virtual GenFunction<T> asGenFunction() {
        auto thisClone = clone();
        return [thisClone](Random& rand) -> Shrinkable<T> {
            return thisClone->operator()(rand);
        };
    }

    virtual Function1 asGenFunction1() {
        auto thisClone = clone();
        return [thisClone](Random& rand) -> Shrinkable<T> {
            return thisClone->operator()(rand);
        };
    }
};

/* Wrapping a Function */
template <typename T>
struct PROPTEST_API Generator : public GeneratorBase<T>
{
public:
    Generator(const Generator& other) : func(other.func) {}
    Generator(const GeneratorCommon& common) : func(common.func) {}
    Generator(const Function<Shrinkable<T>(Random&)>& _func) : func(_func) {}
    Generator(const Function1 _func) : func(_func) {}
    Generator(const Arbi<T>& arbi) : Generator(GeneratorCommon(arbi)) {}

    virtual Shrinkable<T> operator()(Random& rand) const override {
        return func(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>(true);
    }

    virtual shared_ptr<GeneratorBase<T>> clone() const override {
        return util::make_shared<Generator<T>>(*this);
    }

    GenFunction<T> asGenFunction() override {
        return GenFunction<T>([func = this->func](Random& rand) {
            return Shrinkable<T>(func(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>(true));
        });
    }

    Function1 asGenFunction1() override {
        return func;
    }

    Function1 func;
};

/**
 * @ingroup Generators
 * @brief Helper function to create \ref Generator<T> from generator functions to provide utility methods.
 */
template <GenLike GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(Function1(util::forward<GEN>(gen)));
}

template <GenLike GEN>
decltype(auto) asGenFunction(GEN&& gen)
{
    using retType = decltype(gen(declval<Random&>()));
    return static_cast<Function<retType(Random&)>>(gen);
}

struct PROPTEST_API AnyGenerator
{
    template <typename T>
    AnyGenerator(const Generator<T>& gen) : anyGen(gen)
    {
    }

    template <typename T>
    AnyGenerator(GenFunction<T> gen) : anyGen(gen) {}

    template <typename T>
    AnyGenerator(const Arbi<T>& arbi) : anyGen(arbi) {}

    ShrinkableAny operator()(Random& rand) const;

    template <typename T>
    Shrinkable<T> generate(Random& rand) {
        return anyGen(rand).getAny().getRef<T>();
    }

private:
    Function<ShrinkableAny(Random&)> anyGen;
};

// extern template struct PROPTEST_API Function<Shrinkable<Any>(Random&)>;

}  // namespace proptest


#include "proptest/GeneratorImpl.hpp"

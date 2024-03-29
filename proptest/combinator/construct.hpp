#pragma once
#include "proptest/Generator.hpp"
#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/chain.hpp"
#include "proptest/std/concepts.hpp"

/**
 * @file construct.hpp
 * @brief Generator combinator for generating a type with a constructor
 */
namespace proptest {

class Random;

template <class CLASS, typename... ARGTYPES>
class Construct;

namespace util {

template <typename TO, typename SHRINKABLE>
    requires (!is_lvalue_reference<TO>::value)
TO autoCast(SHRINKABLE&& shr)
{
    return shr.get();
}

template <typename TO, typename SHRINKABLE>
    requires(is_pointer<TO>::value)
TO autoCast(SHRINKABLE&& shr)
{
    return shr.getPtr();
}

template <typename TO, typename SHRINKABLE>
    requires(is_lvalue_reference<TO>::value)
TO autoCast(SHRINKABLE&& shr)
{
    return shr.getRef();
}

template <typename ToTuple, size_t N, typename FromTuple>
decltype(auto) autoCastTuple(FromTuple&& tuple)
{
    return autoCast<typename tuple_element<N, ToTuple>::type>(get<N>(tuple));
}

template <typename T, typename... ARGTYPES>
struct ConstructFunctor {
    ConstructFunctor(shared_ptr<Construct<T, ARGTYPES...>> ptr) : thisPtr(ptr) {}
    Shrinkable<T> operator()(Random& rand) { return thisPtr->operator()(rand); }
    shared_ptr<Construct<T,ARGTYPES...>> thisPtr;
};

}  // namespace util

template <class CLASS, typename... ARGTYPES>
class Construct : public GeneratorBase<CLASS> {
public:
    using ArgumentList = util::TypeList<ARGTYPES...>;
    using GenTuple = tuple<GenFunction<remove_reference_t<ARGTYPES>>...>;

    static constexpr auto Size = sizeof...(ARGTYPES);

    Construct() : genTup(util::createGenTuple(ArgumentList{})) {}

    Construct(GenTuple g) : genTup(g) {}

    Shrinkable<CLASS> operator()(Random& rand) override { return constructAccordingly(generateArgs(rand)); }

private:
    template <size_t... index>
    decltype(auto) generateArgsHelper(Random& rand, index_sequence<index...>)
    {
        return util::make_tuple(get<index>(genTup)(rand)...);
    }

    decltype(auto) generateArgs(Random& rand) { return generateArgsHelper(rand, make_index_sequence<Size>{}); }

    template <typename CastTuple, typename ValueTuple, size_t... index>
    static Shrinkable<CLASS> constructByTupleType(ValueTuple&& valueTuple, index_sequence<index...>)
    {
        return make_shrinkable<CLASS>(util::autoCastTuple<CastTuple, index>(util::forward<ValueTuple>(valueTuple))...);
    }

    template <typename ValueTuple>
    static Shrinkable<CLASS> constructAccordingly(ValueTuple&& valueTuple)
    {
        using ArgsAsTuple = tuple<ARGTYPES...>;
        constexpr auto arity = sizeof...(ARGTYPES);
        return constructByTupleType<ArgsAsTuple>(
            util::forward<ValueTuple>(valueTuple), make_index_sequence<arity>{}  // {0,1,2,3,...,N-1}
        );
    }

    GenTuple genTup;
};

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
template <typename CLASS, typename... ARGTYPES>
decltype(auto) construct()
{
    constexpr auto ImplicitSize = sizeof...(ARGTYPES);
    using ArgsAsTuple = tuple<decay_t<ARGTYPES>...>;
    auto implicits = util::createGenHelperListed<ArgsAsTuple>(make_index_sequence<ImplicitSize>{});
    return Construct<CLASS, ARGTYPES...>(implicits);
}

// some explicits
template <typename CLASS, typename... ARGTYPES, GenLike EXPGEN0, GenLike... EXPGENS>
Construct<CLASS, ARGTYPES...> construct(EXPGEN0&& gen0, EXPGENS&&... gens)
{
    constexpr auto ExplicitSize = sizeof...(EXPGENS) + 1;
    constexpr auto ImplicitSize = sizeof...(ARGTYPES) - ExplicitSize;
    using ArgsAsTuple = tuple<decay_t<ARGTYPES>...>;
    using explictTypes = tuple<invoke_result_t<EXPGEN0, Random&>::type, invoke_result_t<EXPGENS, Random&>::type...>;

    vector<AnyGenerator> explicitVec = {AnyGenerator(util::forward<EXPGEN0>(gen0)),
                                        AnyGenerator(util::forward<EXPGENS>(gens))...};
    vector<AnyGenerator> implicitVec = {AnyGenerator(Arbi<ARGS>())...};

    auto explicits = util::make_tuple(util::asGenFunction(gen0), util::asGenFunction(gens)...);
    auto implicits =
        util::createGenHelperListed<ArgsAsTuple>(util::addOffset<ExplicitSize>(make_index_sequence<ImplicitSize>{}));

    return Construct<CLASS, ARGTYPES...>(tuple_cat(explicits, implicits));
}

}  // namespace proptest

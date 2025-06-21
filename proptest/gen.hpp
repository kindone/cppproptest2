#pragma once

#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/combinator/combinators.hpp"
#include "proptest/generator/generators.hpp"

/**
 * @file gen.hpp
 * @brief Dedicated namespace for generator aliases and combinators
 *
 * This namespace provides convenient aliases for all built-in generators and combinators.
 * It serves as a unified interface for accessing generators and combinators with shorter,
 * more intuitive names.
 */

namespace proptest {

/**
 * @brief Dedicated namespace for generator aliases and combinators
 *
 * The `gen` namespace provides convenient aliases for all built-in generators and combinators.
 * This makes the API more intuitive and reduces verbosity when writing property-based tests.
 */
namespace gen {

// ============================================================================
// GENERATOR ALIASES
// ============================================================================

/**
 * @brief Alias for Arbitrary<T> - the default generator for type T
 */
template <typename T>
using arbitrary = Arbitrary<T>;

/**
 * @brief Alias for Arbi<T> - the default generator for type T
 */
template <typename T>
using arbi = Arbi<T>;

// ============================================================================
// BASIC TYPE GENERATORS
// ============================================================================

/**
 * @brief Boolean generator
 */
using boolean = Arbi<bool>;

/**
 * @brief Character generator
 */
using character = Arbi<char>;

/**
 * @brief Integer generators
 */
using int8 = Arbi<int8_t>;
using uint8 = Arbi<uint8_t>;
using int16 = Arbi<int16_t>;
using uint16 = Arbi<uint16_t>;
using int32 = Arbi<int32_t>;
using uint32 = Arbi<uint32_t>;
using int64 = Arbi<int64_t>;
using uint64 = Arbi<uint64_t>;

/**
 * @brief Floating point generators
 */
using float32 = Arbi<float>;
using float64 = Arbi<double>;

// ============================================================================
// STRING GENERATORS
// ============================================================================

/**
 * @brief String generators
 */
using string = Arbi<std::string>;
using utf8string = Arbi<UTF8String>;
using cesu8string = Arbi<CESU8String>;
using utf16bestring = Arbi<UTF16BEString>;
using utf16lestring = Arbi<UTF16LEString>;

// ============================================================================
// CONTAINER GENERATORS
// ============================================================================

/**
 * @brief Container generators
 */
template <typename T>
using vector = Arbi<std::vector<T>>;

template <typename T>
using list = Arbi<std::list<T>>;

template <typename T>
using set = Arbi<std::set<T>>;

template <typename Key, typename Value>
using map = Arbi<std::map<Key, Value>>;

template <typename T>
using optional = Arbi<std::optional<T>>;

template <typename T>
using shared_ptr = Arbi<std::shared_ptr<T>>;

// ============================================================================
// TUPLE AND PAIR GENERATORS
// ============================================================================

/**
 * @brief Tuple and pair generators
 */
template <typename T1, typename T2>
using pair = Arbi<std::pair<T1, T2>>;

template <typename... Ts>
using tuple = Arbi<std::tuple<Ts...>>;

// ============================================================================
// NUMERIC RANGE GENERATORS
// ============================================================================

/**
 * @brief Numeric range generators
 */
template <typename T>
auto natural(T max) -> decltype(proptest::natural<T>(max)) {
    return proptest::natural<T>(max);
}

template <typename T>
auto nonNegative(T max) -> decltype(proptest::nonNegative<T>(max)) {
    return proptest::nonNegative<T>(max);
}

template <typename T>
auto interval(T min, T max) -> decltype(proptest::interval<T>(min, max)) {
    return proptest::interval<T>(min, max);
}

template <typename T>
auto inRange(T from, T to) -> decltype(proptest::inRange<T>(from, to)) {
    return proptest::inRange<T>(from, to);
}

template <typename T>
auto integers(T start, T count) -> decltype(proptest::integers<T>(start, count)) {
    return proptest::integers<T>(start, count);
}

// ============================================================================
// COMBINATOR ALIASES
// ============================================================================

/**
 * @brief Constant value generator
 */
template <typename T>
auto just(T&& value) -> decltype(proptest::just<T>(std::forward<T>(value))) {
    return proptest::just<T>(std::forward<T>(value));
}

/**
 * @brief Element selection from values
 */
template <typename T, typename... Args>
auto elementOf(Args&&... args) -> decltype(proptest::elementOf<T>(std::forward<Args>(args)...)) {
    return proptest::elementOf<T>(std::forward<Args>(args)...);
}

/**
 * @brief Union of generators
 */
template <typename T, typename... Gens>
auto oneOf(Gens&&... gens) -> decltype(proptest::oneOf<T>(std::forward<Gens>(gens)...)) {
    return proptest::oneOf<T>(std::forward<Gens>(gens)...);
}

/**
 * @brief Alias for oneOf
 */
template <typename T, typename... Gens>
auto unionOf(Gens&&... gens) -> decltype(proptest::unionOf<T>(std::forward<Gens>(gens)...)) {
    return proptest::unionOf<T>(std::forward<Gens>(gens)...);
}

/**
 * @brief Object construction
 */
template <typename Class, typename... Args, typename... Gens>
auto construct(Gens&&... gens) -> decltype(proptest::construct<Class, Args...>(std::forward<Gens>(gens)...)) {
    return proptest::construct<Class, Args...>(std::forward<Gens>(gens)...);
}

/**
 * @brief Value transformation
 */
template <typename T, typename U, typename Gen>
auto transform(Gen&& gen, auto&& transformer) -> decltype(proptest::transform<T, U>(std::forward<Gen>(gen), std::forward<decltype(transformer)>(transformer))) {
    return proptest::transform<T, U>(std::forward<Gen>(gen), std::forward<decltype(transformer)>(transformer));
}

/**
 * @brief Value derivation (flat-map)
 */
template <typename T, typename U, typename Gen>
auto derive(Gen&& gen, auto&& genUGen) -> decltype(proptest::derive<T, U>(std::forward<Gen>(gen), std::forward<decltype(genUGen)>(genUGen))) {
    return proptest::derive<T, U>(std::forward<Gen>(gen), std::forward<decltype(genUGen)>(genUGen));
}

/**
 * @brief Value filtering
 */
template <typename T, typename Gen>
auto filter(Gen&& gen, auto&& predicate) -> decltype(proptest::filter<T>(std::forward<Gen>(gen), std::forward<decltype(predicate)>(predicate))) {
    return proptest::filter<T>(std::forward<Gen>(gen), std::forward<decltype(predicate)>(predicate));
}

/**
 * @brief Alias for filter
 */
template <typename T, typename Gen>
auto suchThat(Gen&& gen, auto&& predicate) -> decltype(proptest::suchThat<T>(std::forward<Gen>(gen), std::forward<decltype(predicate)>(predicate))) {
    return proptest::suchThat<T>(std::forward<Gen>(gen), std::forward<decltype(predicate)>(predicate));
}

/**
 * @brief Dependency between values
 */
template <typename T, typename U, typename GenT>
auto dependency(GenT&& genT, auto&& genUGen) -> decltype(proptest::dependency<T, U>(std::forward<GenT>(genT), std::forward<decltype(genUGen)>(genUGen))) {
    return proptest::dependency<T, U>(std::forward<GenT>(genT), std::forward<decltype(genUGen)>(genUGen));
}

/**
 * @brief Chain of generators
 */
template <typename T, typename GenT>
auto chain(GenT&& genT, auto&& genGen) -> decltype(proptest::chain<T>(std::forward<GenT>(genT), std::forward<decltype(genGen)>(genGen))) {
    return proptest::chain<T>(std::forward<GenT>(genT), std::forward<decltype(genGen)>(genGen));
}

/**
 * @brief Aggregation of values
 */
template <typename T, typename GenT>
auto aggregate(GenT&& genT, auto&& aggregator) -> decltype(proptest::aggregate<T>(std::forward<GenT>(genT), std::forward<decltype(aggregator)>(aggregator))) {
    return proptest::aggregate<T>(std::forward<GenT>(genT), std::forward<decltype(aggregator)>(aggregator));
}

/**
 * @brief Accumulation of values
 */
template <typename T, typename GenT>
auto accumulate(GenT&& genT, auto&& accumulator, size_t minSize, size_t maxSize) -> decltype(proptest::accumulate<T>(std::forward<GenT>(genT), std::forward<decltype(accumulator)>(accumulator), minSize, maxSize)) {
    return proptest::accumulate<T>(std::forward<GenT>(genT), std::forward<decltype(accumulator)>(accumulator), minSize, maxSize);
}

/**
 * @brief Lazy evaluation
 */
template <typename T>
auto lazy(auto&& func) -> decltype(proptest::lazy<T>(std::forward<decltype(func)>(func))) {
    return proptest::lazy<T>(std::forward<decltype(func)>(func));
}

/**
 * @brief Reference wrapper
 */
template <typename T>
auto reference(T& ref) -> decltype(proptest::reference<T>(ref)) {
    return proptest::reference<T>(ref);
}

// ============================================================================
// INTERVAL GENERATORS
// ============================================================================

/**
 * @brief Multiple intervals for signed integers
 */
inline auto intervals(proptest::initializer_list<proptest::Interval> interval_list) -> decltype(proptest::intervals(interval_list)) {
    return proptest::intervals(interval_list);
}

/**
 * @brief Multiple intervals for unsigned integers
 */
inline auto uintervals(proptest::initializer_list<proptest::UInterval> interval_list) -> decltype(proptest::uintervals(interval_list)) {
    return proptest::uintervals(interval_list);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Weighted generator decorator
 */
template <typename T>
auto weighted(GenFunction<T> gen, double weight) -> decltype(proptest::weightedGen<T>(gen, weight)) {
    return proptest::weightedGen<T>(gen, weight);
}

/**
 * @brief Weighted value decorator
 */
template <typename T>
auto weightedVal(T value, double weight) -> decltype(proptest::weightedVal<T>(value, weight)) {
    return proptest::weightedVal<T>(value, weight);
}

} // namespace gen

} // namespace proptest
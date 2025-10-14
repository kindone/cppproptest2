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
 * It serves as a unified interface for accessing generators and combinators
 */

namespace proptest {

/**
 * @brief Dedicated namespace for generator aliases and combinators
 *
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
using integer = Arbi<int>;

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
using string = Arbi<::proptest::string>;
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
using vector = Arbi<::proptest::vector<T>>;

template <typename T>
using list = Arbi<::proptest::list<T>>;

template <typename T>
using set = Arbi<::proptest::set<T>>;

template <typename Key, typename Value>
using map = Arbi<::proptest::map<Key, Value>>;

template <typename T>
using optional = Arbi<::proptest::optional<T>>;

template <typename T>
using shared_ptr = Arbi<::proptest::shared_ptr<T>>;

// template <typename T1, typename T2>
// using pair = Arbi<::proptest::pair<T1, T2>>;



} // namespace gen

} // namespace proptest

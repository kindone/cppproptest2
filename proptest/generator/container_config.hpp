#pragma once

#include "proptest/GenType.hpp"
#include "proptest/std/optional.hpp"

/**
 * @file container_config.hpp
 * @brief Configuration structs for container generators
 */
namespace proptest {

namespace util {

/**
 * @brief Configuration for list-like container generators (Arbi<vector<T>>, Arbi<list<T>>, Arbi<set<T>>, etc.).
 * All fields are optional; unspecified fields use defaults.
 * Use with designated initializers: Arbi<vector<int>>({.minSize = 5, .maxSize = 20})
 */
template <typename T>
struct ContainerGenConfig {
    optional<GenFunction<T>> elemGen = nullopt;
    optional<size_t> minSize = nullopt;
    optional<size_t> maxSize = nullopt;
};

/**
 * @brief Configuration for map generators (Arbi<map<K,V>>).
 * All fields are optional; unspecified fields use defaults.
 * Use with designated initializers: Arbi<map<int,int>>({.keyGen = gen::int32(), .valueGen = gen::int32(), .minSize = 5, .maxSize = 20})
 */
template <typename Key, typename Value>
struct MapGenConfig {
    optional<GenFunction<Key>> keyGen = nullopt;
    optional<GenFunction<Value>> valueGen = nullopt;
    optional<size_t> minSize = nullopt;
    optional<size_t> maxSize = nullopt;
};

}  // namespace util

}  // namespace proptest

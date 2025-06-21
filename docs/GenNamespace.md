# The `gen` Namespace

The `gen` namespace provides convenient aliases for all built-in generators and combinators in `cppproptest`. This makes the API more intuitive and reduces verbosity when writing property-based tests.

## Overview

The `gen` namespace is designed to provide a unified, user-friendly interface for accessing generators and combinators. Instead of writing verbose names like `Arbi<int>()` or `proptest::interval<int>(1, 100)`, you can use shorter, more intuitive names like `gen::int32()` or `gen::interval<int>(1, 100)`.

## Basic Usage

To use the `gen` namespace, simply include the main header and use the `gen::` prefix:

```cpp
#include "proptest/proptest.hpp"

using namespace proptest;

// Use gen namespace for cleaner syntax
auto intGen = gen::int32();
auto stringGen = gen::string();
auto vectorGen = gen::vector<int>();
```

## Generator Aliases

### Basic Type Generators

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::boolean` | `Arbi<bool>` | Boolean values |
| `gen::character` | `Arbi<char>` | Character values |
| `gen::int8` | `Arbi<int8_t>` | 8-bit signed integers |
| `gen::uint8` | `Arbi<uint8_t>` | 8-bit unsigned integers |
| `gen::int16` | `Arbi<int16_t>` | 16-bit signed integers |
| `gen::uint16` | `Arbi<uint16_t>` | 16-bit unsigned integers |
| `gen::int32` | `Arbi<int32_t>` | 32-bit signed integers |
| `gen::uint32` | `Arbi<uint32_t>` | 32-bit unsigned integers |
| `gen::int64` | `Arbi<int64_t>` | 64-bit signed integers |
| `gen::uint64` | `Arbi<uint64_t>` | 64-bit unsigned integers |
| `gen::float32` | `Arbi<float>` | 32-bit floating point |
| `gen::float64` | `Arbi<double>` | 64-bit floating point |

### String Generators

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::string` | `Arbi<std::string>` | ASCII strings |
| `gen::utf8string` | `Arbi<UTF8String>` | UTF-8 strings |
| `gen::cesu8string` | `Arbi<CESU8String>` | CESU-8 strings |
| `gen::utf16bestring` | `Arbi<UTF16BEString>` | UTF-16 big-endian strings |
| `gen::utf16lestring` | `Arbi<UTF16LEString>` | UTF-16 little-endian strings |

### Container Generators

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::vector<T>` | `Arbi<std::vector<T>>` | Vectors |
| `gen::list<T>` | `Arbi<std::list<T>>` | Lists |
| `gen::set<T>` | `Arbi<std::set<T>>` | Sets |
| `gen::map<K,V>` | `Arbi<std::map<K,V>>` | Maps |
| `gen::optional<T>` | `Arbi<std::optional<T>>` | Optional values |
| `gen::shared_ptr<T>` | `Arbi<std::shared_ptr<T>>` | Shared pointers |

### Tuple and Pair Generators

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::pair<T1,T2>` | `Arbi<std::pair<T1,T2>>` | Pairs |
| `gen::tuple<Ts...>` | `Arbi<std::tuple<Ts...>>` | Tuples |

## Numeric Range Generators

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::natural<T>(max)` | `proptest::natural<T>(max)` | Positive integers ≤ max |
| `gen::nonNegative<T>(max)` | `proptest::nonNegative<T>(max)` | Non-negative integers ≤ max |
| `gen::interval<T>(min, max)` | `proptest::interval<T>(min, max)` | Integers in [min, max] |
| `gen::inRange<T>(from, to)` | `proptest::inRange<T>(from, to)` | Integers in [from, to) |
| `gen::integers<T>(start, count)` | `proptest::integers<T>(start, count)` | Integers in [start, start+count) |

## Combinator Functions

| Function | Original | Description |
|----------|----------|-------------|
| `gen::just<T>(value)` | `proptest::just<T>(value)` | Constant value |
| `gen::elementOf<T>(vals...)` | `proptest::elementOf<T>(vals...)` | Random selection from values |
| `gen::oneOf<T>(gens...)` | `proptest::oneOf<T>(gens...)` | Union of generators |
| `gen::unionOf<T>(gens...)` | `proptest::unionOf<T>(gens...)` | Alias for oneOf |
| `gen::construct<Class,Args...>(gens...)` | `proptest::construct<Class,Args...>(gens...)` | Object construction |
| `gen::transform<T,U>(gen, func)` | `proptest::transform<T,U>(gen, func)` | Value transformation |
| `gen::derive<T,U>(gen, genGen)` | `proptest::derive<T,U>(gen, genGen)` | Value derivation (flat-map) |
| `gen::filter<T>(gen, predicate)` | `proptest::filter<T>(gen, predicate)` | Value filtering |
| `gen::suchThat<T>(gen, predicate)` | `proptest::suchThat<T>(gen, predicate)` | Alias for filter |
| `gen::dependency<T,U>(genT, genUGen)` | `proptest::dependency<T,U>(genT, genUGen)` | Dependency between values |
| `gen::chain<T>(genT, genGen)` | `proptest::chain<T>(genT, genGen)` | Chain of generators |
| `gen::aggregate<T>(genT, aggregator)` | `proptest::aggregate<T>(genT, aggregator)` | Aggregation of values |
| `gen::accumulate<T>(genT, accumulator, min, max)` | `proptest::accumulate<T>(genT, accumulator, min, max)` | Accumulation of values |
| `gen::lazy<T>(func)` | `proptest::lazy<T>(func)` | Lazy evaluation |
| `gen::reference<T>(ref)` | `proptest::reference<T>(ref)` | Reference wrapper |

## Interval Generators

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::intervals({Interval(min1,max1), ...})` | `proptest::intervals({...})` | Multiple intervals for signed integers |
| `gen::uintervals({UInterval(min1,max1), ...})` | `proptest::uintervals({...})` | Multiple intervals for unsigned integers |

## Utility Functions

| Alias | Original | Description |
|-------|----------|-------------|
| `gen::weighted<T>(gen, weight)` | `proptest::weightedGen<T>(gen, weight)` | Weighted generator decorator |
| `gen::weightedVal<T>(value, weight)` | `proptest::weightedVal<T>(value, weight)` | Weighted value decorator |

## Examples

### Basic Generators

```cpp
// Basic types
auto boolGen = gen::boolean();
auto intGen = gen::int32();
auto stringGen = gen::string();

// Numeric ranges - these are function calls, not type aliases
auto smallIntGen = gen::interval(1, 100);
auto positiveIntGen = gen::natural(1000);
auto nonNegIntGen = gen::nonNegative(500);
```

### Container Generators

```cpp
// Simple containers
auto intVectorGen = gen::vector<int>();
auto stringListGen = gen::list<std::string>();
auto intSetGen = gen::set<int>();
auto stringIntMapGen = gen::map<std::string, int>();

// Nested containers
auto vectorOfMapsGen = gen::vector<gen::map<std::string, int>>();
auto mapOfVectorsGen = gen::map<std::string, gen::vector<int>>();
```

### String Generators with Custom Elements

```cpp
// Custom character ranges
auto uppercaseGen = gen::string(gen::interval<char>('A', 'Z'));
auto digitGen = gen::string(gen::interval<char>('0', '9'));
auto alphanumericGen = gen::string(gen::unionOf<char>(
    gen::interval<char>('A', 'Z'),
    gen::interval<char>('a', 'z'),
    gen::interval<char>('0', '9')
));
```

### Combinators

```cpp
// Constants and choices
auto constantGen = gen::just(42);
auto choiceGen = gen::elementOf<int>(1, 3, 5, 7, 11, 13, 17, 19);
auto unionGen = gen::oneOf<int>(
    gen::interval<int>(1, 10),
    gen::interval<int>(100, 110)
);

// Object construction
struct Point { Point(int x, int y) : x(x), y(y) {} int x, y; };
auto pointGen = gen::construct<Point, int, int>(
    gen::interval<int>(-10, 10),
    gen::interval<int>(-10, 10)
);

// Filtering and transformation
auto evenGen = gen::filter<int>(gen::int32(), [](int n) { return n % 2 == 0; });
auto positiveGen = gen::suchThat<int>(gen::int32(), [](int n) { return n > 0; });
auto stringFromIntGen = gen::transform<int, std::string>(
    gen::int32(),
    [](int n) { return std::to_string(n); }
);
```

### Dependencies and Complex Generators

```cpp
// Size-dependent vector
auto sizeAndVectorGen = gen::dependency<int, std::vector<int>>(
    gen::interval<int>(1, 5),
    [](int size) { return gen::vector<int>().setSize(size); }
);

// Complex nested structure
auto complexGen = gen::vector<gen::map<std::string, gen::vector<int>>>();
```

### Property Tests

```cpp
// Simple property test
forAll([](int x, int y) {
    return x + y == y + x;  // Commutativity of addition
}, gen::interval<int>(-100, 100), gen::interval<int>(-100, 100));

// Property test with custom generators
forAll([](std::vector<int> vec) {
    if (vec.empty()) return true;
    auto sorted = vec;
    std::sort(sorted.begin(), sorted.end());
    return sorted.size() == vec.size();
}, gen::vector<int>().setSize(0, 10));
```

## Benefits

1. **Shorter, more readable code**: `gen::int32()` vs `Arbi<int32_t>()`
2. **Consistent naming**: All generators follow the same naming pattern
3. **Intuitive names**: `gen::natural<int>(100)` is more descriptive than `proptest::natural<int>(100)`
4. **Namespace organization**: All generator-related functionality is grouped under `gen::`
5. **Direct implementations**: Combinators are implemented directly in the `gen` namespace for better performance and clarity
6. **Backward compatibility**: Original names still work, `gen` namespace is additive

## Migration

The `gen` namespace is designed to be additive and doesn't break existing code. You can:

1. Continue using the original names: `Arbi<int>()`, `proptest::interval<int>(1, 100)`
2. Gradually migrate to the new names: `gen::int32()`, `gen::interval<int>(1, 100)`
3. Mix both styles in the same codebase

The `gen` namespace provides a more modern, user-friendly interface while maintaining full compatibility with the existing API. 
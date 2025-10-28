# Generators

## Generators in Property-Based Testing

Property-based testing promotes the concept of **input domain** of a property. In property-based testing, **generators** are the means for representing input domains. A _generator_ is basically a function that generates random values. It defines the constraints of the generated random values in its body. Even a simple `forAll()` call depends on generators under the hood. Let's see following example:

```cpp
forAll([](int age, std::string name) {
});
```

This `forAll()` call takes a function with parameters of types `int` and `std::string`. This function is the *property function*. If no additional specification is given on how to generate the values for `age` and `name` as in this example, the parameter types are identified to invoke the default generators for those types. In this case, it calls the default generators for `int` and `std::string` types. Those default generators are called the *arbitraries*. You can access the arbitraries of a type `T` with `Arbitrary<T>` or `Arbi<T>`. This code is actually equivalent to:

```cpp
forAll([](int age, std::string name) {
}, Arbitrary<int>(), Arbitrary<std::string>());
```

Notice the extra arguments `Arbitrary<int>()` and `Arbitrary<std::string>()` in the `forAll()` call. As you can see, `forAll()` actually requires some information on how to generate the values for the parameter types. Some of the commonly used data types have default generators defined in `cppproptest`.

## Built-in Arbitraries and Their Generator Aliases

For the `Arbitrary<int>()` in above example, we have an alias - `gen::int32()`. `cppproptest` provides such built-in arbitraries for common data types and useful aliases to those arbitraries in the `gen` namespace. Thus, you can also write above example as following:

```cpp
forAll([](int age, std::string name) {
}, gen::int32(), gen::string());
```

Here is the complete list of the built-in arbitraries and their aliases. You can also refer to [Arbitrary](Arbitrary.md) page for more about arbitraries, including their configurable options and how they make up as default generators for a data type.

### Basic Type Generators

| Alias | Arbitrary | Description |
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

```cpp
// Basic type generators
auto boolGen = gen::boolean(0.7); // true probability: defaults to 0.5 if not given
auto charGen = gen::character();
auto intGen = gen::int32();
auto uintGen = gen::uint32();
auto floatGen = gen::float32();
auto doubleGen = gen::float64();

// Usage in forAll
forAll([](bool b, char c, int32_t i, float f) {
    // Property test with basic types
}, gen::boolean(), gen::character(), gen::int32(), gen::float32());
```

### String Generators

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| `gen::string` | `Arbi<std::string>` | ASCII strings |
| `gen::utf8string` | `Arbi<UTF8String>` | UTF-8 strings |
| `gen::utf16bestring` | `Arbi<UTF16BEString>` | UTF-16 big-endian strings |
| `gen::utf16lestring` | `Arbi<UTF16LEString>` | UTF-16 little-endian strings |
| `gen::cesu8string` | `Arbi<CESU8String>` | CESU-8 strings |

```cpp
// String generators with default character sets
auto asciiStringGen = gen::string(); // defaults to printable ASCII, a.k.a. in [0x01, 0x7f]
auto utf8StringGen = gen::utf8string(); // defaults to all UTF-8 characters
auto utf16beStringGen = gen::utf16bestring();
auto utf16leStringGen = gen::utf16lestring();
auto cesu8StringGen = gen::cesu8string();

// Custom string with specific character ranges
auto uppercaseGen = gen::string(gen::interval<char>('A', 'Z'));
auto digitGen = gen::string(gen::interval<char>('0', '9'));
auto alphanumericGen = gen::utf8string(gen::unionOf<uint32_t>(
    gen::interval<uint32_t>('A', 'Z'),
    gen::interval<uint32_t>('a', 'z'),
    gen::interval<uint32_t>('0', '9')
));

// Usage in forAll
forAll([](std::string name, UTF8String alphanumeric) {
    // Property test with strings
}, gen::string(), alphanumericGen);
```

### Container Generators

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| `gen::vector<T>` | `Arbi<std::vector<T>>` | Vectors |
| `gen::list<T>` | `Arbi<std::list<T>>` | Lists |
| `gen::set<T>` | `Arbi<std::set<T>>` | Sets |
| `gen::map<K,V>` | `Arbi<std::map<K,V>>` | Maps |
| `gen::optional<T>` | `Arbi<std::optional<T>>` | Optional values |
| `gen::shared_ptr<T>` | `Arbi<std::shared_ptr<T>>` | Shared pointers |

Container generators often take size ranges and an element generator (uses default generator for the type if not given) as arguments.

```cpp
// Basic container generators
auto intVectorGen = gen::vector<int>();
auto stringListGen = gen::list<std::string>();
auto intSetGen = gen::set<int>();
auto stringIntMapGen = gen::map<std::string, int>();
auto optionalIntGen = gen::optional<int>();
auto sharedPtrIntGen = gen::shared_ptr<int>();

// Containers with custom element generators
auto smallIntVectorGen = gen::vector<int>(gen::interval<int>(1, 100));
auto uppercaseStringListGen = gen::list<std::string>(gen::string(gen::interval<char>('A', 'Z')));

// Containers with size constraints
auto fixedSizeVectorGen = gen::vector<int>();
fixedSizeVectorGen.setSize(5); // Always generates vectors of size 5

auto boundedVectorGen = gen::vector<int>();
boundedVectorGen.setMinSize(1);
boundedVectorGen.setMaxSize(10); // Generates vectors of size 1-10

auto vectorGen1 = gen::vector<int>(5, 5); // you can provide the size range directly to constructor
auto vectorGen2 = gen::vector<int>(gen::interval(1,10), 5, 5); // element generator + size range at once

// Map with custom key and value generators
auto customMapGen = gen::map<std::string, int>();
customMapGen.setKeyGen(gen::string(gen::interval<char>('a', 'z')));
customMapGen.setElemGen(gen::interval<int>(0, 100));

// Nested containers
auto vectorOfMapsGen = gen::vector<gen::map<std::string, int>>();
auto mapOfVectorsGen = gen::map<std::string, gen::vector<int>>();

// Usage in forAll
forAll([](std::vector<int> numbers, std::map<std::string, int> data) {
    // Property test with containers
}, gen::vector<int>(), gen::map<std::string, int>());
```

### Tuple and Pair Generators

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| `gen::pair<T1,T2>` | `Arbi<std::pair<T1,T2>>` | Pairs |
| `gen::tuple<Ts...>` | `Arbi<std::tuple<Ts...>>` | Tuples |
| `gen::pair` |  | Pairs (type params inferred) |
| `gen::tuple` |  | Tuples (type params inferred)|

`gen::pair` and `gen::tuple` generators take optional element generators as arguments. The generators are optional only if the types have `Arbitrary` defined.

`gen::pair` and `gen::tuple` each has a convenient wrapper that allows omitting type parameters. You can see the subtle differences below:

```cpp
// Basic pair and tuple generators
auto intStringPairGen = gen::pair<int, std::string>(/* generators optional */);
auto intBoolStringTupleGen = gen::tuple<int, bool, std::string>(/* generators optional */);

// Pairs and tuples with custom element generators
auto smallIntUppercaseStringPairGen = gen::pair(
    gen::interval<int>(1, 100),
    gen::string(gen::interval<char>('A', 'Z'))
); // generator for pair<int, string>

auto customTupleGen = gen::tuple(
    gen::interval<int>(-50, 50),
    gen::boolean(),
    gen::string(gen::interval<char>('a', 'z'))
); // generator for tuple<int, bool, string>

// Usage in forAll
forAll([](std::pair<int, std::string> data, std::tuple<int, bool, std::string> info) {
    // Property test with pairs and tuples
}, gen::pair<int, std::string>(), gen::tuple<int, bool, std::string>());
```

## Utility Numeric Range Generators

There are also utility integer generators based on numeric range for everyday use.

| Alias | Description |
|-------|-------------|
| `gen::natural<T>(max)` | Positive integers ≤ max |
| `gen::nonNegative<T>(max)` | Non-negative integers ≤ max |
| `gen::interval<T>(min, max)` | Integers in [min, max] |
| `gen::inRange<T>(from, to)` | Integers in [from, to) |
| `gen::integers<T>(start, count)` | Integers in [start, start+count) |

```cpp
// Numeric range generators - these are function calls, not type aliases
auto smallIntGen = gen::interval<int>(1, 100); // generates [1, 100]
auto positiveIntGen = gen::natural<int>(1000); // generates [1, 1000]
auto nonNegIntGen = gen::nonNegative<int>(500); // generates [0,500]
auto rangeIntGen = gen::inRange<int>(0, 100); // generates [0, 100)
auto sequenceIntGen = gen::integers<int>(10, 20); // generates [10, 29]

// Different integer types
auto smallInt8Gen = gen::interval<int8_t>(-128, 127);
auto uint16Gen = gen::natural<uint16_t>(65535);
auto int64Gen = gen::interval<int64_t>(-1000000, 1000000);

// Usage in forAll
forAll([](int small, int positive, int nonNeg) {
    // Property test with range-constrained integers
}, gen::interval<int>(1, 100), gen::natural<int>(1000), gen::nonNegative<int>(500));
```

## Interval Generators

Interval generators are basically shorthand equivalents for combining multiple numeric ranges (e.g. `gen::interval`) with a `gen::oneOf` (`gen::unionOf`) combinator.

| Alias | Description |
|-------|-------------|
| `gen::intervals({Interval(min1,max1), ...})` | Multiple intervals for signed integers |
| `gen::uintervals({UInterval(min1,max1), ...})` | Multiple intervals for unsigned integers |

```cpp
// Multiple interval generators
auto multiRangeIntGen = gen::intervals({
    gen::Interval(-100, -1),    // Negative numbers
    gen::Interval(1, 100)       // Positive numbers
});

auto multiRangeUintGen = gen::uintervals({
    gen::UInterval(0, 9),       // Single digits
    gen::UInterval(100, 999)    // Three digits
});

// Usage in forAll
forAll([](int64_t multiRange) {
    // Property test with multi-interval integers
}, gen::intervals({gen::Interval(-100, -1), gen::Interval(1, 100)}));
```

## Access to an Arbitrary

While arbitraries (default generator) for type `T` can be accessed using `proptest::Arbitary<T>`, a proxy in `gen` namespace is also provided. You may find it useful if type should be parameterized:

| Function | Description |
|----------|-------------|
| `gen::arbitrary<T>(...)` | alias to `proptest::Arbitrary<T>` |


## Combinator Functions

The second part of `gen` namespace contains all the combinators provided by `cppproptest`.

| Function | Description |
|----------|-------------|
| `gen::just<T>(value)` | Constant value |
| `gen::elementOf<T>(vals...)` | Random selection from values |
| `gen::oneOf<T>(gens...)` | Union of generators |
| `gen::unionOf<T>(gens...)` | Alias for `gen::oneOf` |
| `gen::construct<Class,Args...>(gens...)` | Object construction |
| `gen::transform<T,U>(gen, func)` | Value transformation |
| `gen::derive<T,U>(gen, genGen)` | Value derivation (flat-map) |
| `gen::filter<T>(gen, predicate)` | Value filtering |
| `gen::suchThat<T>(gen, predicate)` | Alias for filter |
| `gen::dependency<T,U>(genT, genUGen)` | Dependency between values |
| `gen::chain<T>(genT, genGen)` | Chain of generators |
| `gen::aggregate<T>(genT, aggregator)` | Aggregation of values |
| `gen::accumulate<T>(genT, accumulator, min, max)` | Accumulation of values |
| `gen::lazy<T>(func)` | Lazy evaluation |
| `gen::reference<T>(ref)` | Reference wrapper |

```cpp
// Constant value generator
auto constantGen = gen::just<int>(42);

// Random selection from values
auto choiceGen = gen::elementOf<int>(1, 2, 3, 5, 7, 11, 13);

// Union of generators
auto mixedIntGen = gen::oneOf<int>(
    gen::interval<int>(1, 10),
    gen::interval<int>(100, 110),
    gen::interval<int>(1000, 1010)
);

// Object construction
struct Point {
    Point(int x, int y) : x(x), y(y) {}
    int x, y;
};
auto pointGen = gen::construct<Point, int, int>(
    gen::interval<int>(-100, 100),
    gen::interval<int>(-100, 100)
);

// Value transformation
auto stringLengthGen = gen::transform<std::string, size_t>(
    gen::string(),
    [](const std::string& s) { return s.length(); }
);

// Value filtering
auto evenIntGen = gen::filter<int>(
    gen::int32(),
    [](int n) { return n % 2 == 0; }
);

// Dependency between values
auto dependentGen = gen::dependency<int, std::string>(
    gen::interval<int>(1, 10),
    [](int size) { return gen::string().setSize(size); }
);

// Usage in forAll
forAll([](int choice, Point p, size_t length, int even) {
    // Property test with combinator-generated values
}, gen::elementOf<int>(1, 2, 3),
   gen::construct<Point, int, int>(gen::interval<int>(-100, 100), gen::interval<int>(-100, 100)),
   gen::transform<std::string, size_t>(gen::string(), [](const std::string& s) { return s.length(); }),
   gen::filter<int>(gen::int32(), [](int n) { return n % 2 == 0; }));
```

### Utility Functions for `oneOf`(`unionOf`) and `elementOf`

There are utility functions for providing probabilities for each value or generator to be chosen for `gen::oneOf` or `gen::elementOf`.

| Alias | Description |
|-------|-------------|
| `gen::weighted<T>(gen, weight)` | Weighted generator decorator |
| `gen::weightedVal<T>(value, weight)` | Weighted value decorator |


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

## Building Custom Generators

You can build your own generator for type `T` by manually defining the conforming generator type `GenFunction<T>`. You can refer to [Building Custom Generators from Scratch](CustomGenerator.md) for more information.

While you can build a custom generator from scratch, it's usually not recommended as there is a better option - using a **generator combinator**. Generator combinators are toolkit for building new generators based on existing ones.
They can also be chained to create another generator out of themselves. See [Combinators](Combinators.md) page for the detail.

&nbsp;

[^generatorT]: In fact, `Arbitrary<T>` inherits from `Generator<T>`, which provides those helpers.

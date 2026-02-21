# Generators

> **New to property-based testing?** Start with the [Walkthrough](Walkthrough.md) for a step-by-step guide. This page provides a comprehensive reference for all built-in generators in `cppproptest`.

Generators are the foundation of property-based testing in `cppproptest`. They define the **input domain** of a property by specifying how random values should be generated. A generator is essentially a function that produces random values according to defined constraints.

---

## Generator Quick Reference

The following table provides a comprehensive overview of all generators available in `cppproptest`, organized by category. Click on any generator name to jump to its detailed documentation.

| Category | Generator | Description |
|----------|-----------|-------------|
| **Primitives** | | |
| | [`gen::boolean`](#genboolean) | Boolean values |
| | [`gen::character`](#gencharacter) | Character values |
| | [`gen::int8`, `gen::uint8`, `gen::int16`, `gen::uint16`, `gen::int32`, `gen::uint32`, `gen::int64`, `gen::uint64`](#integer-generators) | Integer types (8, 16, 32, 64-bit) |
| | [`gen::float32`](#floating-point-generators), [`gen::float64`](#floating-point-generators) | Floating point numbers |
| **Strings** | | |
| | [`gen::string`](#genstring) | ASCII strings |
| | [`gen::utf8string`](#genutf8string) | UTF-8 strings |
| | [`gen::utf16bestring`, `gen::utf16lestring`](#utf-16-string-generators) | UTF-16 strings (big-endian, little-endian) |
| | [`gen::cesu8string`](#gencesu8string) | CESU-8 strings |
| **Containers** | | |
| | [`gen::vector<T>`](#genvectort) | Vectors |
| | [`gen::list<T>`](#genlistt) | Lists |
| | [`gen::set<T>`](#gensett) | Sets |
| | [`gen::map<K,V>`](#genmapkv) | Maps |
| | [`gen::optional<T>`](#genoptionalt) | Optional values |
| | [`gen::shared_ptr<T>`](#gensharedptrt) | Shared pointers |
| **Tuples & Pairs** | | |
| | [`gen::pair<T1,T2>`](#genpairt1t2) | Pairs |
| | [`gen::tuple<Ts...>`](#gentuplets) | Tuples |
| **Numeric Ranges** | | |
| | [`gen::natural<T>(max)`](#gennaturaltmax) | Positive integers ≤ max |
| | [`gen::nonNegative<T>(max)`](#gennonnegativetmax) | Non-negative integers ≤ max |
| | [`gen::interval<T>(min, max)`](#genintervaltmin-max) | Integers in [min, max] |
| | [`gen::inRange<T>(from, to)`](#geninrangetfrom-to) | Integers in [from, to) |
| | [`gen::integers<T>(start, count)`](#genintegerststart-count) | Integers in [start, start+count) |
| **Intervals** | | |
| | [`gen::intervals({...})`](#interval-generators) | Multiple intervals (signed) |
| | [`gen::uintervals({...})`](#interval-generators) | Multiple intervals (unsigned) |
| **Combinators** | | |
| | [`gen::just<T>(value)`](#combinator-functions) | Constant value |
| | [`gen::elementOf<T>(vals...)`](#combinator-functions) | Random selection from values |
| | [`gen::oneOf<T>(gens...)`](#combinator-functions) | Union of generators |
| | [`gen::unionOf<T>(gens...)`](#combinator-functions) | Alias for `gen::oneOf` |
| | [`gen::construct<Class,Args...>(gens...)`](#combinator-functions) | Object construction |
| | [`gen::transform<T,U>(gen, func)`](#combinator-functions) | Value transformation |
| | [`gen::derive<T,U>(gen, genGen)`](#combinator-functions) | Value derivation (flat-map) |
| | [`gen::filter<T>(gen, predicate)`](#combinator-functions) | Value filtering |
| | [`gen::suchThat<T>(gen, predicate)`](#combinator-functions) | Alias for filter |
| | [`gen::dependency<T,U>(genT, genUGen)`](#combinator-functions) | Dependency between values |
| | [`gen::chain<T>(genT, genGen)`](#combinator-functions) | Chain of generators |
| | [`gen::aggregate<T>(genT, aggregator)`](#combinator-functions) | Aggregation of values |
| | [`gen::accumulate<T>(genT, accumulator, min, max)`](#combinator-functions) | Accumulation of values |
| | [`gen::lazy<T>(func)`](#combinator-functions) | Lazy evaluation |
| | [`gen::reference<T>(ref)`](#combinator-functions) | Reference wrapper |
| | [`gen::weighted<T>(gen, weight)`](#combinator-functions) | Weighted generator decorator |
| | [`gen::weightedVal<T>(value, weight)`](#combinator-functions) | Weighted value decorator |
| **Utilities** | | |
| | [`gen::arbitrary<T>(...)`](#access-to-an-arbitrary) | Access to arbitrary |

**Note:** For detailed documentation on combinators, see the [Combinators](Combinators.md) page. For building custom generators, see [Custom Generator](CustomGenerator.md).

---

## Generators in Property-Based Testing

Property-based testing promotes the concept of **input domain** of a property. In property-based testing, **generators** are the means for representing input domains. Even a simple `forAll()` call depends on generators under the hood. Consider the following example:

```cpp
forAll([](int age, std::string name) {
    // Property test code
});
```

This `forAll()` call takes a function with parameters of types `int` and `std::string`. This function is the *property function*. If no additional specification is given on how to generate the values for `age` and `name` as in this example, the parameter types are identified to invoke the default generators for those types. In this case, it calls the default generators for `int` and `std::string` types.

Those default generators are called the *arbitraries*. You can access the arbitraries of a type `T` with `Arbitrary<T>` or `Arbi<T>`.
Previous code is actually equivalent to:

```cpp
forAll([](int age, std::string name) {
    // Property test code
}, Arbitrary<int>(), Arbitrary<std::string>());
```

Notice the extra arguments `gen::arbitrary<int>()` and `gen::arbitrary<std::string>()` in the `forAll()` call. As you can see, `forAll()` actually requires some information on how to generate the values for the parameter types. Some of the commonly used data types have default generators defined in `cppproptest`.

`gen` namespace also provide alias template `gen::arbitrary<T>`. So you can also write the same code as:

```cpp
forAll([](int age, std::string name) {
    // Property test code
}, gen::arbitrary<int>(), gen:arbitrary<std::string>());
```

**See also:** [Arbitrary](Arbitrary.md) for details on default generators, [Property API Reference](PropertyAPI.md) for `forAll()` usage, and [Walkthrough](Walkthrough.md) for step-by-step examples.

---

## Built-in Arbitraries and Their Generator Aliases

Specifically for `Arbitrary<int>()` in the above example, we have an alias - `gen::int32()`. `cppproptest` provides such built-in arbitraries for common data types and useful aliases to those arbitraries in the `gen` namespace. Thus, you can also write the above example as following:

```cpp
forAll([](int age, std::string name) {
    // Property test code
}, gen::int32(), gen::string());
```

Here is the complete list of the built-in arbitraries and their aliases. You can also refer to the [Arbitrary](Arbitrary.md) page for more about arbitraries, including their configurable options and how they make up as default generators for a data type.

**See also:** [Gen Namespace](GenNamespace.md) for details on the `gen` namespace organization.

---

## Basic Type Generators

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| [`gen::boolean`](#genboolean) | `Arbi<bool>` | Boolean values |
| [`gen::character`](#gencharacter) | `Arbi<char>` | Character values |
| [`gen::int8`, `gen::uint8`, `gen::int16`, `gen::uint16`, `gen::int32`, `gen::uint32`, `gen::int64`, `gen::uint64`](#integer-generators) | `Arbi<int8_t>`, `Arbi<uint8_t>`, `Arbi<int16_t>`, `Arbi<uint16_t>`, `Arbi<int32_t>`, `Arbi<uint32_t>`, `Arbi<int64_t>`, `Arbi<uint64_t>` | Integer types (8, 16, 32, 64-bit) |
| [`gen::float32`](#floating-point-generators), [`gen::float64`](#floating-point-generators) | `Arbi<float>`, `Arbi<double>` | Floating point numbers |

### `gen::boolean`

Generates random boolean values (`true` or `false`) with configurable probability.

**Parameters:**

- `trueProb` (double, default: 0.5): Probability of generating `true` (0.0 to 1.0)

**Returns:** `Generator<bool>`

**Examples:**
```cpp
// Generate random booleans (50% true, 50% false)
auto boolGen = gen::boolean();

// Generate mostly true values (80% true, 20% false)
auto boolGen = gen::boolean(0.8);

// Usage in forAll
forAll([](bool b) {
    // Property test with booleans
}, gen::boolean());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options.

---

### `gen::character`

Generates random character values.

**Parameters:**

- None

**Returns:** `Generator<char>`

**Examples:**
```cpp
// Generate random characters
auto charGen = gen::character();

// Usage in forAll
forAll([](char c) {
    // Property test with characters
}, gen::character());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options, [String Generators](#string-generators) for string generation.

---

### Integer Generators

Generate signed and unsigned integers of various bit widths: `gen::int8`, `gen::uint8`, `gen::int16`, `gen::uint16`, `gen::int32`, `gen::uint32`, `gen::int64`, `gen::uint64`.

**Parameters:**

- None

**Returns:** `Generator<int8_t>`, `Generator<uint8_t>`, `Generator<int16_t>`, `Generator<uint16_t>`, `Generator<int32_t>`, `Generator<uint32_t>`, `Generator<int64_t>`, or `Generator<uint64_t>`

**Examples:**
```cpp
// Generate integers of various sizes
auto int8Gen = gen::int8();
auto uint8Gen = gen::uint8();
auto int16Gen = gen::int16();
auto uint16Gen = gen::uint16();
auto int32Gen = gen::int32();
auto uint32Gen = gen::uint32();
auto int64Gen = gen::int64();
auto uint64Gen = gen::uint64();

// Usage in forAll
forAll([](int32_t i, uint32_t u) {
    // Property test with integers
}, gen::int32(), gen::uint32());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options.

---

## Utility Numeric Range Generators

There are also utility integer generators based on numeric range for everyday use. These are function calls, not type aliases. The first three (`gen::interval`, `gen::inRange`, `gen::integers`) are essentially the same concept with different parameter styles and inclusive/exclusive semantics, so you can choose based on preference and clarity.

| Alias | Description |
|-------|-------------|
| [`gen::interval<T>(min, max)`](#genintervaltmin-max) | Integers in [min, max] (inclusive) |
| [`gen::inRange<T>(from, to)`](#geninrangetfrom-to) | Integers in [from, to) (exclusive) |
| [`gen::integers<T>(start, count)`](#genintegerststart-count) | Integers in [start, start+count) |
| [`gen::natural<T>(max)`](#gennaturaltmax) | Positive integers ≤ max |
| [`gen::nonNegative<T>(max)`](#gennonnegativetmax) | Non-negative integers ≤ max |

### `gen::interval<T>(min, max)`

Generates integers in the range [min, max] (inclusive of both bounds).

**Parameters:**

- `min` (T): Minimum integer value (inclusive)
- `max` (T): Maximum integer value (inclusive)

**Returns:** `Generator<T>`

**Examples:**
```cpp
// Generate integers from 1 to 100
auto smallIntGen = gen::interval<int>(1, 100);

// Different integer types
auto smallInt8Gen = gen::interval<int8_t>(-128, 127);
auto int64Gen = gen::interval<int64_t>(-1000000, 1000000);

// Usage in forAll
forAll([](int small) {
    // Property test with range-constrained integers
}, gen::interval<int>(1, 100));
```

**See also:** [Basic Type Generators](#basic-type-generators) for integer type generators, [gen::inRange<T>(from, to)](#geninrangetfrom-to) for exclusive ranges, [Interval Generators](#interval-generators) for multiple intervals.

---

### `gen::inRange<T>(from, to)`

Generates integers in the range [from, to) (exclusive of `to`).

**Parameters:**

- `from` (T): Minimum integer value (inclusive)
- `to` (T): Maximum integer value (exclusive)

**Returns:** `Generator<T>`

**Examples:**
```cpp
// Generate integers from 0 to 99 (exclusive of 100)
auto rangeIntGen = gen::inRange<int>(0, 100);

// Usage in forAll
forAll([](int index) {
    // Property test with range-constrained integers
}, gen::inRange<int>(0, 100));
```

**See also:** [Basic Type Generators](#basic-type-generators) for integer type generators, [gen::interval<T>(min, max)](#genintervaltmin-max) for inclusive ranges.

---

### `gen::integers<T>(start, count)`

Generates integers in the range [start, start+count). The second parameter is the **count** of values to generate, not the maximum value.

**Parameters:**

- `start` (T): Starting value (inclusive)
- `count` (T): Number of values to generate (must be positive)

**Returns:** `Generator<T>`

**Examples:**
```cpp
// Generate integers from 10 to 29 (20 values)
auto sequenceIntGen = gen::integers<int>(10, 20);

// Usage in forAll
forAll([](int value) {
    // Property test with sequence integers
}, gen::integers<int>(10, 20));
```

**See also:** [Basic Type Generators](#basic-type-generators) for integer type generators, [gen::interval<T>(min, max)](#genintervaltmin-max) for inclusive ranges.

---

### `gen::natural<T>(max)`

Generates positive integers in the range [1, max] (inclusive).

**Parameters:**

- `max` (T): Maximum value (inclusive, must be at least 1)

**Returns:** `Generator<T>`

**Examples:**
```cpp
// Generate natural numbers from 1 to 1000
auto positiveIntGen = gen::natural<int>(1000);

// Different integer types
auto uint16Gen = gen::natural<uint16_t>(65535);

// Usage in forAll
forAll([](int positive) {
    // Property test with positive integers
}, gen::natural<int>(1000));
```

**See also:** [Basic Type Generators](#basic-type-generators) for integer type generators, [gen::nonNegative<T>(max)](#gennonnegativetmax) for non-negative integers.

---

### `gen::nonNegative<T>(max)`

Generates non-negative integers in the range [0, max] (inclusive).

**Parameters:**

- `max` (T): Maximum value (inclusive, must be non-negative)

**Returns:** `Generator<T>`

**Examples:**
```cpp
// Generate non-negative integers from 0 to 500
auto nonNegIntGen = gen::nonNegative<int>(500);

// Usage in forAll
forAll([](int nonNeg) {
    // Property test with non-negative integers
}, gen::nonNegative<int>(500));
```

**See also:** [Basic Type Generators](#basic-type-generators) for integer type generators, [gen::natural<T>(max)](#gennaturaltmax) for positive integers.

---

## String Generators

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| [`gen::string`](#genstring) | `Arbi<std::string>` | ASCII strings |
| [`gen::utf8string`](#genutf8string) | `Arbi<UTF8String>` | UTF-8 strings |
| [`gen::utf16bestring`, `gen::utf16lestring`](#utf-16-string-generators) | `Arbi<UTF16BEString>`, `Arbi<UTF16LEString>` | UTF-16 strings (big-endian, little-endian) |
| [`gen::cesu8string`](#gencesu8string) | `Arbi<CESU8String>` | CESU-8 strings |

### `gen::string`

Generates ASCII strings. Defaults to printable ASCII characters (0x01-0x7f).

**Parameters:**

- `charGen` (optional): Generator for characters. If not provided, uses printable ASCII.

**Returns:** `Generator<std::string>`

**Examples:**
```cpp
// Generate ASCII strings with default character set
auto asciiStringGen = gen::string();

// Custom string with specific character ranges
auto uppercaseGen = gen::string(gen::interval<char>('A', 'Z'));
auto digitGen = gen::string(gen::interval<char>('0', '9'));

// Usage in forAll
forAll([](std::string name) {
    // Property test with strings
}, gen::string());
```

**See also:** [Utility Numeric Range Generators](#utility-numeric-range-generators) for `gen::interval`, [Combinators](Combinators.md) for combining character generators, [Arbitrary](Arbitrary.md) for configuration options.

---

### `gen::utf8string`

Generates UTF-8 strings. Defaults to all UTF-8 characters.[^encstring]

**Parameters:**

- `charGen` (optional): Generator for Unicode code points. If not provided, uses all UTF-8 characters.

**Returns:** `Generator<UTF8String>`

**Examples:**
```cpp
// Generate UTF-8 strings with default character set
auto utf8StringGen = gen::utf8string();

// Custom UTF-8 string with specific character ranges
auto alphanumericGen = gen::utf8string(gen::unionOf<uint32_t>(
    gen::interval<uint32_t>('A', 'Z'),
    gen::interval<uint32_t>('a', 'z'),
    gen::interval<uint32_t>('0', '9')
));

// Usage in forAll
forAll([](UTF8String text) {
    // text behaves like std::string for byte-level APIs:
    // const char* raw      = text.c_str();   // pass to C APIs
    // auto        byteSize = text.size();    // buffer size in bytes
    // auto        charCnt  = text.charsize(); // number of UTF-8 code units / characters

    // Property test with UTF-8 strings
}, gen::utf8string());
```

**See also:** [Combinators](Combinators.md) for `gen::unionOf` and other combinators, [Utility Numeric Range Generators](#utility-numeric-range-generators) for `gen::interval`, [Arbitrary](Arbitrary.md) for configuration options.

---

### UTF-16 String Generators

Generate UTF-16 strings in big-endian or little-endian byte order: `gen::utf16bestring`, `gen::utf16lestring`.[^encstring]

**Parameters:**

- `charGen` (optional): Generator for Unicode code points. If not provided, uses default character set.

**Returns:** `Generator<UTF16BEString>` or `Generator<UTF16LEString>`

**Examples:**
```cpp
// Generate UTF-16 big-endian strings
auto utf16beStringGen = gen::utf16bestring();

// Generate UTF-16 little-endian strings
auto utf16leStringGen = gen::utf16lestring();

// Usage in forAll
forAll([](UTF16BEString beText, UTF16LEString leText) {
    // Underlying storage is std::string; .charsize() reports UTF-16 code units:
    // const char* rawBE   = beText.c_str();
    // const char* rawLE   = leText.c_str();
    // auto        beBytes = beText.size();
    // auto        beChars = beText.charsize();

    // Property test with UTF-16 strings
}, gen::utf16bestring(), gen::utf16lestring());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options, [`gen::utf8string`](#genutf8string) for UTF-8 strings.

---

### `gen::cesu8string`

Generates CESU-8 strings.[^encstring]

**Parameters:**

- `charGen` (optional): Generator for Unicode code points.

**Returns:** `Generator<CESU8String>`

**Examples:**
```cpp
// Generate CESU-8 strings
auto cesu8StringGen = gen::cesu8string();

// Usage in forAll
forAll([](CESU8String text) {
    // Access raw bytes and logical length:
    // const char* raw      = text.data();
    // auto        byteSize = text.size();
    // auto        charCnt  = text.charsize();

    // Property test with CESU-8 strings
}, gen::cesu8string());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options.

---

## Container Generators

Container generators often take size ranges and an element generator (uses default generator for the type if not given) as arguments.

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| [`gen::vector<T>`](#genvectort) | `Arbi<std::vector<T>>` | Vectors |
| [`gen::list<T>`](#genlistt) | `Arbi<std::list<T>>` | Lists |
| [`gen::set<T>`](#gensett) | `Arbi<std::set<T>>` | Sets |
| [`gen::map<K,V>`](#genmapkv) | `Arbi<std::map<K,V>>` | Maps |
| [`gen::optional<T>`](#genoptionalt) | `Arbi<std::optional<T>>` | Optional values |
| [`gen::shared_ptr<T>`](#gensharedptrt) | `Arbi<std::shared_ptr<T>>` | Shared pointers |

### `gen::vector<T>`

Generates random vectors with elements of type `T`.

**Parameters:**

- `elementGen` (optional): Generator for vector elements. If not provided, uses default generator for type `T`.
- `minSize`, `maxSize` (optional): Size constraints for the vector.

**Returns:** `Generator<std::vector<T>>`

**Examples:**
```cpp
// Basic vector generator
auto intVectorGen = gen::vector<int>();

// Vector with custom element generator
auto smallIntVectorGen = gen::vector<int>(gen::interval<int>(1, 100));

// Vector with size constraints
auto fixedSizeVectorGen = gen::vector<int>();
fixedSizeVectorGen.setSize(5); // Always generates vectors of size 5

auto boundedVectorGen = gen::vector<int>();
boundedVectorGen.setMinSize(1);
boundedVectorGen.setMaxSize(10); // Generates vectors of size 1-10

// Size range in constructor
auto vectorGen1 = gen::vector<int>(5, 5); // Fixed size 5
auto vectorGen2 = gen::vector<int>(gen::interval(1,10), 5, 5); // element generator + size range

// Usage in forAll
forAll([](std::vector<int> numbers) {
    // Property test with vectors
}, gen::vector<int>());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options, [Utility Numeric Range Generators](#utility-numeric-range-generators) for `gen::interval`.

---

### `gen::list<T>`

Generates random lists with elements of type `T`.

**Parameters:**

- `elementGen` (optional): Generator for list elements. If not provided, uses default generator for type `T`.
- `minSize`, `maxSize` (optional): Size constraints for the list.

**Returns:** `Generator<std::list<T>>`

**Examples:**
```cpp
// Basic list generator
auto stringListGen = gen::list<std::string>();

// List with custom element generator
auto uppercaseStringListGen = gen::list<std::string>(gen::string(gen::interval<char>('A', 'Z')));

// Usage in forAll
forAll([](std::list<std::string> items) {
    // Property test with lists
}, gen::list<std::string>());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options, [String Generators](#string-generators) for string element generators.

---

### `gen::set<T>`

Generates random sets with elements of type `T`.

**Parameters:**

- `elementGen` (optional): Generator for set elements. If not provided, uses default generator for type `T`.
- `minSize`, `maxSize` (optional): Size constraints for the set.

**Returns:** `Generator<std::set<T>>`

**Examples:**
```cpp
// Basic set generator
auto intSetGen = gen::set<int>();

// Usage in forAll
forAll([](std::set<int> uniqueNumbers) {
    // Property test with sets
}, gen::set<int>());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options.

---

### `gen::map<K,V>`

Generates random maps with keys of type `K` and values of type `V`.

**Parameters:**

- `keyGen` (optional): Generator for map keys. If not provided, uses default generator for type `K`.
- `valueGen` (optional): Generator for map values. If not provided, uses default generator for type `V`.
- `minSize`, `maxSize` (optional): Size constraints for the map.

**Returns:** `Generator<std::map<K,V>>`

**Examples:**
```cpp
// Basic map generator
auto stringIntMapGen = gen::map<std::string, int>();

// Map with custom key and value generators
auto customMapGen = gen::map<std::string, int>();
customMapGen.setKeyGen(gen::string(gen::interval<char>('a', 'z')));
customMapGen.setElemGen(gen::interval<int>(0, 100));

// Usage in forAll
forAll([](std::map<std::string, int> data) {
    // Property test with maps
}, gen::map<std::string, int>());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options, [String Generators](#string-generators) for string key generators, [Utility Numeric Range Generators](#utility-numeric-range-generators) for integer value generators.

---

### `gen::optional<T>`

Generates random optional values of type `T`.

**Parameters:**

- `elementGen` (optional): Generator for the contained value. If not provided, uses default generator for type `T`.

**Returns:** `Generator<std::optional<T>>`

**Examples:**
```cpp
// Basic optional generator
auto optionalIntGen = gen::optional<int>();

// Usage in forAll
forAll([](std::optional<int> maybeValue) {
    // Property test with optionals
}, gen::optional<int>());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options.

---

### `gen::shared_ptr<T>`

Generates random shared pointers to values of type `T`.

**Parameters:**

- `elementGen` (optional): Generator for the pointed-to value. If not provided, uses default generator for type `T`.

**Returns:** `Generator<std::shared_ptr<T>>`

**Examples:**
```cpp
// Basic shared_ptr generator
auto sharedPtrIntGen = gen::shared_ptr<int>();

// Usage in forAll
forAll([](std::shared_ptr<int> ptr) {
    // Property test with shared pointers
}, gen::shared_ptr<int>());
```

**See also:** [Arbitrary](Arbitrary.md) for configuration options.

---

## Tuple and Pair Generators

`gen::pair` and `gen::tuple` generators take optional element generators as arguments. The generators are optional only if the types have `Arbitrary` defined.

| Alias | Arbitrary | Description |
|-------|----------|-------------|
| [`gen::pair<T1,T2>`](#genpairt1t2) | `Arbi<std::pair<T1,T2>>` | Pairs |
| [`gen::tuple<Ts...>`](#gentuplets) | `Arbi<std::tuple<Ts...>>` | Tuples |
| `gen::pair` |  | Pairs (type params inferred) |
| `gen::tuple` |  | Tuples (type params inferred)|

### `gen::pair<T1,T2>`

Generates random pairs with elements of types `T1` and `T2`.

**Parameters:**

- `gen1`, `gen2` (optional): Generators for the first and second elements. If not provided, uses default generators for types `T1` and `T2`.

**Returns:** `Generator<std::pair<T1,T2>>`

**Examples:**
```cpp
// Basic pair generator
auto intStringPairGen = gen::pair<int, std::string>();

// Pair with custom element generators
auto smallIntUppercaseStringPairGen = gen::pair(
    gen::interval<int>(1, 100),
    gen::string(gen::interval<char>('A', 'Z'))
);

// Type-inferred pair (convenient wrapper)
auto pairGen = gen::pair(
    gen::interval<int>(1, 100),
    gen::string(gen::interval<char>('A', 'Z'))
); // generator for pair<int, string>

// Usage in forAll
forAll([](std::pair<int, std::string> data) {
    // Property test with pairs
}, gen::pair<int, std::string>());
```

**See also:** [Utility Numeric Range Generators](#utility-numeric-range-generators) for `gen::interval`, [String Generators](#string-generators) for string generators, [Combinators](Combinators.md) for `gen::chain` and dependent pair generation.

---

### `gen::tuple<Ts...>`

Generates random tuples with elements of types `Ts...`.

**Parameters:**

- `...gens` (optional): Generators for tuple elements. If not provided, uses default generators for the element types.

**Returns:** `Generator<std::tuple<Ts...>>`

**Examples:**
```cpp
// Basic tuple generator
auto intBoolStringTupleGen = gen::tuple<int, bool, std::string>();

// Tuple with custom element generators
auto customTupleGen = gen::tuple(
    gen::interval<int>(-50, 50),
    gen::boolean(),
    gen::string(gen::interval<char>('a', 'z'))
);

// Type-inferred tuple (convenient wrapper)
auto tupleGen = gen::tuple(
    gen::interval<int>(-50, 50),
    gen::boolean(),
    gen::string(gen::interval<char>('a', 'z'))
); // generator for tuple<int, bool, string>

// Usage in forAll
forAll([](std::tuple<int, bool, std::string> info) {
    // Property test with tuples
}, gen::tuple<int, bool, std::string>());
```

**See also:** [Utility Numeric Range Generators](#utility-numeric-range-generators) for `gen::interval`, [Basic Type Generators](#basic-type-generators) for element generators, [Combinators](Combinators.md) for `gen::chain` and dependent tuple generation.

---


## Interval Generators

Interval generators are basically shorthand equivalents for combining multiple numeric ranges (e.g. `gen::interval`) with a `gen::oneOf` (`gen::unionOf`) combinator.

| Alias | Description |
|-------|-------------|
| `gen::intervals({Interval(min1,max1), ...})` | Multiple intervals for signed integers |
| `gen::uintervals({UInterval(min1,max1), ...})` | Multiple intervals for unsigned integers |

**Examples:**
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

**See also:** [Combinators](Combinators.md) for `gen::oneOf` and `gen::unionOf`, [Utility Numeric Range Generators](#utility-numeric-range-generators) for single interval generators, [Basic Type Generators](#basic-type-generators) for integer type generators.

---

## Floating Point Generators

Floating point generators (`gen::float32` and `gen::float64`) support configurable probabilities for special IEEE 754 values: NaN, positive infinity, and negative infinity.

### Interface

**Constructor signatures:**
```cpp
gen::float32(double nanProb = 0.0, double posInfProb = 0.0, double negInfProb = 0.0)
gen::float64(double nanProb = 0.0, double posInfProb = 0.0, double negInfProb = 0.0)

// Equivalent to:
Arbi<float>(double nanProb = 0.0, double posInfProb = 0.0, double negInfProb = 0.0)
Arbi<double>(double nanProb = 0.0, double posInfProb = 0.0, double negInfProb = 0.0)
```

**Parameters:**

- **`nanProb`** (default: `0.0`): Probability of generating NaN, must be in range [0.0, 1.0]
- **`posInfProb`** (default: `0.0`): Probability of generating positive infinity, must be in range [0.0, 1.0]
- **`negInfProb`** (default: `0.0`): Probability of generating negative infinity, must be in range [0.0, 1.0]

**Constraints:**

- Each probability must be between 0.0 and 1.0 (inclusive)
- The sum of all probabilities must be ≤ 1.0
- The remaining probability (1.0 - sum) is used for generating finite values

The constructor validates these requirements and throws `runtime_error` if they are not met.

### Default Behavior

By default, floating point generators produce only **finite values** (no NaN or infinity):

```cpp
// Default: generates only finite values
auto floatGen = gen::float32();
auto doubleGen = gen::float64();

forAll([](float f, double d) {
    PROP_ASSERT(isfinite(f));
    PROP_ASSERT(isfinite(d));
}, gen::float32(), gen::float64());
```

### Special Value Probabilities

You can configure the probability of generating special values using constructor parameters:

```cpp
// Generate 10% NaN, 90% finite
auto floatWithNaN = gen::float32(0.1, 0.0, 0.0);

// Generate 5% +inf, 5% -inf, 90% finite
auto floatWithInf = gen::float32(0.0, 0.05, 0.05);

// Generate 5% each special value, 85% finite
auto floatWithAll = gen::float32(0.05, 0.05, 0.05);

// When sum = 1.0, no finite values are generated
auto onlySpecial = gen::float32(0.5, 0.3, 0.2); // 50% NaN, 30% +inf, 20% -inf

// Same API for double
auto doubleWithNaN = gen::float64(0.1, 0.0, 0.0);

// Usage in forAll
forAll([](float f) {
    PROP_STAT(std::isfinite(f));
    PROP_STAT(std::isinf(f));
    PROP_STAT(std::isnan(f));
    PROP_STAT(f > 0);
    PROP_STAT(f < 0);
}, gen::float64(0.05, 0.05, 0.05));
```

**See also:** [Property API Reference](PropertyAPI.md) for `PROP_STAT` usage, [Basic Type Generators](#basic-type-generators) for other numeric generators, [Arbitrary](Arbitrary.md) for floating-point generator configuration.

---

## Access to an Arbitrary

While arbitraries (default generator) for type `T` can be accessed using `proptest::Arbitrary<T>`, a proxy in `gen` namespace is also provided. You may find it useful if type should be parameterized:

| Function | Description |
|----------|-------------|
| `gen::arbitrary<T>(...)` | alias to `proptest::Arbitrary<T>` |

**See also:** [Arbitrary](Arbitrary.md) for details on arbitraries and how to define custom ones.

---

## Combinator Functions

The second part of `gen` namespace contains all the combinators provided by `cppproptest`. Combinators transform or combine existing generators to create new, more complex ones.

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

**Examples:**
```cpp
// Constant value generator
auto constantGen = gen::just<int>(42);

// Random selection from values
auto choiceGen = gen::elementOf<int>(1, 2, 3, 5, 7, 11, 13);

// Union of generators (see [Combinators](Combinators.md) for gen::oneOf, [Utility Numeric Range Generators](#utility-numeric-range-generators) for gen::interval)
auto mixedIntGen = gen::oneOf<int>(
    gen::interval<int>(1, 10),
    gen::interval<int>(100, 110),
    gen::interval<int>(1000, 1010)
);

// Object construction (see [Combinators](Combinators.md) for gen::construct, [Utility Numeric Range Generators](#utility-numeric-range-generators) for gen::interval)
struct Point {
    Point(int x, int y) : x(x), y(y) {}
    int x, y;
};
auto pointGen = gen::construct<Point, int, int>(
    gen::interval<int>(-100, 100),
    gen::interval<int>(-100, 100)
);

// Value transformation (see [Combinators](Combinators.md) for gen::transform, [String Generators](#string-generators) for gen::string)
auto stringLengthGen = gen::transform<std::string, size_t>(
    gen::string(),
    [](const std::string& s) { return s.length(); }
);

// Value filtering (see [Combinators](Combinators.md) for gen::filter, [Basic Type Generators](#basic-type-generators) for gen::int32)
auto evenIntGen = gen::filter<int>(
    gen::int32(),
    [](int n) { return n % 2 == 0; }
);

// Dependency between values (see [Combinators](Combinators.md) for gen::dependency, [Utility Numeric Range Generators](#utility-numeric-range-generators) for gen::interval, [String Generators](#string-generators) for gen::string)
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

**See also:** [Combinators](Combinators.md) for comprehensive documentation on all combinators, including detailed examples and use cases.

### Utility Functions for `oneOf`(`unionOf`) and `elementOf`

There are utility functions for providing probabilities for each value or generator to be chosen for `gen::oneOf` or `gen::elementOf`.

| Alias | Description |
|-------|-------------|
| `gen::weighted<T>(gen, weight)` | Weighted generator decorator |
| `gen::weightedVal<T>(value, weight)` | Weighted value decorator |

**See also:** [Combinators](Combinators.md) for details on weighted selection.

---

## Examples

### Basic Generators

```cpp
// Basic types (see [Basic Type Generators](#basic-type-generators))
auto boolGen = gen::boolean();
auto intGen = gen::int32();
auto stringGen = gen::string(); // see [String Generators](#string-generators)

// Floating point - default (finite only) (see [Floating Point Generators](#floating-point-generators))
auto floatGen = gen::float32();
auto doubleGen = gen::float64();

// Floating point with special values (see [Floating Point Generators](#floating-point-generators))
auto floatWithNaN = gen::float32(0.1, 0.0, 0.0); // 10% NaN, 90% finite
auto doubleWithInf = gen::float64(0.0, 0.05, 0.05); // 5% +inf, 5% -inf, 90% finite

// Numeric ranges - these are function calls, not type aliases (see [Utility Numeric Range Generators](#utility-numeric-range-generators))
auto smallIntGen = gen::interval(1, 100);
auto positiveIntGen = gen::natural(1000);
auto nonNegIntGen = gen::nonNegative(500);
```

### Container Generators

```cpp
// Simple containers (see [Container Generators](#container-generators))
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
// Custom character ranges (see [String Generators](#string-generators), [Utility Numeric Range Generators](#utility-numeric-range-generators) for gen::interval, [Combinators](Combinators.md) for gen::unionOf)
auto uppercaseGen = gen::string(gen::interval<char>('A', 'Z'));
auto digitGen = gen::string(gen::interval<char>('0', '9'));
auto alphanumericGen = gen::string(gen::unionOf<char>(
    gen::interval<char>('A', 'Z'),
    gen::interval<char>('a', 'z'),
    gen::interval<char>('0', '9')
));
```

**See also:** [Generator Examples](GeneratorExamples.md) for more comprehensive real-world examples.

---

## Building Custom Generators

You can build your own generator for type `T` by manually defining the conforming generator type `GenFunction<T>`. You can refer to [Building Custom Generators from Scratch](CustomGenerator.md) for more information.

While you can build a custom generator from scratch, it's usually not recommended as there is a better option - using a **generator combinator**. Generator combinators are toolkit for building new generators based on existing ones. They can also be chained to create another generator out of themselves.

**See also:** [Combinators](Combinators.md) for comprehensive guide on using combinators, [Custom Generator](CustomGenerator.md) for building generators from scratch, [Generator Examples](GeneratorExamples.md) for practical examples.

---

## Related Topics

- [Arbitrary](Arbitrary.md) - Default generators for types
- [Combinators](Combinators.md) - Transform and combine generators
- [Custom Generator](CustomGenerator.md) - Building generators from scratch
- [Generator Examples](GeneratorExamples.md) - Real-world generator usage
- [Gen Namespace](GenNamespace.md) - Organization of the `gen` namespace
- [Walkthrough](Walkthrough.md) - Step-by-step guide to using generators
- [Property API Reference](PropertyAPI.md) - Using generators with `forAll()` and `property()`
- [Shrinking](Shrinking.md) - How generated values are simplified when tests fail

[^encstring]: Encoding-aware string types `UTF8String`, `UTF16BEString`, `UTF16LEString`, and `CESU8String` are implemented as classes derived from `std::string`. This lets you use standard string APIs such as `.c_str()` / `.data()` to access the underlying bytes and `.size()` to get the buffer length in bytes, while `.charsize()` reports the logical character/code-unit count. For example:
```cpp
UTF8String s = UTF8String("café");
auto bytes = s.size();     // buffer size in bytes
auto chars = s.charsize(); // number of Unicode code units
const char* raw = s.c_str(); // pass to C APIs
```

This design differs from `std::u8string` / `std::u16string`, but is used in `cppproptest` to keep generator and shrinking support built on top of `std::string` storage.

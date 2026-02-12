# Generator Combinators

> **New to property-based testing?** Start with the [Walkthrough](Walkthrough.md) for a step-by-step guide. This page provides a comprehensive reference for all generator combinators in `cppproptest`.

Generator combinators are functions that build new generators from existing ones. Many combinators are inspired by functional programming patterns and can be chained together to create complex generation logic. Each combinator receives one or more existing generators as arguments and returns a new generator.

**Note:** All combinators are available in the `proptest::gen` namespace. You can use them with the `gen::` prefix (e.g., `gen::just`, `gen::elementOf`) or include the backward compatibility aliases from the `proptest` namespace. The examples in this documentation use the modern `gen::` prefix.

---

## Combinator Quick Reference

The following table provides a quick overview of available combinators. Click on any combinator name to jump to its detailed documentation.

| Combinator | Purpose | See Also |
|------------|---------|----------|
| [`gen::just<T>`](#genjustt) | Generate a constant value | [Constants](#constants) |
| [`gen::lazy<T>`](#genlazyt) | Generate a value by calling a function | [Constants](#constants) |
| [`gen::elementOf<T>`](#genelementoft) | Randomly select from a list of constant values | [Selecting from Constants](#selecting-from-constants) |
| [`gen::interval<T>`](#genintervalt), [`gen::integers<T>`](#genintegerst), [`gen::natural<T>`](#gennaturalt), [`gen::nonNegative<T>`](#gennonnegativet) | Generate values within numeric ranges | [Integers and Intervals](#integers-and-intervals), [Generators.md](Generators.md#utility-numeric-range-generators) |
| [`gen::pair<T1,T2>`](#genpairt1t2), [`gen::tuple<Ts...>`](#gentuplets) | Combine generators to produce pairs or tuples | [Pair and Tuples](#pair-and-tuples) |
| [`gen::oneOf<T>`](#genoneoft), [`gen::unionOf<T>`](#genunionoft) | Randomly select from multiple generators | [Selecting from Generators](#selecting-from-generators) |
| [`gen::transform<T,U>`](#gentransformtu) | Transform generator output to another type | [Transforming or Mapping](#transforming-or-mapping) |
| [`gen::derive<T,U>`](#genderivetu) | Derive a new generator based on generated value | [Deriving or Flat-mapping](#deriving-or-flat-mapping) |
| [`gen::construct<T,Args...>`](#genconstructtargs) | Generate objects by calling constructors | [Constructing an Object](#constructing-an-object) |
| [`gen::filter<T>`](#genfiltert), [`gen::suchThat<T>`](#gensuchthatt) | Filter generated values by predicate | [Applying Constraints](#applying-constraints) |
| [`gen::dependency<T,U>`](#gendependencytu) | Generate pairs with dependent second element | [Values with Dependencies](#values-with-dependencies) |
| [`gen::chain<Ts...,U>`](#genchaintsu) | Generate tuples with dependent elements | [Values with Dependencies](#values-with-dependencies) |
| [`gen::aggregate<T>`](#genaggregatet) | Generate sequences where each value depends on previous | [Aggregation or Accumulation](#aggregation-or-accumulation-of-values) |
| [`gen::accumulate<T>`](#genaccumulatet) | Generate sequences, return only the final value | [Aggregation or Accumulation](#aggregation-or-accumulation-of-values) |

&nbsp;

## Basic Generator Combinators

### Constants

#### `gen::just<T>`

Generates a constant value. Useful when you need a fixed value in your property tests.

**Signatures:**

* `gen::just<T>(T*)` - Takes a pointer to a value
* `gen::just<T>(T)` - Takes a value by copy
* `gen::just<T>(shared_ptr<T>)` - Takes a shared pointer (useful for non-copyable types)

**Example:**

```cpp
auto zeroGen = gen::just(0); // template argument is optional if type is deducible
auto stringGen = gen::just<std::string>("hello");
```

**See also:** [Generators.md](Generators.md#combinator-functions)

#### `gen::lazy<T>`

Generates a value by calling a function each time. Useful when you need to generate a fresh value on each call (e.g., for mutable objects or time-dependent values).

**Signature:** `gen::lazy<T>(function<T()>)`

**Example:**

```cpp
auto oneGen = gen::lazy<int>([]() { return 1; });
auto timestampGen = gen::lazy<int64_t>([]() { return getCurrentTimestamp(); });
```

**See also:** [Generators.md](Generators.md#combinator-functions)

### Selecting from Constants

#### `gen::elementOf<T>`

Randomly selects one value from a provided list of constant values. Useful for choosing from a fixed set of options.

**Signature:** `gen::elementOf<T>(val1, ..., valN)`

**Example:**

```cpp
// generates a prime number under 50
auto primeGen = gen::elementOf<int>(2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47);

// generates a HTTP method
auto httpMethodGen = gen::elementOf<std::string>("GET", "POST", "PUT", "DELETE");
```

**Weighted Selection:**

`gen::elementOf` supports probabilistic weights (`0 < weight <= 1`). The sum of weights should ideally be 1.0 but must not exceed 1.0. If a weight is unspecified for a value, the remaining probability (1.0 minus the sum of specified weights) is distributed evenly among the values without specified weights.

Use `gen::weightedVal(<value>, <weight>)` to annotate the desired weight.

**Example:**

```cpp
// generates 2, 5, or 10 with specified probabilities
// weight for 10 automatically becomes 1.0 - 0.8 - 0.15 == 0.05
gen::elementOf<int>(gen::weightedVal(2, 0.8), gen::weightedVal(5, 0.15), 10);
```

**See also:** [Generators.md](Generators.md#combinator-functions)

### Integers and Intervals

Utility generators for generating integers within specific ranges. These combinators are convenient wrappers around the base integer generators.

**See also:** [Generators.md](Generators.md#utility-numeric-range-generators) for more details on numeric range generators.

#### `gen::interval<T>`

Generates an integer type in the closed interval `[min, max]` (both endpoints inclusive).

**Signature:** `gen::interval<INT_TYPE>(min, max)`

**Example:**

```cpp
gen::interval<int64_t>(1, 28);
gen::interval(1, 48); // template type argument can be omitted if input type matches output type
gen::interval(0, 10); // generates an integer in {0, ..., 10}
gen::interval('A', 'Z'); // generates a char of uppercase alphabet
```

#### `gen::integers<T>`

Generates an integer type starting from `from` with a count of values.

**Signature:** `gen::integers<INT_TYPE>(from, count)`

**Example:**

```cpp
gen::integers(0, 10); // generates an integer in {0, ..., 9}
gen::integers(1, 10); // generates an integer in {1, ..., 10}
```

#### `gen::natural<T>`

Generates a positive integer (greater than 0) up to `max` (inclusive).

**Signature:** `gen::natural<INT_TYPE>(max)`

**Example:**

```cpp
auto positiveGen = gen::natural<int>(100); // generates integers in {1, ..., 100}
```

#### `gen::nonNegative<T>`

Generates zero or a positive integer (>= 0) up to `max` (inclusive).

**Signature:** `gen::nonNegative<INT_TYPE>(max)`

**Example:**

```cpp
auto nonNegGen = gen::nonNegative<int>(100); // generates integers in {0, ..., 100}
```

**Note:** `gen::interval`, `gen::inRange`, and `gen::integers` are essentially the same with subtle differences for user preference. See [Generators.md](Generators.md#utility-numeric-range-generators) for details.

### Pair and Tuples

Combine generators of different types to produce `std::pair` or `std::tuple` values. These are convenient wrappers that support type inference.

**See also:** [Generators.md](Generators.md#tuples--pairs) for more details on tuple and pair generators.

#### `gen::pair<T1,T2>`

Generates a `std::pair<T1,T2>` based on the results of two generators.

**Signature:** `gen::pair(gen1, gen2)`

**Example:**

```cpp
auto pairGen = gen::pair(gen::int32(), gen::string());
// Equivalent to: gen::pair<int32_t, std::string>(gen::int32(), gen::string())
```

#### `gen::tuple<Ts...>`

Generates a `std::tuple<T1,...,TN>` based on the results of multiple generators.

**Signature:** `gen::tuple(gen1, ..., genN)`

**Example:**

```cpp
auto tupleGen = gen::tuple(gen::int32(), gen::string(), gen::float64());
// Generates std::tuple<int32_t, std::string, double>
```

&nbsp;

&nbsp;

## Advanced Generator Combinators

### Selecting from Generators

Combine multiple generators of the same type into a single generator that randomly selects one of the underlying generators to produce a value. This can be considered as taking a *union* of generators.

#### `gen::oneOf<T>`

Generates a value of type `T` by randomly choosing one of the provided generators. All generators (`gen1` to `genN`) must generate the same type `T`.

**Signature:** `gen::oneOf<T>(gen1, ..., genN)`

**Example:**

```cpp
// generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
auto rangeGen = gen::oneOf<int>(
    gen::interval(0, 10),
    gen::interval(100, 1000),
    gen::interval(10000, 100000)
);
```

**Weighted Selection:**

`gen::oneOf` supports probabilistic weights (`0 < weight <= 1`). The sum of weights should ideally be 1.0 but must not exceed 1.0. If a weight is unspecified for a generator, the remaining probability (1.0 minus the sum of specified weights) is distributed evenly among the generators without specified weights.

Use `weightedGen(<generator>, <weight>)` to annotate the desired weight.

**Example:**

```cpp
// generates a numeric within ranges with specified probabilities
// weight for third generator automatically becomes 1.0 - (0.8 + 0.15) == 0.05
auto weightedRangeGen = gen::oneOf<int>(
    weightedGen(gen::interval(0, 10), 0.8),
    weightedGen(gen::interval(100, 1000), 0.15),
    gen::interval(10000, 100000)
);
```

#### `gen::unionOf<T>`

Alias of `gen::oneOf<T>`.

**See also:** [Generators.md](Generators.md#combinator-functions)

### Constructing an Object

Generate objects of a class or struct type `T` by calling a matching constructor.

#### `gen::construct<T, Args...>`

Generates an object of type `T` by calling its constructor that matches the signature `(ARG1, ..., ARGN)`. Custom generators can be supplied for generating arguments. If fewer generators are provided than constructor arguments, the remaining arguments are generated using their default `Arbi` generators.

**Signature:** `gen::construct<T, ARG1, ..., ARGN>([gen1, ..., genM])`

**Example:**

```cpp
struct Coordinate {
    Coordinate(int x, int y) {
        // ...
    }
};

// Generate Coordinate with custom generators for both arguments
auto coordinateGen1 = gen::construct<Coordinate, int, int>(
    gen::interval(-10, 10),
    gen::interval(-20, 20)
);

// Generate Coordinate with custom generator for x only; y uses default Arbi<int>
auto coordinateGen2 = gen::construct<Coordinate, int, int>(
    gen::interval(-10, 10)
);
```

**See also:** [Arbitrary.md](Arbitrary.md) for information on default generators

### Applying Constraints

Add a filtering condition to a generator to restrict generated values to satisfy a certain constraint.

#### `gen::filter<T>`

Generates a value of type `T` using the base generator `gen`, but only yields values that satisfy the `condition_predicate` (i.e., for which the predicate returns `true`). If the predicate returns `false`, the generation is retried with a new value from `gen`.

**Signature:** `gen::filter<T>(gen, condition_predicate)`

**Example:**

```cpp
// generates even numbers
auto evenGen = gen::filter<int>(gen::int32(), [](const int& num) {
    return num % 2 == 0;
});

// generates positive integers
auto positiveGen = gen::filter<int>(gen::int32(), [](const int& num) {
    return num > 0;
});
```

**Note:** Using `gen::filter` with tight constraints can be inefficient, as many values may be discarded. Consider using [dependency combinators](#values-with-dependencies) or [custom generators](CustomGenerator.md) for better performance.

#### `gen::suchThat<T>`

Alias of `gen::filter<T>`.

**See also:** [CustomGenerator.md](CustomGenerator.md) for creating generators with built-in constraints

### Transforming or Mapping

Transform an existing generator to create a new generator by providing a transformer function. This is equivalent to *mapping* in functional programming.

#### `gen::transform<T,U>`

Generates type `U` based on a generator for type `T`, using a transformer function that converts a value of type `const T&` to type `U`.

**Signature:** `gen::transform<T,U>(gen, transformer)`

**Example:**

```cpp
// generates strings from integers (e.g. "0", "1", ... , "-16384")
auto numStringGen = gen::transform<int, std::string>(
    gen::int32(),
    [](const int& num) {
        return std::to_string(num);
    }
);

// generates uppercase strings from lowercase strings
auto upperGen = gen::transform<std::string, std::string>(
    gen::string(),
    [](const std::string& str) {
        std::string upper = str;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        return upper;
    }
);
```

**See also:** [Deriving or Flat-mapping](#deriving-or-flat-mapping) for context-dependent generation, [Utility Methods](#utility-methods-in-standard-generators) for fluent chaining with `.map<U>()`

### Deriving or Flat-mapping

Another combinator that resembles `gen::transform` is `gen::derive`. This is equivalent to *flat-mapping* or *binding* in functional programming. The key difference from `gen::transform<T, U>` is that the function provided to `gen::derive` returns a *new generator* (`Generator<U>`) based on the intermediate value of type `T`, rather than just transforming the value into a `U`. This allows for more complex, context-dependent generation logic.

#### `gen::derive<T,U>`

Derives a new generator for type `U`. It first generates a value of type `T` using `genT`. Then, it passes this value to `genUGen`, which is a function that returns a `Generator<U>`. This returned generator is then used to produce the final value of type `U`.

**Signature:** `gen::derive<T, U>(genT, genUGen)`

**Example:**

```cpp
// generates a string consisting of only uppercase/lowercase alphabets/numeric characters
auto stringGen = gen::derive<int, std::string>(
    gen::interval(0, 2),
    [](const int& num) {
        if (num == 0)
            return gen::string(gen::interval('A', 'Z'));
        else if (num == 1)
            return gen::string(gen::interval('a', 'z'));
        else // num == 2
            return gen::string(gen::interval('0', '9'));
    }
);
```

**Comparison with `gen::transform`:**

| Combinator | Transformer Signature | Result Type |
|------------|----------------------|-------------|
| `gen::transform<T,U>` | `function<U(T)>` | `Generator<U>` |
| `gen::derive<T,U>` | `function<Generator<U>(T)>` | `Generator<U>` |

**See also:** [Transforming or Mapping](#transforming-or-mapping), [Utility Methods](#utility-methods-in-standard-generators) for fluent chaining with `.flatMap<U>()`



### Values with Dependencies

Generate values where one value depends on another. Two combinators facilitate this: one generates a `std::pair`, and the other generates a `std::tuple`.

#### `gen::dependency<T,U>`

Generates a `std::pair<T,U>`. It first uses `genT` to generate a value of type `T`. This value is then passed to `genUgen`, which is a function that returns a `Generator<U>`. This returned generator produces the second element of the pair. This effectively creates a generator for a pair where the second item depends on the first.

**Signature:** `gen::dependency<T,U>(genT, genUgen)`

**Example:**

```cpp
// Example 1: Generate a size and a vector of that size
auto sizeAndVectorGen = gen::dependency<int, std::vector<bool>>(
    gen::interval(1, 100),
    [](const int& size) {
        auto vectorGen = gen::vector<bool>();
        vectorGen.setSize(size);
        return vectorGen; // generates a vector with exactly 'size' elements
    }
);

// Example 2: Generate a bool and an int depending on the bool
auto nullableIntegerGen = gen::dependency<bool, int>(
    gen::boolean(),
    [](const bool& isNull) -> Generator<int> {
        if (isNull)
            return gen::just(0);
        else
            return gen::interval(10, 20);
    }
);
```

**See also:** [Utility Methods](#utility-methods-in-standard-generators) for fluent chaining with `.pairWith<U>()`

#### `gen::chain<Ts...,U>`

Similar to `gen::dependency`, but operates on tuples. It takes a generator `genTuple` for `std::tuple<Ts...>` and a function `genUFromTuple`. This function receives the generated tuple (`const std::tuple<Ts...>&`) and returns a `Generator<U>`. The final result is a generator for `std::tuple<Ts..., U>`. `gen::chain` can be repeatedly applied to build tuples with multiple dependent elements.

**Signature:** `gen::chain<Ts..., U>(genTuple, genUFromTuple)`

**Example:**

```cpp
auto yearMonthGen = gen::tuple(gen::interval(1900, 2100), gen::interval(1, 12));
// number of days in a month depends on the month and whether the year is a leap year
auto yearMonthDayGen = gen::chain<int, int, int>(
    yearMonthGen,
    [](const std::tuple<int, int>& yearMonth) -> Generator<int> {
        int year = std::get<0>(yearMonth);
        int month = std::get<1>(yearMonth);
        if (monthHas31Days(month)) {
            return gen::interval(1, 31);
        } else if (monthHas30Days(month)) {
            return gen::interval(1, 30);
        } else { // February (month == 2)
            if (isLeapYear(year))
                return gen::interval(1, 29);
            else
                return gen::interval(1, 28);
        }
    }
); // yearMonthDayGen generates std::tuple<int, int, int> representing (year, month, day)
```

**See also:** [Utility Methods](#utility-methods-in-standard-generators) for fluent chaining with `.tupleWith<U>()`

**Alternative Approach with `gen::filter`:**

You can often achieve similar goals using the `gen::filter` combinator:

```cpp
// generate any year, month, day combination first
auto anyYearMonthDayGen = gen::tuple(
    gen::interval(1900, 2100),
    gen::interval(1, 12),
    gen::interval(1, 31)
);
// then apply filter to keep only valid dates
auto validYearMonthDayGen = anyYearMonthDayGen.filter([](const std::tuple<int, int, int>& ymd) {
    int year = std::get<0>(ymd);
    int month = std::get<1>(ymd);
    int day = std::get<2>(ymd);
    return isValidDate(year, month, day); // Assuming isValidDate helper function
});
```

However, using `gen::filter` for generating values with complex dependencies can be inefficient. If the constraints are tight, many generated values might be discarded before a valid one is found, leading to performance issues. In such cases, `gen::dependency` or `gen::chain` are often preferable as they construct valid values directly.


### Aggregation or Accumulation of Values

Generate sequences of values where each value depends on the previously generated one(s). A common example is generating a sequence of chess moves, where knowing the previous move is crucial for generating the next valid move.

Both combinators take:
1. A base generator (`genT`) for the initial value.
2. A function (`genTFromT`) that receives the *previously generated value* of type `T` and returns a `Generator<T>` for the *next* value in the sequence.
3. Minimum (`minSize`) and maximum (`maxSize`) lengths for the generated sequence.

| Combinator | Result Type | Description |
|------------|-------------|-------------|
| `gen::accumulate<T>(genT, genTFromT, minSize, maxSize)` | `Generator<T>` | Generates sequence, returns the **last** element |
| `gen::aggregate<T>(genT, genTFromT, minSize, maxSize)` | `Generator<vector<T>>` | Generates sequence, returns the **entire sequence** vector |

#### `gen::accumulate<T>`

Generates a sequence of values based on the previous one, but ultimately yields only the *final* value generated in that sequence.

**Signature:** `gen::accumulate<T>(genT, genTFromT, minSize, maxSize)`

**Example:**

```cpp
// generate initial value
auto baseGen = gen::interval(0, 1000);
// generate a value based on previous value, return only the last one
Generator<int> gen = gen::accumulate<int>(
    baseGen,
    [](const int& num) {
        // Generate next number between 0.5x and 1.5x of the previous
        // Ensure lower bound is not negative if num is small
        int lower = std::max(0, num / 2);
        int upper = num + num / 2;
        return gen::interval(lower, upper);
    },
    2 /* min sequence length */,
    10 /* max sequence length */
);
```

#### `gen::aggregate<T>`

Generates a sequence of values where each depends on the previous, and yields the *entire sequence* as a `std::vector<T>`.

**Signature:** `gen::aggregate<T>(genT, genTFromT, minSize, maxSize)`

**Example:**

```cpp
// generate initial value
auto baseGen = gen::interval(100, 1000); // Ensure initial value is not too small for division
// generate list of values where each depends on the previous
Generator<vector<int>> gen = gen::aggregate<int>(
    baseGen,
    [](int num) {
        // Generate next number between 0.5x and 1.5x of the previous
        int lower = std::max(0, num / 2);
        int upper = num + num / 2;
        return gen::interval(lower, upper);
    },
    2 /* min sequence length */,
    10 /* max sequence length */
);
```

**See also:** [StatefulTesting.md](StatefulTesting.md) for testing stateful systems with sequences of actions

&nbsp;

## Utility Methods in Standard Generators

Standard generator objects (like those returned by `gen::*`, `Arbi<T>`, `gen::construct<T>`, and the combinators themselves) provide convenient member functions that mirror the standalone combinator functions. These methods allow for fluent chaining of operations directly on the generator object. The underlying type is typically `Generator<T>`, representing a function `(Random&) -> Shrinkable<T>` (aliased as `GenFunction<T>`).

These methods have equivalent standalone counterparts and can be continuously chained:

| Method | Result Type | Equivalent Standalone Combinator |
|--------|-------------|----------------------------------|
| `Generator<T>::filter(filterer)` | `Generator<T>` | [`gen::filter<T>`](#genfiltert) |
| `Generator<T>::map<U>(mapper)` | `Generator<U>` | [`gen::transform<T,U>`](#gentransformtu) |
| `Generator<T>::flatMap<U>(genUFromT)` | `Generator<U>` | [`gen::derive<T,U>`](#genderivetu) |
| `Generator<T>::pairWith<U>(genUFromT)` | `Generator<std::pair<T,U>>` | [`gen::dependency<T,U>`](#gendependencytu) |
| `Generator<T>::tupleWith<U>(genUFromT)` | `Generator<std::tuple<T,U>>` | [`gen::chain<T,U>`](#genchaintsu) |
| `Generator<std::tuple<Ts...>>::tupleWith<U>(genUFromTuple)` | `Generator<std::tuple<Ts...,U>>` | [`gen::chain<std::tuple<Ts...>,U>`](#genchaintsu) |

### `.map<U>(mapper)`

Equivalent to calling `gen::transform<T,U>(*this, mapper)`. Transforms the output of the current generator using the provided `mapper` function.

**Example:**

```cpp
// generator for strings representing arbitrary integers
auto numStringGen = gen::int32().map<std::string>([](const int& num) {
    return std::to_string(num);
});
// which is equivalent to:
auto numStringGenEquivalent = gen::transform<int, std::string>(
    gen::int32(),
    [](const int& num) {
        return std::to_string(num);
    }
);
```

**See also:** [Transforming or Mapping](#transforming-or-mapping)

### `.filter(filterer)`

Equivalent to calling `gen::filter<T>(*this, filterer)`. Applies a predicate to filter the values produced by the current generator.

**Example:**

```cpp
// two equivalent ways to generate random even numbers
auto evenGen = gen::int32().filter([](const int& num) {
    return num % 2 == 0;
});

auto evenGenEquivalent = gen::filter<int>(gen::int32(), [](const int& num) {
    return num % 2 == 0;
});
```

**See also:** [Applying Constraints](#applying-constraints)

### `.flatMap<U>(genUFromT)`

Equivalent to calling `gen::derive<T, U>(*this, genUFromT)`. Based on the generated value of the current generator (type `T`), it uses the `genUFromT` function to obtain a *new* generator (`Generator<U>`), which is then used to produce the final value. This allows the characteristics of the resulting generator `U` to depend on the intermediate value `T`.

**Example:**

```cpp
// Generate a string whose maximum size depends on a generated integer
auto stringGen = gen::interval(1, 50).flatMap<std::string>([](const int& maxSize) {
    auto sizedStringGen = gen::string();
    sizedStringGen.setMaxSize(maxSize); // Configure the string generator
    return sizedStringGen; // Return the configured generator
});
```

**See also:** [Deriving or Flat-mapping](#deriving-or-flat-mapping)

### `.pairWith<U>(genUFromT)` and `.tupleWith<U>(genUFromT)`

Chain the current generator with another one where the second depends on the first. `.pairWith<U>` is equivalent to `gen::dependency<T, U>(*this, genUFromT)`, and `.tupleWith<U>` is equivalent to `gen::chain<T, U>(*this, genUFromT)`. If the current generator produces a `std::tuple<Ts...>`, `.tupleWith<U>` is equivalent to `gen::chain<Ts..., U>(*this, genUFromTuple)`.

**Example:**

```cpp
// Chain multiple dependent generators using tupleWith
auto complexGen = gen::boolean().tupleWith<int>([](const bool& isEven) -> Generator<int> {
    if (isEven)
        // If bool is true, generate an even integer
        return gen::int32().filter([](const int& value) {
            return value % 2 == 0;
        });
    else
        // If bool is false, generate an odd integer
        return gen::int32().filter([](const int& value) {
            return value % 2 != 0;
        });
}).tupleWith<std::string>([](const std::tuple<bool, int>& pair) -> Generator<std::string> {
    // Generate a string whose size depends on the generated integer (second element)
    int size = std::get<1>(pair);
    // Ensure size is non-negative for the string generator
    size = std::max(0, std::abs(size) % 100); // Example: Limit size reasonably
    auto stringGen = gen::string();
    stringGen.setSize(size);
    return stringGen;
}); // Results in Generator<tuple<bool, int, string>>
```

**Note:** `tupleWith` automatically chains generators, increasing the tuple size by one element at each step (`Generator<bool>` -> `Generator<tuple<bool, int>>` -> `Generator<tuple<bool, int, string>>` in the example above).

**See also:** [Values with Dependencies](#values-with-dependencies)

---

## Related Topics

* [Generators.md](Generators.md) - Comprehensive reference for all built-in generators
* [Arbitrary.md](Arbitrary.md) - Default generators for common types
* [CustomGenerator.md](CustomGenerator.md) - Creating custom generators with built-in constraints
* [Walkthrough.md](Walkthrough.md) - Step-by-step guide for creating property tests
* [PropertyAPI.md](PropertyAPI.md) - API reference for property-based tests
* [StatefulTesting.md](StatefulTesting.md) - Testing stateful systems with sequences of actions

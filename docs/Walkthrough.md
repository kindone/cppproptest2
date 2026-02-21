# Creating Your First Property Test: A Step-by-Step Walkthrough

This guide walks you through creating property-based tests with `cppproptest` from scratch. We'll start with a simple example and gradually build up to more complex scenarios.

## Prerequisites

Before starting, make sure you have:

- `cppproptest` integrated into your project (see [Getting Started](GettingStarted.md))
- A test framework (Google Test is used in these examples)
- Basic understanding of C++ lambdas

## Step 1: Your First Property Test

Let's start with the simplest possible property test - testing that addition is commutative.

### Complete Example

```cpp
#include "proptest/proptest.hpp"
#include "<gtest/gtest.h>"

using namespace proptest;

TEST(Arithmetic, AdditionIsCommutative)
{
    forAll([](int a, int b) -> bool {
        return a + b == b + a;
    });
}
```

### Breaking It Down

1. **Includes**:

    - `proptest/proptest.hpp` - Main header with all property testing functionality
    - `<gtest/gtest.h>` - Google Test integration

2. **Using namespace**: `using namespace proptest;` gives you access to `forAll`, `property`, `gen`, etc.

3. **Test structure**: This is a standard Google Test `TEST` macro. The property test runs inside it.

4. **The `forAll` function**: This is the simplest way to run a property test. It:

    - Takes a callable (here, a lambda) that defines your property
    - Automatically generates random inputs for each parameter
    - Runs the property multiple times (default: 1000 runs)
    - Reports failures if the property doesn't hold

   See [Property API Reference](PropertyAPI.md) for complete details on `forAll` and other property testing functions.

5. **The property function**:

    - Parameters `int a, int b` - these will be randomly generated
    - Return type `bool` - `true` means the property holds, `false` means it fails
    - Body: `return a + b == b + a;` - the property we're testing

### Running the Test

When you run this test, `cppproptest` will:

- Generate 1000 random pairs of integers
- Call your lambda with each pair
- Verify that `a + b == b + a` for all pairs
- If any pair fails, it will report the failure and attempt to 'shrink' it to a simpler counterexample (see [Shrinking](Shrinking.md) for details)

## Step 2: Using Assertions Instead of Return Values

Instead of returning `bool`, you can use assertions. This is often more readable and provides better error messages on failures:

```cpp
TEST(Arithmetic, AdditionIsCommutativeWithAssertions)
{
    forAll([](int a, int b) {
        PROP_ASSERT_EQ(a + b, b + a);
    });
}
```

**Key differences:**

- No return type needed (implicitly `void`)
- `PROP_ASSERT_EQ` provides detailed failure messages
- Test case stops immediately on first failure (fatal assertion)

**Available assertion macros:**

- `PROP_ASSERT_EQ(A, B)` - Assert equality
- `PROP_ASSERT_NE(A, B)` - Assert not equal
- `PROP_ASSERT_LT(A, B)` - Assert less than
- `PROP_ASSERT_LE(A, B)` - Assert less than or equal
- `PROP_ASSERT_GT(A, B)` - Assert greater than
- `PROP_ASSERT_GE(A, B)` - Assert greater than or equal
- `PROP_EXPECT_*` - Non-fatal versions (continues testing on failure)

See [Property API Reference - Using Assertions](PropertyAPI.md#using-assertions) for the complete list of assertion macros.

## Step 3: Specifying Custom Generators

By default, `forAll` uses built-in generators (arbitraries) for each parameter type. You can specify custom generators to control the input domain. See [Generators](Generators.md) for a complete list of available generators and [Arbitrary](Arbitrary.md) for details on default generators.

```cpp
TEST(Arithmetic, AdditionWithCustomRange)
{
    forAll([](int a, int b) {
        PROP_ASSERT_EQ(a + b, b + a);
    }, gen::interval(0, 100), gen::interval(0, 100));
}
```

**What changed:**

- Added two generator arguments: `gen::interval(0, 100)`
- `a` and `b` will now only be generated in the range [0, 100]
- This is useful for testing specific ranges or avoiding problematic values

**Some common generator functions:**

- `gen::interval(min, max)` - Generate integers in a range
- `gen::elementOf(values...)` - Generate one of the specified values
- `gen::just(value)` - Always generate the same value
- `gen::string()` - Generate random strings
- `gen::vector<T>()` - Generate random vectors

**Learn more:**

- [Generators](Generators.md) - Complete guide to all built-in generators
- [Combinators](Combinators.md) - Combine generators to create new ones (e.g., `map`, `filter`, `flatMap`)
- [Gen Namespace](GenNamespace.md) - Overview of the `gen` namespace
- [Generator Examples](GeneratorExamples.md) - Real-world examples of generator usage

## Step 4: Testing a Real Function

Let's test an actual function from your codebase. Suppose you have a function that reverses a string:

```cpp
// Your code (e.g., in a header file)
std::string reverseString(const std::string& str);

// Your test
TEST(StringUtils, ReverseTwiceIsIdentity)
{
    forAll([](const std::string& original) {
        std::string reversed = reverseString(original);
        std::string reversedTwice = reverseString(reversed);
        PROP_ASSERT_EQ(original, reversedTwice);
    });
}
```

**Property being tested:** Reversing a string twice should yield the original string.

**What's happening:**

- `original` is a randomly generated string (see [Generators - String Generators](Generators.md#string-generators) for string generation options)
- We reverse it once, then reverse the result
- We verify the final result equals the original

## Step 4.5: Understanding Parameter Types

In the examples so far, we've used different parameter styles - value types like `int a` and const references like `const std::string& original`. Both work identically with `cppproptest` while the latter is often preferred to avoid copy-construction.

**Value types vs const references:**

- **Value types**: `int`, `std::string`, `std::vector<int>` - Parameters are passed by value
- **Const references**: `const int&`, `const std::string&`, `const std::vector<int>&` - Parameters are passed by const reference

**How generators are selected:**
The library automatically selects the appropriate generator based on the underlying type, regardless of whether you use value or const reference:

```cpp
// These are equivalent - both use Arbi<int>
forAll([](int a) { ... });
forAll([](const int& a) { ... });

// These are equivalent - both use Arbi<std::string>
forAll([](std::string s) { ... });
forAll([](const std::string& s) { ... });
```

**When to use which:**

- **Use value types** for simple, cheap-to-copy types: `int`, `bool`, `char`, `double`, etc.
- **Use const references** for larger or expensive-to-copy types: `std::string`, `std::vector<T>`, `std::map<K,V>`, custom structs, etc.

**Example showing both styles:**

```cpp
TEST(ParameterTypes, ValueAndReference)
{
    // Value type for simple int
    forAll([](int value) {
        PROP_ASSERT_GE(value * value, 0);
    });

    // Const reference for string (avoids copying)
    forAll([](const std::string& str) {
        PROP_ASSERT_EQ(str.length(), str.length());
    });

    // Mixed: const reference for vector, value for int
    forAll([](const std::vector<int>& vec, int index) {
        if (index < vec.size()) {
            PROP_ASSERT_GE(vec[index], INT_MIN);
        }
    });
}
```

The generator selection happens automatically - you don't need to specify different generators for value vs const reference types.

## Step 5: Using the `property()` API for More Control

The `property()` function gives you more control and flexibility:

```cpp
TEST(StringUtils, ReverseWithPropertyAPI)
{
    auto prop = property([](const std::string& original) {
        std::string reversed = reverseString(original);
        std::string reversedTwice = reverseString(reversed);
        PROP_ASSERT_EQ(original, reversedTwice);
    });

    // Configure and run
    prop.setNumRuns(500).forAll();
}
```

You can also set a maximum duration to limit how long the test runs:

```cpp
TEST(StringUtils, ReverseWithTimeLimit)
{
    auto prop = property([](const std::string& original) {
        std::string reversed = reverseString(original);
        std::string reversedTwice = reverseString(reversed);
        PROP_ASSERT_EQ(original, reversedTwice);
    });

    // Run for at most 5 seconds, or until 1000 runs complete (whichever comes first)
    prop.setMaxDurationMs(5000).setNumRuns(1000).forAll();
}
```

You can also use `setConfig()` to configure multiple options at once:

```cpp
TEST(StringUtils, ReverseWithSetConfig)
{
    auto prop = property([](const std::string& original) {
        std::string reversed = reverseString(original);
        std::string reversedTwice = reverseString(reversed);
        PROP_ASSERT_EQ(original, reversedTwice);
    });

    // Batch configuration using setConfig()
    prop.setConfig({
        .seed = 12345,
        .numRuns = 500,
        .maxDurationMs = 5000
    }).forAll();
}
```

**Capability of `property()`:**

with a `Property` object obtained from `property()` higher order function, you can perform various test combinations with custom configurations, using the same property function.

- Can test with random examples: `.forAll()`
    - Can set number of runs: `.setNumRuns(500)` - stops after this many runs
    - Can set maximum duration: `.setMaxDurationMs(5000)` - stops after this many milliseconds (whichever limit is reached first)
    - Can set random seed: `.setSeed(12345)` for reproducibility
    - Can batch configure: `.setConfig({.seed = 123, .numRuns = 500})` - configure multiple options at once (C++20)
- Can test specific examples: `.example("hello")`
- Can test all possible combinations: `.matrix({"a", "ab", "abc"})`

**Note:** When both `setNumRuns()` and `setMaxDurationMs()` are set, the test stops when either limit is reached - whichever comes first.

### Alternative: Configure `forAll()` directly (C++20)

Instead of using `property().setX().forAll()`, you can configure `forAll()` directly using designated initializers (C++20). This provides a cleaner syntax for simple configuration cases:

```cpp
TEST(StringUtils, ReverseWithForAllConfig)
{
    // Configure forAll directly with seed and number of runs
    forAll([](const std::string& original) {
        std::string reversed = reverseString(original);
        std::string reversedTwice = reverseString(reversed);
        PROP_ASSERT_EQ(original, reversedTwice);
    }, {
        .seed = 12345,
        .numRuns = 500
    });
}
```

You can also combine configuration with explicit generators:

```cpp
TEST(StringUtils, ReverseWithConfigAndGenerators)
{
    forAll([](const std::string& original) {
        std::string reversed = reverseString(original);
        PROP_ASSERT_EQ(original.length(), reversed.length());
    }, {
        .seed = 0,
        .numRuns = 100,
        .maxDurationMs = 5000
    }, gen::string(gen::character(), 0, 100));
}
```

**When to use which:**

- Use `forAll()` with configuration `{ ... }` for simple, one-off configurations
- Use `property().setConfig({ ... }).forAll()` or `property().setX().setY().forAll()` when you need to reuse the property object to chain multiple operations (like `.example()` or `.matrix()`)

See [Property API Reference](PropertyAPI.md) for complete details on configuration options and methods.

## Step 6: Testing Multiple Properties Together

You can test multiple related properties in one test:

```cpp
TEST(StringUtils, ReverseProperties)
{
    // Property 1: Reversing twice is identity
    forAll([](const std::string& s) {
        PROP_ASSERT_EQ(s, reverseString(reverseString(s)));
    });

    // Property 2: Reversed string has same length
    forAll([](const std::string& s) {
        PROP_ASSERT_EQ(s.length(), reverseString(s).length());
    });

    // Property 3: Reversing empty string yields empty string
    // For specific examples, use property().example() instead
}
```

Or better, use the `property().example()` for specific cases:

```cpp
TEST(StringUtils, ReverseEdgeCases)
{
    auto prop = property([](const std::string& s) {
        PROP_ASSERT_EQ(s.length(), reverseString(s).length());
    });

    // Test specific examples
    prop.example("");
    prop.example("a");
    prop.example("ab");
    prop.example("hello world");
}
```

See [Property API Reference](PropertyAPI.md) for details on `.example()` and `.matrix()` methods.

## Step 7: Testing with Multiple Parameters

Properties can have multiple parameters with different types:

```cpp
TEST(Container, VectorOperations)
{
    forAll([](const std::vector<int>& vec, int value) {
        // Property: Adding then removing should preserve size (if value wasn't present)
        auto vecCopy = vec;
        auto originalSize = vecCopy.size();

        vecCopy.push_back(value);
        PROP_ASSERT_EQ(vecCopy.size(), originalSize + 1);

        // Remove the value we just added
        vecCopy.pop_back();
        PROP_ASSERT_EQ(vecCopy.size(), originalSize);
    });
}
```

**Note:** Each parameter type needs a generator. Built-in types like `int`, `std::string`, `std::vector<T>` have default generators, so you don't need to specify them unless you want custom ranges. Notice we use `const std::vector<int>&` (const reference) for the vector and `int` (value) for the integer - both value types and const reference types work identically (see [Step 4.5](#step-45-understanding-parameter-types) for details).

## Step 8: Using Statistics to Understand Test Coverage

You can collect statistics about the generated inputs:

```cpp
TEST(Statistics, CollectingStats)
{
    forAll([](int value) {
        PROP_STAT(value > 0);  // Track how often value > 0
        PROP_STAT(value < 0);  // Track how often value < 0
        PROP_STAT(value == 0);  // Track how often value == 0

        // Your actual property test
        PROP_ASSERT_GE(value * value, 0);  // Square is always non-negative
    });
}
```

**`PROP_STAT(condition)`:**

- Tracks how often the condition is true or false
- Doesn't fail the test if false
- Useful for understanding input distribution
- Statistics are printed at the end of the test run
- The key is automatically derived from the condition expression by the precompiler

**`PROP_TAG(KEY, VALUE)`:**

- Similar to `PROP_STAT`, but allows you to specify a custom key name
- Tracks how often the VALUE is true or false with your custom KEY
- Useful when you want more descriptive or grouped statistics
- Doesn't fail the test if VALUE is false
- Statistics are printed at the end of the test run

**`PROP_CLASSIFY(condition, KEY, VALUE)`:**

- Conditionally tags test cases only when the condition is true
- Only applies the tag when the condition evaluates to true (unlike `PROP_TAG` which tracks both true and false)
- Useful for categorizing test cases into groups based on conditions
- Doesn't fail the test if condition is false
- Statistics are printed at the end of the test run

```cpp
TEST(Statistics, UsingTags)
{
    forAll([](int value) {
        PROP_STAT(value > 0);  // Key: "value > 0"
        PROP_TAG("positive", value > 0);  // Key: "positive" (custom)
        PROP_TAG("in range", value >= 0 && value < 100);  // Key: "in range" (custom)

        PROP_ASSERT_GE(value * value, 0);
    });
}

TEST(Statistics, UsingClassify)
{
    forAll([](int x, int y) {
        // Only tag when conditions are true
        PROP_CLASSIFY(x == y, "relationship", "equal");
        PROP_CLASSIFY(x > y, "relationship", "greater");
        PROP_CLASSIFY(x < y, "relationship", "less");
        PROP_CLASSIFY(x % 2 == 0, "parity", "even");
        PROP_CLASSIFY(y % 2 == 0, "parity", "even");

        PROP_ASSERT_EQ(x + y, y + x);
    });
}
```

See [Test Strategies](TestStrategies.md) for more advanced testing techniques and statistics usage.

## Step 9: Integrating with Google Test Macros

To make property test failures properly reported as failures in your Google Test, use the provided macros:

```cpp
TEST(StringUtils, ReverseWithGTestIntegration)
{
    // This will properly fail the Google Test if property fails
    EXPECT_FOR_ALL([](const std::string& s) {
        PROP_ASSERT_EQ(s, reverseString(reverseString(s)));
    });

    // Or use ASSERT_FOR_ALL for fatal failures
    ASSERT_FOR_ALL([](const std::string& s) {
        PROP_ASSERT_EQ(s.length(), reverseString(s).length());
    });

    // For Cartesian product tests, use EXPECT_MATRIX or ASSERT_MATRIX
    EXPECT_MATRIX([](const std::string& a, const std::string& b) {
        PROP_ASSERT_EQ(a.length() + b.length(), (a + b).length());
    }, {"", "a", "ab"}, {"", "x", "xy"});
}
```

**Difference:**

- `forAll()` and `matrix()` return `bool` - you need to wrap them: `EXPECT_TRUE(forAll(...))` or `EXPECT_TRUE(matrix(...))`
- `EXPECT_FOR_ALL(...)` - Shorthand for `EXPECT_TRUE(forAll(...))`
- `ASSERT_FOR_ALL(...)` - Shorthand for `ASSERT_TRUE(forAll(...))`
- `EXPECT_MATRIX(callable, ...lists)` - Shorthand for `EXPECT_TRUE(matrix(callable, ...lists))` (Cartesian product)
- `ASSERT_MATRIX(callable, ...lists)` - Shorthand for `ASSERT_TRUE(matrix(callable, ...lists))` (Cartesian product)

See [Property API Reference - Google Test Integration Macros](PropertyAPI.md#google-test-integration-macros) for more details.

## Step 10: Handling Failures and Shrinking

When a property fails, `cppproptest` automatically tries to shrink the counterexample to a simpler one:

```cpp
TEST(Example, FailingProperty)
{
    forAll([](const std::vector<int>& vec) {
        // This will fail for non-empty vectors
        PROP_ASSERT_EQ(vec.size(), 0);
    });
}
```

**What happens on failure:**

1. Test finds a counterexample (e.g., `vec = [42, 17, 99]`)
2. Shrinking process tries to simplify it
3. May find a simpler counterexample (e.g., `vec = [0]` or `vec = [1]`)
4. Reports the minimal counterexample

See [Shrinking](Shrinking.md) for a detailed explanation of how shrinking works and how to customize it.

**Reproducing failures:**
When a test fails, `cppproptest` reports a seed. You can reproduce it:

```cpp
// Using property() API
auto prop = property([](const std::vector<int>& vec) {
    PROP_ASSERT_EQ(vec.size(), 0);
});

prop.setSeed(12345678).forAll();  // Use the seed from failure

// Or using forAll() with configuration (C++20)
forAll([](const std::vector<int>& vec) {
    PROP_ASSERT_EQ(vec.size(), 0);
}, {
    .seed = 12345678
});
```

Or set it via environment variable:
```bash
PROPTEST_SEED=12345678 ./your_test
```

## Step 11: Testing Complex Data Structures

You can test properties involving complex data structures:

```cpp
TEST(DataStructures, MapProperties)
{
    forAll([](const std::map<int, std::string>& m, int key, const std::string& value) {
        auto mCopy = m;
        mCopy[key] = value;

        // Property: After insertion, map should contain the key
        PROP_ASSERT_NE(mCopy.find(key), mCopy.end());
        PROP_ASSERT_EQ(mCopy[key], value);
    });
}
```

**Some built-in generators for containers:**

- `gen::vector<T>()` - Vectors
- `gen::map<K, V>()` - Maps
- `gen::set<T>()` - Sets
- `gen::pair<A, B>()` - Pairs
- `gen::tuple<...>()` - Tuples

See [Generators - Container Generators](Generators.md#container-generators) for complete details and configuration options.

## Step 12: Creating Custom Generators

For custom types, you may want to create custom generators for multiple uses:

```cpp
struct Point {
    int x, y;
};

// Define a generator for Point using combinators
auto pointGen = gen::tuple(gen::interval(-100, 100), gen::interval(-100, 100))
    .map<Point>([](const std::tuple<int, int>& t) {
        return Point{std::get<0>(t), std::get<1>(t)};
    });

TEST(Geometry, PointProperties)
{
    forAll([](const Point& p) {
        // Property: Distance from origin is non-negative
        double dist = std::sqrt(p.x * p.x + p.y * p.y);
        PROP_ASSERT_GE(dist, 0.0);
    }, pointGen);
}
```

See [Custom Generator](CustomGenerator.md) for detailed instructions on creating custom generators, and [Combinators](Combinators.md) for using generator combinators like `map`, `filter`, and `flatMap`.

See [Arbitraries](Arbitrary.md) for more on arbitraries(default generators) and how to make a new arbitrary out of a custom generator.

## Common Patterns and Best Practices

For more testing strategies and patterns, see [Test Strategies](TestStrategies.md).

### Pattern 1: Round-trip Properties
Test that encoding/decoding, serialization/deserialization, etc. preserve data:

```cpp
TEST(Codec, EncodeDecodeRoundTrip)
{
    forAll([](const MyData& data) {
        auto encoded = encode(data);
        auto decoded = decode(encoded);
        PROP_ASSERT_EQ(data, decoded);
    });
}
```

### Pattern 2: Invariant Properties
Test that operations preserve invariants:

```cpp
TEST(Container, SizeInvariant)
{
    forAll([](std::vector<int>& vec, int value) {
        size_t originalSize = vec.size();
        vec.push_back(value);
        PROP_ASSERT_EQ(vec.size(), originalSize + 1);
    });
}
```

### Pattern 3: Idempotency
Test that applying an operation twice is the same as once:

```cpp
TEST(Operations, Idempotent)
{
    forAll([](MyObject& obj) {
        auto original = obj;
        normalize(obj);
        normalize(obj);  // Apply twice
        PROP_ASSERT_EQ(obj, original);  // Should be same as after first normalize
    });
}
```

### Pattern 4: Commutativity and Associativity
Test mathematical properties:

```cpp
TEST(Math, AdditionProperties)
{
    // Commutativity
    forAll([](int a, int b) {
        PROP_ASSERT_EQ(a + b, b + a);
    });

    // Associativity
    forAll([](int a, int b, int c) {
        PROP_ASSERT_EQ((a + b) + c, a + (b + c));
    });
}
```

## Troubleshooting

### Problem: Compile error about missing generator
**Solution:** Specify a generator explicitly or create a custom one:
```cpp
forAll([](MyCustomType t) { ... }, myCustomTypeGenerator);
```
See [Custom Generator](CustomGenerator.md) for creating generators for custom types.

### Problem: Test runs too slowly
**Solution:** Reduce number of runs, set a maximum duration, or limit input size:
```cpp
property([](...) { ... })
    .setNumRuns(100)  // Reduce from default 1000
    .forAll();

// Or limit by time instead of number of runs
property([](...) { ... })
    .setMaxDurationMs(5000)  // Stop after 5 seconds
    .forAll();

// Or use both - stops when either limit is reached
property([](...) { ... })
    .setNumRuns(1000)
    .setMaxDurationMs(10000)  // Stop after 10 seconds or 1000 runs, whichever comes first
    .forAll();
```

### Problem: Need to test specific values
**Solution:** Use `.example()` or `.matrix()`:
```cpp
auto prop = property([](int x) { ... });
prop.example(0);
prop.example(INT_MAX);
prop.matrix({-1, 0, 1});
```

### Problem: Property fails but counterexample is too complex
**Solution:** Shrinking should help automatically, but you can also constrain generators:
```cpp
forAll([](int x) { ... }, gen::interval(-10, 10));  // Smaller range
```
See [Shrinking](Shrinking.md) for details on how shrinking works and how to customize it.

## Next Steps

Now that you understand the basics, explore:

- [Generators](Generators.md) - Learn about all available generators
- [Combinators](Combinators.md) - Combine generators to create new ones
- [Shrinking](Shrinking.md) - Understand how counterexamples are simplified
- [Stateful Testing](StatefulTesting.md) - Test state machines and sequences
- [Property API Reference](PropertyAPI.md) - Complete API documentation

## Summary

Creating a property test involves:

1. **Define the property** - Write a function/lambda that expresses what should always be true
2. **Choose generators** - Specify how to generate inputs (or use defaults)
3. **Run the test** - Use `forAll()` or `property().forAll()`
4. **Handle failures** - Use reported seeds to reproduce and fix issues

The key insight: Instead of testing specific examples, you're testing universal properties that should hold for all (or most) inputs in a domain.

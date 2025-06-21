# The `gen` Namespace

The `gen` namespace provides convenient collection for all built-in generators and combinators in `cppproptest`. The `gen` namespace is designed to provide a unified, user-friendly interface for accessing generators and combinators. Instead of writing verbose built-in arbitrary names like `Arbi<int>()`, you can use more intuitive names like `gen::int32()`.

**Note:** All functions in the `gen` namespace are aliases to the original `proptest` namespace functions. The original names remain available for backward compatibility.

## Generators

To use the `gen` namespace, simply include the main header and use the `gen::` prefix:

```cpp
#include "proptest/proptest.hpp"

using namespace proptest;

// Use gen namespace for cleaner syntax
auto intGen = gen::int32();
auto stringGen = gen::string();
auto vectorGen = gen::vector<int>();
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

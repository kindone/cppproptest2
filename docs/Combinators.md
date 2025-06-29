# Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. Many of them come from ideas and best practices of functional programming. They can be chained, as each combinator receives one or more existing generators as arguments and returns a new generator.

**Note:** All combinators are now available in the `proptest::gen` namespace. You can use them with the `gen::` prefix (e.g., `gen::just`, `gen::elementOf`) or include the backward compatibility aliases from the `proptest` namespace. The examples in this documentation use the modern `gen::` prefix.

While you can read this document sequentially, you might want to use the following table to quickly find a suitable combinator for your use case:

| Generator/Combinator                                                                                                   | Purpose                                              | Examples                                                                      |
|------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------|-------------------------------------------------------------------------------|
| [`gen::just<T>`](#constants)                                                                                                | Generate just a constant                             | `0` or `"1337"`                                                               |
| [`gen::elementOf<T>`](#selecting-from-values)                                                                               | Generate a value within constants                    | a prime number under 100                                                      |
| [`gen::set<T>`](Generators.md#built-in-arbitraries)                                                                   | Generate a list of unique values                     | `{3,5,1}` but not `{3,5,5}`                                                   |
| [`gen::interval<T>`, `gen::integers<T>`](#integers-and-intervals)                                                                | Generate a value within numeric range of values      | a number within `1`~`9999`                                                    |
| [`gen::pairOf<T1,T2>`, `gen::tupleOf<Ts...>`](#pair-and-tuples)                                                                  | Generate a pair or a tuple of different types        | a `pair<int, string>`                                                         |
| [`gen::unionOf<T>` (`gen::oneOf<T>`)](#selecting-from-generators)                                                                | Union multiple generators                            | `20~39` or `60~79` combined                                                   |
| [`gen::transform<T,U>`](#transforming-or-mapping)                                                                           | Transform into another type or a value               | `"0"` or `"1.4"` (a number as a string)                                       |
| [`gen::construct<T,ARGS...>`](#constructing-an-object)                                                                      | Generate a struct or a class object                  | a `Rectangle` object with width and height                                    |
| [`gen::filter` (`gen::suchThat`)](#applying-constraints)                                                                         | Apply constraints to generated values                | an even natural number (`n % 2 == 0`)                                         |
| [`gen::dependency`, `gen::chain`](#values-with-dependencies), [`pairWith`, `tupleWith`](#utility-methods-in-standard-generators) | Generate values with dependencies or relationships   | a rectangle where `width == height * 2`                                       |
| [`gen::aggregate`, `gen::accumulate`](#aggregation-or-accumulation-of-values)                                                    | Generate a value based on previously generated value | a sequence of numbers where each is between 0.5x and 1.5x of its predecessor |

&nbsp;

## Basic Generator Combinators

### Constants

* `gen::just<T>(T*)`, `gen::just<T>(T)`, `gen::just<T>(shared_ptr<T>)`: always generates a specific value. A shared pointer can be used for non-copyable types.
* `gen::lazy<T>(function<T()>)`: generates a value by calling a function
    ```cpp
    auto zeroGen = gen::just(0); // template argument is optional if type is deducible
    auto oneGen = gen::lazy<int>([]() { return 1; });
    ```

### Selecting from constants

You may want to randomly choose from a specific list of values.

* `gen::elementOf<T>(val1, ..., valN)`: generates a value of type `T` by randomly choosing one from the provided constant values.
    ```cpp
    // generates a prime number under 50
    auto primeGen = gen::elementOf<int>(2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47);
    ```

    * `gen::elementOf` can receive optional probabilitistic weights (`0 < weight <= 1`, the sum of weights should ideally be 1.0 but must not exceed 1.0) for values. If a weight is unspecified for a value, the remaining probability (1.0 minus the sum of specified weights) is distributed evenly among the values without specified weights.
    `weightedVal(<value>, <weight>)` is used to annotate the desired weight.

    ```cpp
    // generates 2, 5, or 10 with specified probabilities
    //   weight for 10 automatically becomes 1.0 - 0.8 - 0.15 == 0.05
    gen::elementOf<int>(weightedVal(2, 0.8), weightedVal(5, 0.15), 10);
    ```

### Integers and intervals

Some utility generators for integers are provided

* `gen::interval<INT_TYPE>(min, max)`: generates an integer type(e.g. `uint16_t`) in the closed interval `[min, max]`.
* `gen::integers<INT_TYPE(from, count)`: generates an integer type starting from `from`
    ```cpp
    gen::interval<int64_t>(1, 28);
    gen::interval(1, 48); // template type argument can be ommitted if the input type(`int`) is the same as the output type.
    gen::interval(1L, 48L); // template type argument can be ommitted if the input type(`int`) is the same as the output type.
    gen::interval(0, 10); // generates an integer in {0, ..., 10}
    gen::interval('A', 'Z'); // generates a char of uppercase alphabet
    gen::integers(0, 10); // generates an integer in {0, ..., 9}
    gen::integers(1, 10); // generates an integer in {1, ..., 10}
    ```
* `gen::natural<INT_TYPE>(max)`: generates a positive integer up to `max`(inclusive)
* `gen::natural<INT_TYPE>(max)`: generates a positive integer (greater than 0) up to `max` (inclusive).
* `gen::nonNegative<INT_TYPE>(max)`: : generates zero or a positive integer up to `max`(inclusive)
* `gen::nonNegative<INT_TYPE>(max)`: generates zero or a positive integer (>= 0) up to `max` (inclusive).

### Pair and Tuples

Generators for different types can be combined to produce a `std::pair` or `std::tuple`.

* `gen::pairOf<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on the results of generators `gen1` and `gen2`.

    ```cpp
    auto pairGen = gen::pairOf(Arbi<int>(), Arbi<std::string>());
    ```

* `gen::tupleOf<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

    ```cpp
    auto tupleGen = gen::tupleOf(Arbi<int>(), Arbi<std::string>(), Arbi<double>());
    ```

&nbsp;

## Advanced Generator Combinators

### Selecting from generators

You can combine multiple generators of the same type into a single generator that randomly selects one of the underlying generators to produce a value. This can be considered as taking a *union* of generators.

* `gen::oneOf<T>(gen1, ..., genN)`: generates a value of type `T` by randomly choosing one of the provided generators (`gen1` to `genN`, which must all generate type `T`).

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
    auto evenGen = gen::oneOf<int>(gen::interval(0, 10), gen::interval(100, 1000), gen::interval(10000, 100000));
    auto rangeGen = gen::oneOf<int>(gen::interval(0, 10), gen::interval(100, 1000), gen::interval(10000, 100000));
    ```

    * `gen::oneOf` can receive optional probabilitistic weights (`0 < weight <= 1`, the sum of weights should ideally be 1.0 but must not exceed 1.0) for generators. If a weight is unspecified for a generator, the remaining probability (1.0 minus the sum of specified weights) is distributed evenly among the generators without specified weights.
    `weightedGen(<generator>, <weight>)` is used to annotate the desired weight.

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000] with specified probabilities
    auto evenGen = gen::oneOf<int>(weightedGen(gen::interval(0, 10), 0.8), weightedGen(gen::interval(100, 1000), 0.15), gen::interval(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
    auto rangeGen = gen::oneOf<int>(weightedGen(gen::interval(0, 10), 0.8), weightedGen(gen::interval(100, 1000), 0.15), gen::interval(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
    ```

* `gen::unionOf<T>` is an alias of `gen::oneOf<T>`

### Constructing an object

You can generate an object of a class or a struct type `T`, by calling a matching constructor of `T`.

* `gen::construct<T, ARG1, ..., ARGN>([gen1, ..., genM])`: generates an object of type `T` by calling its constructor that matches the signature `(ARG1, ..., ARGN)`. Custom generators `gen1`,..., `genM` can be supplied for generating arguments. If `M < N`, the remaining `N - M` arguments are generated using their default `Arbi` generators (`Arbi<ARG_{M+1}>`, etc.).

    ```cpp
    struct Coordinate {
        Coordinate(int x, int y) {
        // ...
        }
    };
    // ...
    auto coordinateGen1 = gen::construct<Coordinate, int, int>(gen::interval(-10, 10), gen::interval(-20, 20));
    auto coordinateGen2 = gen::construct<Coordinate, int, int>(gen::interval(-10, 10)); // y is generated with Arbi<int>
    ```

### Applying constraints

You can add a filtering condition to a generator to restrict the generated values to satisfy a certain constraint.

* `gen::filter<T>(gen, condition_predicate)`: generates a value of type `T` using the base generator `gen`, but only yields values that satisfy the `condition_predicate` (i.e., for which the predicate returns `true`). If the predicate returns `false`, the generation is retried with a new value from `gen`.

    ```cpp
    // generates even numbers
    auto evenGen = gen::filter<int>(gen::int32(),[](const int& num) {
        return num % 2 == 0;
    });
    ```

* `gen::suchThat<T>`: an alias of `gen::filter`

### Transforming or mapping

You can transform an existing generator to create a new generator by providing a transformer function. This is equivalent to *mapping* in a functional programming context.

* `gen::transform<T,U>(gen, transformer)`: generates type `U` based on a generator for type `T` (`gen`), using the `transformer` function which converts a value of type `const T&` to type `U`.

    ```cpp
    // generates strings from integers (e.g. "0", "1", ... , "-16384")
    auto numStringGen = gen::transform<int, std::string>(Arbi<int>(),[](const int& num) {
        return std::to_string(num);
    });
    ```

### Deriving or flat-mapping

Another combinator that resembles `gen::transform` is `gen::derive`. This is equivalent to *flat-mapping* or *binding* in functional programming. The key difference from `gen::transform<T, U>` is that the function provided to `gen::derive` returns a *new generator* (`Generator<U>`) based on the intermediate value of type `T`, rather than just transforming the value into a `U`. This allows for more complex, context-dependent generation logic.

* `gen::derive<T, U>(genT, genUGen)`: derives a new generator for type `U`. It first generates a value of type `T` using `genT`. Then, it passes this value to `genUGen`, which is a function that returns a `Generator<U>`. This returned generator is then used to produce the final value of type `U`.

    ```cpp
    // generates a string something like "KOPZZFASF", "ghnpqpojv", or "49681002378", ... that consists of only uppercase/lowercase alphabets/numeric characters.
    auto stringGen = gen::derive<int, std::string>(gen::integers(0, 2), [](const int& num) {
        if(num == 0)
            return Arbi<std::string>(gen::interval('A', 'Z'));
        else if(num == 1)
            return Arbi<std::string>(gen::interval('a', 'z'));
        else // num == 2
            return Arbi<std::string>(gen::interval('0', '9'));
    });
    ```

Following table compares `gen::transform` and `gen::derive`:

| Combinator       | transformer signature       | Result type    |
|------------------|-----------------------------|----------------|
| `gen::transform<T,U>` | `function<U(T)>`            | `Generator<U>` |
| `gen::derive<T,U>`    | `function<Generator<U>(T)>` | `Generator<U>` |



### Values with dependencies

You may want to include dependencies between generated values. Two combinators facilitate this: one generates a `std::pair`, and the other generates a `std::tuple`.

* `gen::dependency<T,U>(genT, genUgen)`: generates a `std::pair<T,U>`. It first uses `genT` to generate a value of type `T`. This value is then passed to `genUgen`, which is a function that returns a `Generator<U>`. This returned generator produces the second element of the pair. This effectively creates a generator for a pair where the second item depends on the first.

    ```cpp
    auto sizeAndVectorGen = gen::dependency<int, std::vector<bool>>(Arbi<bool>(), [](const int& num) {
        auto vectorGen = Arbi<std::vector<int>>();
        vectorGen.maxLen = num;
        // generates a vector with maximum size of num
        return vectorGen;
    });

    auto nullableIntegers = gen::dependency<bool, int>(Arbi<bool>(), [](bool& isNull) {
        if(isNull)
        return gen::just<int>(0);
        else
        return gen::fromTo<int>(10, 20);
    });
    // Example 1: Generate a size and a vector of that size
    auto sizeAndVectorGen = gen::dependency<int, std::vector<bool>>(gen::interval(1, 100), [](const int& size) {
        // Assume Arbi<std::vector<T>> has a method like setSize
        auto vectorGen = Arbi<std::vector<bool>>();
        vectorGen.setSize(size);
        // generates a vector with exactly 'size' elements
        return vectorGen;
    });

    // Example 2: Generate a bool and an int depending on the bool
    auto nullableIntegerGen = gen::dependency<bool, int>(Arbi<bool>(), [](const bool& isNull) -> Generator<int> {
        if (isNull)
            // If bool is true, return a generator for 0
            return gen::just(0);
        else
            // If bool is false, return a generator for ints between 10 and 20
            return gen::interval(10, 20);
    });
    ```

* `gen::chain<Ts..., U>(genTuple, genUFromTuple)`: similar to `gen::dependency`, but operates on tuples. It takes a generator `genTuple` for `std::tuple<Ts...>` and a function `genUFromTuple`. This function receives the generated tuple (`const std::tuple<Ts...>&`) and returns a `Generator<U>`. The final result is a generator for `std::tuple<Ts..., U>`. `gen::chain` can be repeatedly applied to build tuples with multiple dependent elements.

    ```cpp
    auto yearMonthGen = gen::tupleOf(gen::fromTo(0, 9999), gen::fromTo(1,12));
    // number of days of month depends on month (28~31 days) and year (whether it's a leap year)
    auto yearMonthDayGen = gen::chain<std::tuple<int, int>, int>(yearMonthGen, [](std::tuple<int,int>& yearMonth) {
        int year = std::get<0>(yearMonth);
        int month = std::get<1>(yearMonth);
        if(monthHas31Days(month)) {
            return gen::interval(1, 31);
        }
        else if(monthHas30Days(month)) {
            return gen::interval(1, 30);
        }
        else { // february has 28 or 29 days
            if(isLeapYear(year))
            return gen::interval(1, 29);
        else
            return gen::interval(1, 28);
        }
    }); // yearMonthDayGen generates std::tuple<int, int, int> of (year, month, day)
    // Assuming helper functions: isLeapYear(int), monthHas31Days(int), monthHas30Days(int) exist
    auto yearMonthGen = gen::tupleOf(gen::interval(1900, 2100), gen::interval(1, 12));
    // number of days in a month depends on the month and whether the year is a leap year
    auto yearMonthDayGen = gen::chain<int, int, int>(yearMonthGen, [](const std::tuple<int, int>& yearMonth) -> Generator<int> {
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
    }); // yearMonthDayGen generates std::tuple<int, int, int> representing (year, month, day)
    ```

Actually, you can often achieve a similar goal using the `gen::filter` combinator:

```cpp
    // generate any year,month,day combination
    auto yearMonthDayGen = gen::tupleOf(gen::fromTo(0, 9999), gen::fromTo(1,12), gen::fromTo(1,31));
    // apply filter
    auto validYearMonthDayGen = yearMonthDayGen.filter([](std::tuple<int,int,int>& ymd) {
    // generate any year, month, day combination first
    auto anyYearMonthDayGen = gen::tupleOf(gen::interval(1900, 2100), gen::interval(1, 12), gen::interval(1, 31));
    // then apply filter to keep only valid dates
    auto validYearMonthDayGen = anyYearMonthDayGen.filter([](const std::tuple<int, int, int>& ymd) {
        int year = std::get<0>(ymd);
        int month = std::get<1>(ymd);
        int day = std::get<2>(ymd);
        if(monthHas31Days(month) && day <= 31)
            return true;
        else if(monthHas30Days(month) && day <= 30)
            return true;
        else { // february has 28 or 29 days
            if(isLeapYear(year) && day <= 29)
                return true;
            else
                return day <= 28;
        }
        return isValidDate(year, month, day); // Assuming isValidDate helper function
    });
```

However, using `gen::filter` for generating values with complex dependencies can be inefficient. If the constraints are tight, many generated values might be discarded before a valid one is found, leading to performance issues. In such cases, `gen::dependency` or `gen::chain` are often preferable as they construct valid values directly.


### Aggregation or Accumulation of Values

You may want to generate a sequence of values where each value depends on the previously generated one(s). A common example is generating a sequence of chess moves, where knowing the previous move is crucial for generating the next valid move.
This can be achieved with `gen::aggregate` or `gen::accumulate`.
Both of the combinators take base generator in the form of `Generator<T>` as the first argument and a factory that takes a value of type `T` and returns `Generator<T>`, as the second argument.

While `gen::accumulate` generates a single value, `gen::aggregate` generates a vector of values at each generation.

| Combinator                                                      | Result type            | Remark |
|-----------------------------------------------------------------|------------------------|--------|
| `gen::accumulate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize)` | `Generator<T>`         |        |
| `gen::aggregate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize)`  | `Generator<vector<T>>` |        |
Both combinators take:
1. A base generator (`genT`) for the initial value.
2. A function (`genTFromT`) that receives the *previously generated value* of type `T` and returns a `Generator<T>` for the *next* value in the sequence.
3. Minimum (`minSize`) and maximum (`maxSize`) lengths for the generated sequence.

`gen::accumulate` generates a sequence of values based on the previous one, but ultimately yields only the *final* value generated in that sequence. `gen::aggregate` generates a similar sequence but yields the *entire sequence* as a `std::vector<T>`.

| Combinator                                      | Result type            | Remark                                                     |
|-------------------------------------------------|------------------------|------------------------------------------------------------|
| `gen::accumulate<T>(genT, genTFromT, minSize, maxSize)` | `Generator<T>`         | Generates sequence, returns the **last** element           |
| `gen::aggregate<T>(genT, genTFromT, minSize, maxSize)`  | `Generator<vector<T>>` | Generates sequence, returns the **entire sequence** vector |

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
        2 /* min sequence length */, 10 /* max sequence length */);
```

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
        2 /* min sequence length */, 10 /* max sequence length */);
```

## Utility Methods in Standard Generators

Standard generator objects (like those returned by `Arbi<T>`, `gen::construct<T>`, and the combinators themselves) often provide convenient member functions that mirror the standalone combinator functions. These methods allow for fluent chaining of operations directly on the generator object. The underlying type is typically `Generator<T>`, representing a function `(Random&) -> Shrinkable<T>` (aliased as `GenFunction<T>`). These methods have equivalent standalone counterparts, as shown below:

| Decorated method                             | Result type                      | Equivalent Standalone combinator |
|----------------------------------------------|----------------------------------|----------------------------------|
| `Generator<T>::filter`                       | `Generator<T>`                   | `gen::filter<T>`                      |
| `Generator<T>::map<U>`                       | `Generator<U>`                   | `gen::transform<T,U>`                 |
| `Generator<T>::flatMap<U>`                   | `Generator<U>`                   | `gen::derive<T,U>`                    |
| `Generator<T>::pairWith<U>`                  | `Generator<std::pair<T,U>>`      | `gen::dependency<T,U>`                |
| `Generator<T>::tupleWith<U>`                 | `Generator<std::tuple<T,U>>`     | `gen::chain<T,U>`                     |
| `Generator<std::tuple<Ts...>>::tupleWith<U>` | `Generator<std::tuple<Ts...,U>>` | `gen::chain<std::tuple<Ts...>,U>`     |

These functions and methods can be continuously chained.

* `.map<U>(mapper)`: Equivalent to calling `gen::transform<T,U>(*this, mapper)`. It transforms the output of the current generator using the provided `mapper` function.

    ```cpp
    // generator for strings of arbitrary number
    Arbi<int>().map<std::string>([](int &num) {
        return std::to_string(num);
    });
    // this is equivalent to:
    gen::transform<int, std::string>(Arbi<int>(), [](int &num) {
        return std::to_string(num);
    });
    // generator for strings representing arbitrary integers
    auto numStringGen = gen::int32().map<std::string>([](const int& num) {
        return std::to_string(num);
    });
    // which is equivalent to:
    auto numStringGenEquivalent = gen::transform<int, std::string>(Arbi<int>(), [](const int& num) {
        return std::to_string(num);
    });
    ```

* `.filter(filterer)`: Equivalent to calling `gen::filter<T>(*this, filterer)`. Applies a predicate to filter the values produced by the current generator.

    ```cpp
    // two equivalent ways to generate random even numbers
    auto evenGen = Arbi<int>().filter([](int& num) {
        return num % 2 == 0;
    });

    auto evenGenEquivalent = gen::filter<int>(Arbi<int>(),[](const int& num) {
        return num % 2 == 0;
    });
    ```

* `.flatMap<U>(genUFromT)`: Equivalent to calling `gen::derive<T, U>(*this, genUFromT)`. Based on the generated value of the current generator (type `T`), it uses the `genUFromT` function to obtain a *new* generator (`Generator<U>`), which is then used to produce the final value. This allows the characteristics of the resulting generator `U` to depend on the intermediate value `T`.

    ```cpp
    auto stringGen = Arbi<int>().flatMap<std::string>([](int& num) {
        auto genString = Arbi<std::string>();
        genString.setMaxSize(num);
        return genString;
    });
    // Generate a string whose maximum size depends on a generated integer
    auto stringGen = gen::interval(1, 50).flatMap<std::string>([](const int& maxSize) {
        // Assume Arbi<string> has setMaxSize or similar
        auto sizedStringGen = Arbi<std::string>();
        sizedStringGen.setMaxSize(maxSize); // Configure the string generator
        return sizedStringGen; // Return the configured generator
    });
    ```

* `.pairWith<U>(genUFromT)` or `tupleWith<U>(genUFromT)`: Chain the current generator with another one where the second depends on the first. Equivalent to `gen::dependency<T, U>(*this, genUFromT)` and `gen::chain<T, U>(*this, genUFromT)` respectively. If the current generator produces a `std::tuple<Ts...>`, `tupleWith<U>` is equivalent to `gen::chain<Ts..., U>(*this, genUFromTuple)`.
    ```cpp
    Arbi<bool>().tupleWith<int>([](bool& isEven) {
        if(isEven)
            return Arbi<int>().filter([](int& value) {
    // Chain multiple dependent generators using tupleWith
    auto complexGen = Arbi<bool>().tupleWith<int>([](const bool& isEven) -> Generator<int> {
        if (isEven)
            // If bool is true, generate an even integer
            return Arbi<int>().filter([](const int& value) {
                return value % 2 == 0;
            });
        else
            // If bool is false, generate an odd integer
            return Arbi<int>().filter([](const int& value) {
                return value % 2 != 0; // Fixed: Use != 0 for odd
            });
    }).tupleWith<std::string>([](std::tuple<bool, int>& tuple) {
    }).tupleWith<std::string>([](const std::tuple<bool, int>& pair) -> Generator<std::string> {
        // Generate a string whose size depends on the generated integer (second element)
        int size = std::get<1>(pair);
        // Ensure size is non-negative for the string generator
        size = std::max(0, std::abs(size) % 100); // Example: Limit size reasonably
        auto stringGen = gen::string();
        // Assume Arbi<string> has setSize or similar
        stringGen.setSize(size);
        return stringGen;
    }); // Results in Generator<tuple<bool, int, string>>
    ```

    Notice `tupleWith` can automatically chain a tuple generator of `n` parameters into a tuple generator of `n+1` parameters (`bool` generator -> `tuple<bool, int>` generator -> `tuple<bool, int, string>` generator in above example)
    Notice how `tupleWith` automatically chains generators, increasing the tuple size by one element at each step (`Generator<bool>` -> `Generator<tuple<bool, int>>` -> `Generator<tuple<bool, int, string>>` in the example above).
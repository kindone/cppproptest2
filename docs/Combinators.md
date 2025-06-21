# Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. Many of them come from ideas and best practices of functional programming. They can be chained, as each combinator receives one or more existing generators as arguments and returns a new generator.

While you can read this document sequentially, you might want to use the following table to quickly find a suitable combinator for your use case:

| Generator/Combinator                                                                                                   | Purpose                                              | Examples                                                                      |
|------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------|-------------------------------------------------------------------------------|
| [`just<T>`](#constants)                                                                                                | Generate just a constant                             | `0` or `"1337"`                                                               |
| [`elementOf<T>`](#selecting-from-values)                                                                               | Generate a value within constants                    | a prime number under 100                                                      |
| [`Arbi<set<T>>`](Generators.md#built-in-arbitraries)                                                                   | Generate a list of unique values                     | `{3,5,1}` but not `{3,5,5}`                                                   |
| [`interval<T>`, `integers<T>`](#integers-and-intervals)                                                                | Generate a value within numeric range of values      | a number within `1`~`9999`                                                    |
| [`pairOf<T1,T2>`, `tupleOf<Ts...>`](#pair-and-tuples)                                                                  | Generate a pair or a tuple of different types        | a `pair<int, string>`                                                         |
| [`unionOf<T>` (`oneOf<T>`)](#selecting-from-generators)                                                                | Union multiple generators                            | `20~39` or `60~79` combined                                                   |
| [`transform<T,U>`](#transforming-or-mapping)                                                                           | Transform into another type or a value               | `"0"` or `"1.4"` (a number as a string)                                       |
| [`construct<T,ARGS...>`](#constructing-an-object)                                                                      | Generate a struct or a class object                  | a `Rectangle` object with width and height                                    |
| [`filter` (`suchThat`)](#applying-constraints)                                                                         | Apply constraints to generated values                | an even natural number (`n % 2 == 0`)                                         |
| [`dependency`, `chain`](#values-with-dependencies), [`pairWith`, `tupleWith`](#utility-methods-in-standard-generators) | Generate values with dependencies or relationships   | a rectangle where `width == height * 2`                                       |
| [`aggregate`, `accumulate`](#aggregation-or-accumulation-of-values)                                                    | Generate a value based on previously generated value | a sequence of numbers where each is between 0.5x and 1.5x of its predecessor |

&nbsp;

## Basic Generator Combinators

### Constants

* `just<T>(T*)`, `just<T>(T)`, `just<T>(shared_ptr<T>)`: always generates a specific value. A shared pointer can be used for non-copyable types.
* `lazy<T>(function<T()>)`: generates a value by calling a function
    ```cpp
    auto zeroGen = just(0); // template argument is optional if type is deducible
    auto oneGen = lazy<int>([]() { return 1; });
    ```

### Selecting from constants

You may want to randomly choose from a specific list of values.

* `elementOf<T>(val1, ..., valN)`: generates a value of type `T` by randomly choosing one from the provided constant values.
    ```cpp
    // generates a prime number under 50
    auto primeGen = elementOf<int>(2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47);
    ```

    * `elementOf` can receive optional probabilitistic weights (`0 < weight <= 1`, the sum of weights should ideally be 1.0 but must not exceed 1.0) for values. If a weight is unspecified for a value, the remaining probability (1.0 minus the sum of specified weights) is distributed evenly among the values without specified weights.
    `weightedVal(<value>, <weight>)` is used to annotate the desired weight.

    ```cpp
    // generates 2, 5, or 10 with specified probabilities
    //   weight for 10 automatically becomes 1.0 - 0.8 - 0.15 == 0.05
    elementOf<int>(weightedVal(2, 0.8), weightedVal(5, 0.15), 10);
    ```

### Integers and intervals

Some utility generators for integers are provided

* `interval<INT_TYPE>(min, max)`: generates an integer type(e.g. `uint16_t`) in the closed interval `[min, max]`.
* `integers<INT_TYPE(from, count)`: generates an integer type starting from `from`
    ```cpp
    interval<int64_t>(1, 28);
    interval(1, 48); // template type argument can be ommitted if the input type(`int`) is the same as the output type.
    interval(1L, 48L); // template type argument can be ommitted if the input type(`int`) is the same as the output type.
    interval(0, 10); // generates an integer in {0, ..., 10}
    interval('A', 'Z'); // generates a char of uppercase alphabet
    integers(0, 10); // generates an integer in {0, ..., 9}
    integers(1, 10); // generates an integer in {1, ..., 10}
    ```
* `natural<INT_TYPE>(max)`: generates a positive integer up to `max`(inclusive)
* `natural<INT_TYPE>(max)`: generates a positive integer (greater than 0) up to `max` (inclusive).
* `nonNegative<INT_TYPE>(max)`: : generates zero or a positive integer up to `max`(inclusive)
* `nonNegative<INT_TYPE>(max)`: generates zero or a positive integer (>= 0) up to `max` (inclusive).

### Pair and Tuples

Generators for different types can be combined to produce a `std::pair` or `std::tuple`.

* `pairOf<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on the results of generators `gen1` and `gen2`.

    ```cpp
    auto pairGen = pairOf(Arbi<int>(), Arbi<std::string>());
    ```

* `tupleOf<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

    ```cpp
    auto tupleGen = tupleOf(Arbi<int>(), Arbi<std::string>(), Arbi<double>());
    ```

&nbsp;

## Advanced Generator Combinators

### Selecting from generators

You can combine multiple generators of the same type into a single generator that randomly selects one of the underlying generators to produce a value. This can be considered as taking a *union* of generators.

* `oneOf<T>(gen1, ..., genN)`: generates a value of type `T` by randomly choosing one of the provided generators (`gen1` to `genN`, which must all generate type `T`).

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
    auto evenGen = oneOf<int>(interval(0, 10), interval(100, 1000), interval(10000, 100000));
    auto rangeGen = oneOf<int>(interval(0, 10), interval(100, 1000), interval(10000, 100000));
    ```

    * `oneOf` can receive optional probabilitistic weights (`0 < weight <= 1`, the sum of weights should ideally be 1.0 but must not exceed 1.0) for generators. If a weight is unspecified for a generator, the remaining probability (1.0 minus the sum of specified weights) is distributed evenly among the generators without specified weights.
    `weightedGen(<generator>, <weight>)` is used to annotate the desired weight.

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000] with specified probabilities
    auto evenGen = oneOf<int>(weightedGen(interval(0, 10), 0.8), weightedGen(interval(100, 1000), 0.15), interval(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
    auto rangeGen = oneOf<int>(weightedGen(interval(0, 10), 0.8), weightedGen(interval(100, 1000), 0.15), interval(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
    ```

* `unionOf<T>` is an alias of `oneOf<T>`

### Constructing an object

You can generate an object of a class or a struct type `T`, by calling a matching constructor of `T`.

* `construct<T, ARG1, ..., ARGN>([gen1, ..., genM])`: generates an object of type `T` by calling its constructor that matches the signature `(ARG1, ..., ARGN)`. Custom generators `gen1`,..., `genM` can be supplied for generating arguments. If `M < N`, the remaining `N - M` arguments are generated using their default `Arbi` generators (`Arbi<ARG_{M+1}>`, etc.).

    ```cpp
    struct Coordinate {
        Coordinate(int x, int y) {
        // ...
        }
    };
    // ...
    auto coordinateGen1 = construct<Coordinate, int, int>(interval(-10, 10), interval(-20, 20));
    auto coordinateGen2 = construct<Coordinate, int, int>(interval(-10, 10)); // y is generated with Arbi<int>
    ```

### Applying constraints

You can add a filtering condition to a generator to restrict the generated values to satisfy a certain constraint.

* `filter<T>(gen, condition_predicate)`: generates a value of type `T` using the base generator `gen`, but only yields values that satisfy the `condition_predicate` (i.e., for which the predicate returns `true`). If the predicate returns `false`, the generation is retried with a new value from `gen`.

    ```cpp
    // generates even numbers
    auto evenGen = filter<int>(Arbi<int>(),[](const int& num) {
        return num % 2 == 0;
    });
    ```

* `suchThat<T>`: an alias of `filter`

### Transforming or mapping

You can transform an existing generator to create a new generator by providing a transformer function. This is equivalent to *mapping* in a functional programming context.

* `transform<T,U>(gen, transformer)`: generates type `U` based on a generator for type `T` (`gen`), using the `transformer` function which converts a value of type `const T&` to type `U`.

    ```cpp
    // generates strings from integers (e.g. "0", "1", ... , "-16384")
    auto numStringGen = transform<int, std::string>(Arbi<int>(),[](const int& num) {
        return std::to_string(num);
    });
    ```

### Deriving or flat-mapping

Another combinator that resembles `transform` is `derive`. This is equivalent to *flat-mapping* or *binding* in functional programming. The key difference from `transform<T, U>` is that the function provided to `derive` returns a *new generator* (`Generator<U>`) based on the intermediate value of type `T`, rather than just transforming the value into a `U`. This allows for more complex, context-dependent generation logic.

* `derive<T, U>(genT, genUGen)`: derives a new generator for type `U`. It first generates a value of type `T` using `genT`. Then, it passes this value to `genUGen`, which is a function that returns a `Generator<U>`. This returned generator is then used to produce the final value of type `U`.

    ```cpp
    // generates a string something like "KOPZZFASF", "ghnpqpojv", or "49681002378", ... that consists of only uppercase/lowercase alphabets/numeric characters.
    auto stringGen = derive<int, std::string>(integers(0, 2), [](const int& num) {
        if(num == 0)
            return Arbi<std::string>(interval('A', 'Z'));
        else if(num == 1)
            return Arbi<std::string>(interval('a', 'z'));
        else // num == 2
            return Arbi<std::string>(interval('0', '9'));
    });
    ```

Following table compares `transform` and `derive`:

| Combinator       | transformer signature       | Result type    |
|------------------|-----------------------------|----------------|
| `transform<T,U>` | `function<U(T)>`            | `Generator<U>` |
| `derive<T,U>`    | `function<Generator<U>(T)>` | `Generator<U>` |



### Values with dependencies

You may want to include dependencies between generated values. Two combinators facilitate this: one generates a `std::pair`, and the other generates a `std::tuple`.

* `dependency<T,U>(genT, genUgen)`: generates a `std::pair<T,U>`. It first uses `genT` to generate a value of type `T`. This value is then passed to `genUgen`, which is a function that returns a `Generator<U>`. This returned generator produces the second element of the pair. This effectively creates a generator for a pair where the second item depends on the first.

    ```cpp
    auto sizeAndVectorGen = dependency<int, std::vector<bool>>(Arbi<bool>(), [](const int& num) {
        auto vectorGen = Arbi<std::vector<int>>();
        vectorGen.maxLen = num;
        // generates a vector with maximum size of num
        return vectorGen;
    });

    auto nullableIntegers = dependency<bool, int>(Arbi<bool>(), [](bool& isNull) {
        if(isNull)
        return just<int>(0);
        else
        return fromTo<int>(10, 20);
    });
    // Example 1: Generate a size and a vector of that size
    auto sizeAndVectorGen = dependency<int, std::vector<bool>>(interval(1, 100), [](const int& size) {
        // Assume Arbi<std::vector<T>> has a method like setSize
        auto vectorGen = Arbi<std::vector<bool>>();
        vectorGen.setSize(size);
        // generates a vector with exactly 'size' elements
        return vectorGen;
    });

    // Example 2: Generate a bool and an int depending on the bool
    auto nullableIntegerGen = dependency<bool, int>(Arbi<bool>(), [](const bool& isNull) -> Generator<int> {
        if (isNull)
            // If bool is true, return a generator for 0
            return just(0);
        else
            // If bool is false, return a generator for ints between 10 and 20
            return interval(10, 20);
    });
    ```

* `chain<Ts..., U>(genTuple, genUFromTuple)`: similar to `dependency`, but operates on tuples. It takes a generator `genTuple` for `std::tuple<Ts...>` and a function `genUFromTuple`. This function receives the generated tuple (`const std::tuple<Ts...>&`) and returns a `Generator<U>`. The final result is a generator for `std::tuple<Ts..., U>`. `chain` can be repeatedly applied to build tuples with multiple dependent elements.

    ```cpp
    auto yearMonthGen = tupleOf(fromTo(0, 9999), fromTo(1,12));
    // number of days of month depends on month (28~31 days) and year (whether it's a leap year)
    auto yearMonthDayGen = chain<std::tuple<int, int>, int>(yearMonthGen, [](std::tuple<int,int>& yearMonth) {
        int year = std::get<0>(yearMonth);
        int month = std::get<1>(yearMonth);
        if(monthHas31Days(month)) {
            return fromTo(1, 31);
        }
        else if(monthHas30Days(month)) {
            return fromTo(1, 30);
        }
        else { // february has 28 or 29 days
            if(isLeapYear(year))
            return fromTo(1, 29);
        else
            return fromTo(1, 28);
        }
    }); // yearMonthDayGen generates std::tuple<int, int, int> of (year, month, day)
    // Assuming helper functions: isLeapYear(int), monthHas31Days(int), monthHas30Days(int) exist
    auto yearMonthGen = tupleOf(interval(1900, 2100), interval(1, 12));
    // number of days in a month depends on the month and whether the year is a leap year
    auto yearMonthDayGen = chain<int, int, int>(yearMonthGen, [](const std::tuple<int, int>& yearMonth) -> Generator<int> {
        int year = std::get<0>(yearMonth);
        int month = std::get<1>(yearMonth);
        if (monthHas31Days(month)) {
            return interval(1, 31);
        } else if (monthHas30Days(month)) {
            return interval(1, 30);
        } else { // February (month == 2)
            if (isLeapYear(year))
                return interval(1, 29);
            else
                return interval(1, 28);
        }
    }); // yearMonthDayGen generates std::tuple<int, int, int> representing (year, month, day)
    ```

Actually, you can often achieve a similar goal using the `filter` combinator:

```cpp
    // generate any year,month,day combination
    auto yearMonthDayGen = tupleOf(fromTo(0, 9999), fromTo(1,12), fromTo(1,31));
    // apply filter
    auto validYearMonthDayGen = yearMonthDayGen.filter([](std::tuple<int,int,int>& ymd) {
    // generate any year, month, day combination first
    auto anyYearMonthDayGen = tupleOf(interval(1900, 2100), interval(1, 12), interval(1, 31));
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

However, using `filter` for generating values with complex dependencies can be inefficient. If the constraints are tight, many generated values might be discarded before a valid one is found, leading to performance issues. In such cases, `dependency` or `chain` are often preferable as they construct valid values directly.


### Aggregation or Accumulation of Values

You may want to generate a sequence of values where each value depends on the previously generated one(s). A common example is generating a sequence of chess moves, where knowing the previous move is crucial for generating the next valid move.
This can be achieved with `aggregate` or `accumulate`.
Both of the combinators take base generator in the form of `Generator<T>` as the first argument and a factory that takes a value of type `T` and returns `Generator<T>`, as the second argument.

While `accumulate` generates a single value, `aggregate` generates a vector of values at each generation.

| Combinator                                                      | Result type            | Remark |
|-----------------------------------------------------------------|------------------------|--------|
| `accumulate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize)` | `Generator<T>`         |        |
| `aggregate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize)`  | `Generator<vector<T>>` |        |
Both combinators take:
1. A base generator (`genT`) for the initial value.
2. A function (`genTFromT`) that receives the *previously generated value* of type `T` and returns a `Generator<T>` for the *next* value in the sequence.
3. Minimum (`minSize`) and maximum (`maxSize`) lengths for the generated sequence.

`accumulate` generates a sequence of values based on the previous one, but ultimately yields only the *final* value generated in that sequence. `aggregate` generates a similar sequence but yields the *entire sequence* as a `std::vector<T>`.

| Combinator                                      | Result type            | Remark                                                     |
|-------------------------------------------------|------------------------|------------------------------------------------------------|
| `accumulate<T>(genT, genTFromT, minSize, maxSize)` | `Generator<T>`         | Generates sequence, returns the **last** element           |
| `aggregate<T>(genT, genTFromT, minSize, maxSize)`  | `Generator<vector<T>>` | Generates sequence, returns the **entire sequence** vector |

```cpp
    // generate initial value
    auto baseGen = interval(0, 1000);
    // generate a value based on previous value, return only the last one
    Generator<int> gen = accumulate<int>(
        baseGen,
        [](const int& num) {
            // Generate next number between 0.5x and 1.5x of the previous
            // Ensure lower bound is not negative if num is small
            int lower = std::max(0, num / 2);
            int upper = num + num / 2;
            return interval(lower, upper);
        },
        2 /* min sequence length */, 10 /* max sequence length */);
```

```cpp
    // generate initial value
    auto baseGen = interval(100, 1000); // Ensure initial value is not too small for division
    // generate list of values where each depends on the previous
    Generator<vector<int>> gen = aggregate<int>(
        baseGen,
        [](int num) {
            // Generate next number between 0.5x and 1.5x of the previous
            int lower = std::max(0, num / 2);
            int upper = num + num / 2;
            return interval(lower, upper);
        },
        2 /* min sequence length */, 10 /* max sequence length */);
```

## Utility Methods in Standard Generators

Standard generator objects (like those returned by `Arbi<T>`, `construct<T>`, and the combinators themselves) often provide convenient member functions that mirror the standalone combinator functions. These methods allow for fluent chaining of operations directly on the generator object. The underlying type is typically `Generator<T>`, representing a function `(Random&) -> Shrinkable<T>` (aliased as `GenFunction<T>`). These methods have equivalent standalone counterparts, as shown below:

| Decorated method                             | Result type                      | Equivalent Standalone combinator |
|----------------------------------------------|----------------------------------|----------------------------------|
| `Generator<T>::filter`                       | `Generator<T>`                   | `filter<T>`                      |
| `Generator<T>::map<U>`                       | `Generator<U>`                   | `transform<T,U>`                 |
| `Generator<T>::flatMap<U>`                   | `Generator<U>`                   | `derive<T,U>`                    |
| `Generator<T>::pairWith<U>`                  | `Generator<std::pair<T,U>>`      | `dependency<T,U>`                |
| `Generator<T>::tupleWith<U>`                 | `Generator<std::tuple<T,U>>`     | `chain<T,U>`                     |
| `Generator<std::tuple<Ts...>>::tupleWith<U>` | `Generator<std::tuple<Ts...,U>>` | `chain<std::tuple<Ts...>,U>`     |

These functions and methods can be continuously chained.

* `.map<U>(mapper)`: Equivalent to calling `transform<T,U>(*this, mapper)`. It transforms the output of the current generator using the provided `mapper` function.

    ```cpp
    // generator for strings of arbitrary number
    Arbi<int>().map<std::string>([](int &num) {
        return std::to_string(num);
    });
    // this is equivalent to:
    transform<int, std::string>(Arbi<int>(), [](int &num) {
        return std::to_string(num);
    });
    // generator for strings representing arbitrary integers
    auto numStringGen = Arbi<int>().map<std::string>([](const int& num) {
        return std::to_string(num);
    });
    // which is equivalent to:
    auto numStringGenEquivalent = transform<int, std::string>(Arbi<int>(), [](const int& num) {
        return std::to_string(num);
    });
    ```

* `.filter(filterer)`: Equivalent to calling `filter<T>(*this, filterer)`. Applies a predicate to filter the values produced by the current generator.

    ```cpp
    // two equivalent ways to generate random even numbers
    auto evenGen = Arbi<int>().filter([](int& num) {
        return num % 2 == 0;
    });

    auto evenGenEquivalent = filter<int>(Arbi<int>(),[](const int& num) {
        return num % 2 == 0;
    });
    ```

* `.flatMap<U>(genUFromT)`: Equivalent to calling `derive<T, U>(*this, genUFromT)`. Based on the generated value of the current generator (type `T`), it uses the `genUFromT` function to obtain a *new* generator (`Generator<U>`), which is then used to produce the final value. This allows the characteristics of the resulting generator `U` to depend on the intermediate value `T`.

    ```cpp
    auto stringGen = Arbi<int>().flatMap<std::string>([](int& num) {
        auto genString = Arbi<std::string>();
        genString.setMaxSize(num);
        return genString;
    });
    // Generate a string whose maximum size depends on a generated integer
    auto stringGen = interval(1, 50).flatMap<std::string>([](const int& maxSize) {
        // Assume Arbi<string> has setMaxSize or similar
        auto sizedStringGen = Arbi<std::string>();
        sizedStringGen.setMaxSize(maxSize); // Configure the string generator
        return sizedStringGen; // Return the configured generator
    });
    ```

* `.pairWith<U>(genUFromT)` or `tupleWith<U>(genUFromT)`: Chain the current generator with another one where the second depends on the first. Equivalent to `dependency<T, U>(*this, genUFromT)` and `chain<T, U>(*this, genUFromT)` respectively. If the current generator produces a `std::tuple<Ts...>`, `tupleWith<U>` is equivalent to `chain<Ts..., U>(*this, genUFromTuple)`.
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
        auto stringGen = Arbi<std::string>();
        // Assume Arbi<string> has setSize or similar
        stringGen.setSize(size);
        return stringGen;
    }); // Results in Generator<tuple<bool, int, string>>
    ```

    Notice `tupleWith` can automatically chain a tuple generator of `n` parameters into a tuple generator of `n+1` parameters (`bool` generator -> `tuple<bool, int>` generator -> `tuple<bool, int, string>` generator in above example)
    Notice how `tupleWith` automatically chains generators, increasing the tuple size by one element at each step (`Generator<bool>` -> `Generator<tuple<bool, int>>` -> `Generator<tuple<bool, int, string>>` in the example above).
# Building Custom Generator from Scratch

> **New to property-based testing?** Start with the [Walkthrough](Walkthrough.md) for a step-by-step guide. Before building a custom generator from scratch, consider combining existing [generators](Generators.md) with [combinators](Combinators.md)—[`.filter()`](Combinators.md#filterfilterer), [`.map<U>()`](Combinators.md#mapumapper), [`.flatMap<U>()`](Combinators.md#flatmapugenufromt), [`.pairWith<U>()`](Combinators.md#pairwithugenufromt-and-tuplewithugenufromt), or [gen::construct](Combinators.md#genconstructtargs) often suffice. Build from scratch only when you need generation logic that cannot be expressed by composing these.

&nbsp;

## When to Build from Scratch

Custom generators are useful when:

- You need values from a domain not covered by [built-in generators](Generators.md) or [Arbitraries](Arbitrary.md)
- Combinators ([filter](Combinators.md#filterfilterer), [map](Combinators.md#mapumapper), [flatMap](Combinators.md#flatmapugenufromt), [construct](Combinators.md#genconstructtargs)) cannot express your constraints or transformations
- You need custom shrinking behavior (see [Shrinking](Shrinking.md))
- You want to define a default generator for a type used across many tests (see [Arbitrary](Arbitrary.md))

&nbsp;

## `Generator<T>` and `Arbitrary<T>`

`Generator<T>` and [`Arbitrary<T>`](Arbitrary.md) are the standard generator types in `cppproptest`. Both share the same chainable utility methods ([`.filter()`](Combinators.md#filterfilterer), [`.map<U>()`](Combinators.md#mapumapper), [`.flatMap<U>()`](Combinators.md#flatmapugenufromt), etc.). `Generator<T>` is commonly the result of [combinators](Combinators.md); `Arbitrary<T>` is the default generator for a type. They are fully chainable—you can use any generator with `forAll` and chain methods as needed.

&nbsp;

## Building a Custom Generator

A generator in `cppproptest` is simply a callable with signature `(Random&) -> Shrinkable<T>`. Simplest way to make a shrinkable is to use `make_shrinkable<T>(value)` to wrap your value. This makes a shrinkable with no further shrinks. See [Shrinking](Shrinking.md) for details on `Shrinkable`.

You can wrap your callable with `Generator<T>` to decorate it as a standard generator with same chainable utility methods as built-in generators and [Arbitraries](Arbitrary.md):

```cpp
auto myIntGen = Generator<int>([](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
});

// Chain with .filter(), .map(), etc. like any other standard generators
auto evenGen = myIntGen.filter([](const int& value) {
    return value % 2 == 0;
});
```

&nbsp;

---

## Related Topics

- [Combinators](Combinators.md) - `.filter()`, `.map()`, `.flatMap()`, and other utility methods for transforming generators
- [Generators](Generators.md) - Built-in generators for primitives and containers
- [Arbitrary](Arbitrary.md) - Defining default generators for types used in `forAll` without explicit generator arguments
- [Shrinking](Shrinking.md) - How `Shrinkable` and `make_shrinkable` enable automated simplification of failing inputs
- [Property API](PropertyAPI.md) - Using custom generators with `forAll` and `property()`
- [Walkthrough](Walkthrough.md) - Step-by-step guide for creating property tests

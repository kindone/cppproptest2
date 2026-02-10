# cppproptest

<!-- ![cppproptest in a glance](images/overview.svg) -->
```kroki-d2
example: |cpp
// example-based test
MyEncoder encoder;
MyDecoder decoder;
auto encodedMsg = encoder.encode("dummy");
auto decodedMsg = decoder.decode(encodedMsg);
ASSERT_EQ("dummy", decodedMsg);
|

property: |cpp
// property-based test
forAll([](std::string original) {
  MyEncoder encoder;
  MyDecoder decoder;
  auto encodedMsg = encoder.encode(original);
  auto decodedMsg = decoder.decode(encodedMsg);
  ASSERT_EQ(original, decodedMsg);
};
|

example -> property : forAll()
```

`cppproptest` is a property-based testing library for C++. It focuses on usability with variety of features including:

* Out-of-box [generators](Generators.md) for primitives and standard containers
* Set of powerful [generator combinators](Combinators.md) for creating new generators from existing generators
* [Shrinking capability](Shrinking.md) for quickly finding root cause of a failure and automated debugging support
* [Stateful testing support](StatefulTesting.md) for testing state changes
* [Concurrency testing support](ConcurrencyTesting.md) for testing concurrent state changes

You can get started with `cppproptest` on this [page](GettingStarted.md). For a step-by-step guide to creating your first property test, see the [Walkthrough](Walkthrough.md).

&nbsp;

# Why Property-based Testing?

## Generalization and Abstraction

Property-based testing lets you write tests using general or universal ideas, instead of introducing specific examples or overly contrived scenarios. You can write tests focusing on specifications or requirements of the tested components.

### Clear Separation of Variants and Invariants

In reality, we often mix variants and invariants in our tests under typical *example-based tests*. This is one of the reasons why those tests become hard to maintain and difficult to read over time. Let's see following example-based test for an encoder and a decoder:

```cpp
// a text encoded and then decoded must be identical to original
MyEncoder encoder;
MyDecoder decoder;
auto encodedMsg = encoder.encode("Some dummy content that hopefully prove or disprove this works");
auto decodedMsg = decoder.decode(encodedMsg);
ASSERT_EQ("Some dummy content that hopefully prove or disprove this works", decodedMsg);
```

Even a simple test like this can be confusing, as some dummy string value seems like as if it's conveying significant meaning in the test but in fact it isn't. Turning this dummy value to a free variable would lead to greater generalization of the test:

```cpp hl_lines="1 8"
[](std::string originalMsg) {
    // a text encoded and then decoded must be identical to originalMsg
    MyEncoder encoder;
    MyDecoder decoder;
    auto encodedMsg = encoder.encode(originalMsg);
    auto decodedMsg = decoder.decode(encodedMsg);
    PROP_ASSERT_EQ(originalMsg, decodedMsg);
};
```

The new free variable `originalMsg` becomes the variant part (**input domain**) of the test while the rest becomes the invariant part.

In order to turn this into a concrete test run, we can feed in some random values for the free variable `originalMsg`. This can be achieved by enclosing this function with `forAll()` function, which calls the given test function (shown here in the form of lambda) many times with some randomly generated values for the function parameter `originalMsg`:

```cpp hl_lines="1 8"
forAll([](std::string originalMsg) {
    // a text encoded and then decoded must be identical to original
    MyEncoder encoder;
    MyDecoder decoder;
    auto encodedMsg = encoder.encode(originalMsg);
    auto decodedMsg = decoder.decode(encodedMsg);
    PROP_ASSERT_EQ(originalMsg, decodedMsg);
});
```

As a result, `forAll()` can essentially test the encoder and decoder against arbitrary input strings, by calling the property function hundreds of times (200 is the default number of runs) with some random strings, instead of relying on some dummy values. This let's you validate the property with various forms of a string.

With this approach, our test becomes more readable, gets easier to maintain, and expresses the test writer's intention more precisely.

## Convenience and Versatility

Property-based testing often provides with the convenient out-of-box **generators** and **generator combinators** (that can make new generators out of existing ones), and lets you to effortlessly define an input domain tailored for your test. You can specify your input domain using various notions - range, constraint, union, transformation, composition, and dependency, just to name a few.

Following example shows how certain input domain of string type can be specified using a built-in generator and generator combinators:

```kroki-d2
generator: "Custom String Generator"{

    arbitrary: |cpp
    gen::integer()
    |

    filter: |cpp
    .filter([] (int num) { return num % 2 == 0; })
    |

    map: |cpp
    .map([] (int num) {
        return "<" + std::to_string(num) + ">";
    });
    |

arbitrary -> filter : filter even numbers only
filter -> map : map to string enclosed with < and >

}

result1: |cpp
"<-232>"
|

result2: |cpp
"<0>"
|

result3: |cpp
"<493284>"
|

generator -> result1: generate {
  style: {
    stroke-dash: 3
  }
}

generator -> result2: generate {
  style: {
    stroke-dash: 3
  }
}

generator -> result3: generate {
  style: {
    stroke-dash: 3
  }
}

```


```cpp
auto stringGen = gen::integer()
    .filter([] (int& num) { return num % 2 == 0; }) // even numbers only
    .map([] (int& num) {
        return "<" + std::to_string(numStr) + ">"; // string like "<0>", ..., "<n>"
    });

forAll([](std::string original) {
    // "<-232>", <0>, <493284>, ... //
}, stringGen);
```

The `forAll` function automatically identifies number of parameters and their data types of the given property function. This mechanism allows any number of parameters of simple or complex data types to be used to define a property-based test.

<!-- ![overview](images/forall.svg)-->

```kroki-d2
forAll: |cpp
forAll([](string s, vector<string> v, map<int, string> m) {
  // Do stuff with s, v, and m
}, gen::string(1,4));
|

S: |cpp
Arbitrary<string> of size 1~4
|

V: |cpp
Arbitrary<vector<string>>
|

M: |cpp
Arbitrary<map<int, string>>
|

VS: |cpp
Arbitrary<string>
|

MI: |cpp
Arbitrary<int>
|

MS: |cpp
Arbitrary<string>
|

forAll -> S: string s
forAll -> V: vector<string> v
forAll -> M: map<int, string> m

V -> VS: "element: string"
M -> MI: "key: int"
M -> MS: "value: string"
```

Once the generators for each parameters are determined, it then automatically feeds in randomly generated values of those types to make a call to the given function as many times as desired. With this powerful generation engine, we can fully parameterize and randomize our tests with high flexibility but with much less effort.

With this framework, you don't need to care too much about *how* to test your requirements. Most of it is automatically done for you by the framework, letting you to focus on *what* to test, instead.

<!--
## Reusability and Scalability

```kroki-nomnoml
#.many: visual=note
#stroke: #1831ab
#fill: #f3f4f7;#f8f7f3
#lineWidth:2
#bendSize: 0.6
#arrowSize: 0.7
#fontSize: 10
#edgeMargin:3
#spacing:60
#direction: right
#gravity:2

[Namespace
  [<many>Arbitraries]
  [<many>Built-in ;Generators]
]

[Custom Generator 1]

[Custom Generator 2]

[New Arbitrary<T>]

[Namespace] -> apply combinator [Custom Generator 1]
[Custom Generator 1] -> apply combinator [Custom Generator 2]
[Custom Generator 2] define arbitrary for T -> [New Arbitrary<T>]

[New Arbitrary<T>] globally available --\> [Namespace]

```

-->


&nbsp;


# Further Topics and Details:

* [Getting Started with cppproptest](GettingStarted.md)
    * [Step-by-Step Walkthrough](Walkthrough.md)
* [Property API Reference](PropertyAPI.md)
* [Introduction to generators](Generators.md)
    * [Arbitraries, the default generators](Arbitrary.md)
    * [`gen` namespace - built-in generators and combinators](GenNamespace.md)
    * [Generating real-world inputs](GeneratorExamples.md)
* [Generator combinators for creating generators from existing ones](Combinators.md)
* [Counterexamples and notion of shrinking for automated debugging](Shrinking.md)
* [Printing facilities for custom data types](Printing.md)
* [Stateful testing with `cppproptest`](StatefulTesting.md)
* [Concurrency testing with `cppproptest`](ConcurrencyTesting.md)
* [Advanced mocking with `cppproptest` (work in progress)](Mocking.md)

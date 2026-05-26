# Concurrency Testing

Once you're familiar with [stateful testing](./StatefulTesting.md), you can get acquainted with concurrent stateful testing easily.
Concurrency testing performs interleaved state transitions using multiple threads in parallel. It allows us to see if any anomaly or breaking of concurrency requirement in the tested component.

Actually, a concurrency test is almost immediately achievable when you have prepared a stateful test for an object type.
Following depicts a concurrent test for `std::vector<int>` with `push_back(int)`, `pop_back()`, and `clear()` actions included.


```cpp

#include "proptest/proptest.hpp"
#include "proptest/statefultest.hpp"

using namespace proptest;
using namespace proptest::stateful;

// ...

auto pushBackGen = gen::int32().map<SimpleAction<std::vector<int>>>([](const int& value) {
    return SimpleAction<std::vector<int>>("PushBack", [value](std::vector<int>& obj) {
        obj.push_back(value);
    });
});

auto popBackGen = gen::just(SimpleAction<std::vector<int>>("PopBack", [](std::vector<int>& obj) {
    if (obj.empty())
        return;
    obj.pop_back();
}));

auto clearGen = gen::just(SimpleAction<std::vector<int>>("Clear", [](std::vector<int>& obj) {
    obj.clear();
}));

auto actionGen = gen::oneOf<SimpleAction<std::vector<int>>>(pushBackGen, popBackGen, clearGen);
auto concurrentProp = statefulProperty<std::vector<int>>(gen::vector<int>(), actionGen)
    .setMaxConcurrency(4);
concurrentProp.go();
```

You may have noticed that the above test would lead to exceptions or crashes, as `std::vector` is not made for concurrent writes unless some synchronization mechanism is present.

You can also add a post-check to be performed after each concurrent test run, by setting a post-check function to concurrent property with `setPostCheck()` method:

```cpp
concurrentProp.setPostCheck([](std::vector<int>& obj) {
    // ... post-check. perform some consistency check for obj
}).go();

// variant with a model
concurrentProp.setPostCheck([](std::vector<int>& obj, VectorModel& model) {
    // ... post-check. perform some consistency check for obj against model
}).go();
```

While you can perform checks in some of the actions, it's sometimes better to have a post-check instead. In concurrent tests, your model as well as the stateful object can be concurrently accessed. Adding synchronization primitives for model object can cause unintended serialization to occur on the stateful object, too. This is why a post-check comes handy, as you don't need to care about synchronization since it's performed after all actions are finished and threads are joined.

`setMaxConcurrency(0)` is the default and runs the property as a sequential stateful test. Values greater than `1` spawn rear worker threads after the front action sequence. `setOnActionStart` and `setOnActionEnd` run for all actions in this mode; callbacks must be thread-safe when `setMaxConcurrency(n)` enables worker threads.

```cpp
```

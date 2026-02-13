#include "proptest/stateful/stateful_function.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/gen.hpp"

using namespace proptest;
using namespace proptest::stateful;

struct VectorModel2
{
    VectorModel2(int size) : size(size) {}
    int size;
};

TEST(stateful_function, basic)
{
    using T = vector<int>;

    auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int value) {
        return SimpleAction<T>([value](T& obj) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
        });
    });

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto popBackGen = gen::just(SimpleAction<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto popBackGen2 = gen::just(SimpleAction<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    // actionGen<T> is shorthand for just<SimpleAction<T>>
    auto clearGen = gen::just(SimpleAction<T>([](T& obj) {
        // cout << "Clear" << endl;
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    }));

    auto actionGen =
        gen::oneOf<SimpleAction<T>>(pushBackGen, popBackGen, popBackGen2, gen::weightedGen(clearGen, 0.1));
    auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
    prop.setOnStartup([]() { cout << "startup" << endl; });
    prop.setOnCleanup([]() { cout << "cleanup" << endl; });
    prop.setSeed(0).setNumRuns(100).go();
}

TEST(stateful_function, basic_model)
{
    using T = vector<int>;
    using Model = VectorModel2;

    auto pushBackGen = gen::int32().map<Action<T, Model>>([](int value) {
        return Action<T, Model>(PROP_ACTION_NAME("PushBack", value), [value](T& obj, Model&) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
        });
    });

    auto popBackGen = gen::just(Action<T, Model>("PopBack", [](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto popBackGen2 = gen::just(Action<T, Model>("PopBack2", [](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto clearGen = gen::just(Action<T, Model>("Clear", [](T& obj, Model&) {
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    }));

    auto actionGen = gen::oneOf<Action<T, Model>>(pushBackGen, popBackGen, popBackGen2, clearGen);
    auto prop = statefulProperty<T, Model>(
        Arbi<T>(), [](T& obj) -> Model { return VectorModel2(obj.size()); }, actionGen);
    prop.setOnStartup([]() { cout << "startup" << endl; });
    prop.setOnCleanup([]() {
        cout << "cleanup" << endl;
        // PROP_ASSERT(false);
    });
    prop.setPostCheck([](T&, Model&) { cout << "postCheck" << endl; });
    auto startTime = steady_clock::now();
    prop.setSeed(0).setNumRuns(1000000).setMaxDurationMs(2000).go();
    auto endTime = steady_clock::now();
    EXPECT_GE(duration_cast<util::milliseconds>(endTime - startTime).count(), 2000);
}

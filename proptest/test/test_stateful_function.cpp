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

    auto popBackAction = SimpleAction<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    });

    auto popBackAction2 = SimpleAction<T>([](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    });

    auto clearAction = SimpleAction<T>([](T& obj) {
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    });

    auto actionGen =
        gen::oneOf<SimpleAction<T>>(pushBackGen, popBackAction, popBackAction2, gen::weightedGen<SimpleAction<T>>(gen::just(clearAction), 0.1));
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

    // Raw actions directly in oneOf (treated as gen::just(action))
    auto popBackAction = Action<T, Model>("PopBack", [](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    });

    auto popBackAction2 = Action<T, Model>("PopBack2", [](T& obj, Model&) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    });

    auto clearAction = Action<T, Model>("Clear", [](T& obj, Model&) {
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    });

    auto actionGen = gen::oneOf<Action<T, Model>>(pushBackGen, popBackAction, popBackAction2, clearAction);
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

TEST(stateful_function, onActionStart_onActionEnd_callbacks)
{
    using T = vector<int>;
    int startCount = 0, endCount = 0;

    auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int value) {
        return SimpleAction<T>([value](T& obj) { obj.push_back(value); });
    });
    auto popBackAction = SimpleAction<T>([](T& obj) {
        if (!obj.empty())
            obj.pop_back();
    });
    auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, popBackAction);

    auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
    prop.setOnActionStart([&startCount](T&, EmptyModel&) { ++startCount; });
    prop.setOnActionEnd([&endCount](T&, EmptyModel&) { ++endCount; });
    prop.setSeed(0).setNumRuns(10).go();

    EXPECT_GT(startCount, 0);
    EXPECT_EQ(startCount, endCount);
}

TEST(stateful_function, onActionEnd_invariant_check)
{
    using T = vector<int>;

    auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int value) {
        return SimpleAction<T>([value](T& obj) { obj.push_back(value); });
    });
    auto popBackAction = SimpleAction<T>([](T& obj) {
        if (!obj.empty())
            obj.pop_back();
    });
    auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, popBackAction);

    auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
    prop.setOnActionEnd([](T& vec, EmptyModel&) {
        PROP_ASSERT(vec.size() >= 0);  // Invariant: size is non-negative
    });
    prop.setSeed(0).setNumRuns(100).go();
}

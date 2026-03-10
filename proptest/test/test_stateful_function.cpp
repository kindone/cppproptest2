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
        return SimpleAction<T>(PROP_ACTION_NAME("PushBack", value), [value](T& obj) {
            auto size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
        });
    });

    auto popBackAction = SimpleAction<T>("PopBack", [](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    });

    auto popBackAction2 = SimpleAction<T>("PopBack2", [](T& obj) {
        auto size = obj.size();
        if (obj.empty())
            return;
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    });

    auto clearAction = SimpleAction<T>("Clear", [](T& obj) {
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    });

    auto actionGen =
        gen::oneOf<SimpleAction<T>>(pushBackGen, popBackAction, popBackAction2, gen::weightedGen<SimpleAction<T>>(gen::just(clearAction), 0.1));
    auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
    int startupCount = 0, cleanupCount = 0;
    prop.setOnStartup([&startupCount]() { ++startupCount; });
    prop.setOnCleanup([&cleanupCount]() { ++cleanupCount; });
    prop.setSeed(0).setNumRuns(100).go();
    EXPECT_GT(startupCount, 0);
    EXPECT_EQ(startupCount, cleanupCount);
}

TEST(stateful_function, basic_model)
{
    using T = vector<int>;
    using Model = VectorModel2;

    int startupCount = 0, cleanupCount = 0, postCheckCount = 0;
    const uint32_t maxDurationMs = 2000;

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
    prop.setOnStartup([&startupCount]() { ++startupCount; });
    prop.setOnCleanup([&cleanupCount]() { ++cleanupCount; });
    prop.setPostCheck([&postCheckCount](T&, Model&) { ++postCheckCount; });
    auto startTime = steady_clock::now();
    prop.setSeed(0).setNumRuns(1000000).setMaxDurationMs(maxDurationMs).go();
    auto endTime = steady_clock::now();

    EXPECT_GE(duration_cast<util::milliseconds>(endTime - startTime).count(), maxDurationMs);
    EXPECT_GT(startupCount, 0);
    EXPECT_EQ(startupCount, cleanupCount);
    EXPECT_EQ(startupCount, postCheckCount);
}

TEST(stateful_function, onActionStart_onActionEnd_callbacks)
{
    using T = vector<int>;
    int startCount = 0, endCount = 0;

    auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int value) {
        return SimpleAction<T>(PROP_ACTION_NAME("PushBack", value), [value](T& obj) { obj.push_back(value); });
    });
    auto popBackAction = SimpleAction<T>("PopBack", [](T& obj) {
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
        return SimpleAction<T>(PROP_ACTION_NAME("PushBack", value), [value](T& obj) { obj.push_back(value); });
    });
    auto popBackAction = SimpleAction<T>("PopBack", [](T& obj) {
        if (!obj.empty())
            obj.pop_back();
    });
    auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, popBackAction);

    auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
    prop.setOnActionEnd([](T& vec, EmptyModel&) {
        PROP_ASSERT(vec.size() < 100000);  // Invariant: size is non-negative
    });
    prop.setSeed(0).setNumRuns(100).go();
}

TEST(stateful_function, action_list_size_configuration)
{
    auto incAction = gen::just(SimpleAction<int>("Inc", [](int& v) { ++v; }));
    auto prop = statefulProperty<int>(gen::just(0), incAction);

    bool ok = prop.setSeed(0)
                  .setNumRuns(20)
                  .setActionListSize(3)
                  .setPostCheck([](int& v) { PROP_ASSERT_EQ(v, 3); })
                  .go();

    EXPECT_TRUE(ok);
}

TEST(stateful_function, shrink_output_uses_labeled_stateful_args)
{
    auto noopAction = gen::just(SimpleAction<int>("Noop", [](int&) {}));
    auto prop = statefulProperty<int>(gen::just(0), noopAction);

    optional<ReproductionStats> stats = nullopt;
    bool ok = prop.setSeed(0)
                  .setNumRuns(1)
                  .setActionListSize(0)
                  .setShrinkMaxRetries(1)
                  .setOnReproductionStats([&stats](ReproductionStats s) { stats = s; })
                  .setPostCheck([](int&) { PROP_ASSERT(false); })
                  .go();

    EXPECT_FALSE(ok);
    ASSERT_TRUE(stats.has_value());
    EXPECT_NE(stats->argsAsString.find("actions:"), string::npos);
    EXPECT_NE(stats->argsAsString.find("initial:"), string::npos);
}

/*
 * Contract:
 * - With maxConcurrency <= 1, statefulProperty executes the generated front
 *   action list and then runs postCheck without launching rear workers.
 * - With maxConcurrency > 1, statefulProperty executes the front action list
 *   and then launches the configured number of rear workers.
 * - State-dependent rear action generation starts from the post-front state
 *   snapshot, not the initial state.
 * - Action start/end callbacks are balanced for successful concurrent runs.
 *
 * Scope:
 * This file covers successful concurrency execution semantics. It does not
 * cover failure shrinking, retry assessment, exact report text, or scheduler
 * fairness.
 *
 * Helpers:
 * - addLocked() mutates a shared integer under a mutex because rear actions run
 *   on separate threads.
 * - Fixed action-list sizes make expected final values independent of random
 *   sequence length.
 * - Atomic callback counters avoid introducing test-side data races.
 */

#include "proptest/statefultest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

using namespace proptest;
using namespace proptest::stateful;
using namespace proptest::testprofile;

namespace {

constexpr int kFrontValue = 1;
constexpr int kRearValue = 10;
constexpr int kTwoRearWorkers = 2;

void addLocked(int& value, int delta)
{
    static mutex mutex;
    lock_guard<proptest::stateful::mutex> guard(mutex);
    value += delta;
}

}  // namespace

TEST(ConcurrencyExecutionContracts, serial_concurrency_executes_only_front_actions)
{
    auto increment = gen::just(SimpleAction<int>("Increment", [](int& value) {
        ++value;
    }));

    const bool ok = statefulProperty<int>(gen::just(0), increment)
        .setSeed(kStatefulReplaySeed)
        .setNumRuns(kStatefulShortRuns)
        .setMaxConcurrency(1)
        .setActionListSize(4)
        .setPostCheck([](int& value) {
            PROP_ASSERT_EQ(value, 4);
        })
        .go();

    EXPECT_TRUE(ok);
}

TEST(ConcurrencyExecutionContracts, concurrent_run_executes_front_and_configured_rear_workers)
{
    auto addOne = gen::just(SimpleAction<int>("AddOne", [](int& value) {
        addLocked(value, 1);
    }));

    const bool ok = statefulProperty<int>(gen::just(0), addOne)
        .setSeed(kStatefulReplaySeed)
        .setNumRuns(kStatefulShortRuns)
        .setMaxConcurrency(kTwoRearWorkers)
        .setActionListSize(1)
        .setPostCheck([](int& value) {
            PROP_ASSERT_EQ(value, 1 + kTwoRearWorkers);
        })
        .go();

    EXPECT_TRUE(ok);
}

TEST(ConcurrencyExecutionContracts, rear_action_factory_uses_post_front_state_snapshot)
{
    auto factory = [](int& value, EmptyModel&) -> ActionGen<int, EmptyModel> {
        if (value == 0) {
            return gen::just(Action<int, EmptyModel>("SetFrontValue", [](int& target, EmptyModel&) {
                addLocked(target, kFrontValue);
            }));
        }

        return gen::just(Action<int, EmptyModel>("AddRearValue", [](int& target, EmptyModel&) {
            addLocked(target, kRearValue);
        }));
    };

    const bool ok = statefulProperty<int>(gen::just(0), factory)
        .setSeed(kStatefulReplaySeed)
        .setNumRuns(kStatefulShortRuns)
        .setMaxConcurrency(kTwoRearWorkers)
        .setActionListSize(1)
        .setPostCheck([](int& value) {
            PROP_ASSERT_EQ(value, kFrontValue + kTwoRearWorkers * kRearValue);
        })
        .go();

    EXPECT_TRUE(ok);
}

TEST(ConcurrencyExecutionContracts, model_based_serial_concurrency_keeps_object_and_model_in_sync)
{
    using Object = vector<int>;
    struct Model {
        int size = 0;
    };

    auto modelFactory = +[](const Object& object) {
        return Model{static_cast<int>(object.size())};
    };

    auto factory = [](Object&, Model& model) -> ActionGen<Object, Model> {
        Action<Object, Model> push("Push", [](Object& object, Model& model) {
            object.push_back(1);
            model.size++;
        });
        Action<Object, Model> pop("Pop", [](Object& object, Model& model) {
            if (!object.empty()) {
                object.pop_back();
                model.size--;
            }
        });

        if (model.size == 0) {
            return gen::just<Action<Object, Model>>(push);
        }
        return gen::oneOf<Action<Object, Model>>(push, pop);
    };

    const bool ok = statefulProperty<Object, Model>(gen::just(Object{}), modelFactory, factory)
        .setSeed(kStatefulReplaySeed)
        .setNumRuns(kStatefulFocusedRuns)
        .setMaxConcurrency(1)
        .setActionListSize(2)
        .setPostCheck([](Object& object, Model& model) {
            PROP_ASSERT_EQ(static_cast<int>(object.size()), model.size);
        })
        .go();

    EXPECT_TRUE(ok);
}

TEST(ConcurrencyExecutionContracts, action_callbacks_are_balanced_for_successful_concurrent_runs)
{
    atomic<int> starts{0};
    atomic<int> ends{0};
    auto addOne = gen::just(SimpleAction<int>("AddOne", [](int& value) {
        addLocked(value, 1);
    }));

    const bool ok = statefulProperty<int>(gen::just(0), addOne)
        .setSeed(kStatefulReplaySeed)
        .setNumRuns(kTinyRuns)
        .setMaxConcurrency(kTwoRearWorkers)
        .setActionListSize(1)
        .setOnActionStart([&starts](int&, EmptyModel&) {
            starts.fetch_add(1);
        })
        .setOnActionEnd([&ends](int&, EmptyModel&) {
            ends.fetch_add(1);
        })
        .setPostCheck([](int& value) {
            PROP_ASSERT_EQ(value, 1 + kTwoRearWorkers);
        })
        .go();

    EXPECT_TRUE(ok);
    EXPECT_EQ(starts.load(), 1 + kTwoRearWorkers);
    EXPECT_EQ(ends.load(), starts.load());
}

/*
 * Contract:
 * - Stateful properties execute generated actions against both the object under
 *   test and its model, and post-checks observe the final agreed state.
 * - State-dependent action factories receive the current model state when
 *   selecting the next action generator.
 * - Lifecycle and action callbacks are balanced for successful stateful runs.
 *
 * Scope:
 * This file covers passing stateful workflows with model/object agreement,
 * state-dependent action selection, and callback counts. It does not cover
 * failing-trace shrinking, concurrent rear actions, retry assessment, exact
 * failure report text, or timeout behavior.
 *
 * Helpers:
 * - VectorMirrorModel mirrors the vector contents so actions can assert full
 *   object/model equality after every transition.
 * - SizePhaseModel tracks only vector size, making state-dependent action
 *   selection easy to observe.
 */

#include "proptest/gen.hpp"
#include "proptest/stateful/stateful_function.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

#include <algorithm>

using namespace proptest;
using namespace proptest::stateful;
using namespace proptest::testprofile;

namespace {

struct VectorMirrorModel
{
    vector<int> values;
};

struct SizePhaseModel
{
    size_t size = 0;
};

VectorMirrorModel mirrorVector(const vector<int>& values)
{
    return VectorMirrorModel{values};
}

bool objectMatchesModel(const vector<int>& object, const VectorMirrorModel& model)
{
    return object == model.values;
}

}  // namespace

TEST(StatefulModelContracts, generated_actions_keep_object_and_model_in_agreement)
{
    using Object = vector<int>;
    using Model = VectorMirrorModel;

    auto initialGen = gen::vector<int>(gen::interval<int>(-2, 2));
    initialGen.setMaxSize(kStatefulShortActions);

    auto pushBackGen = gen::interval<int>(-10, 10).map<Action<Object, Model>>([](const int& value) {
        return Action<Object, Model>(PROP_ACTION_NAME("PushBack", value), [value](Object& object, Model& model) {
            object.push_back(value);
            model.values.push_back(value);
            PROP_ASSERT(objectMatchesModel(object, model));
        });
    });

    auto popBack = Action<Object, Model>("PopBack", [](Object& object, Model& model) {
        if (!object.empty()) {
            object.pop_back();
        }
        if (!model.values.empty()) {
            model.values.pop_back();
        }
        PROP_ASSERT(objectMatchesModel(object, model));
    });

    auto clear = Action<Object, Model>("Clear", [](Object& object, Model& model) {
        object.clear();
        model.values.clear();
        PROP_ASSERT(objectMatchesModel(object, model));
    });

    auto actionGen = gen::oneOf<Action<Object, Model>>(pushBackGen, popBack, clear);
    auto prop = statefulProperty<Object, Model>(initialGen, mirrorVector, actionGen);

    const bool ok = prop.setSeed(kStatefulReplaySeed)
                        .setNumRuns(kStatefulContractRuns)
                        .setActionListMinSize(1)
                        .setActionListMaxSize(kStatefulShortActions)
                        .setPostCheck([](Object& object, Model& model) {
                            PROP_ASSERT(objectMatchesModel(object, model));
                        })
                        .go();

    EXPECT_TRUE(ok);
}

TEST(StatefulModelContracts, state_dependent_factory_uses_current_model_state)
{
    using Object = vector<int>;
    using Model = SizePhaseModel;

    vector<string> selectedActions;
    auto prop = statefulProperty<Object, Model>(
        gen::just(Object{}),
        [](const Object& object) -> Model {
            return Model{object.size()};
        },
        [&selectedActions](Object&, Model& model) -> ActionGen<Object, Model> {
            if (model.size == 0) {
                return gen::interval<int>(1, 3).map<Action<Object, Model>>(
                    [&selectedActions](const int& value) {
                        return Action<Object, Model>(
                            PROP_ACTION_NAME("PushWhenEmpty", value),
                            [value, &selectedActions](Object& object, Model& model) {
                                selectedActions.push_back("PushWhenEmpty");
                                object.push_back(value);
                                ++model.size;
                                PROP_ASSERT_EQ(object.size(), model.size);
                            });
                    });
            }

            return gen::just(Action<Object, Model>(
                "PopWhenNonEmpty",
                [&selectedActions](Object& object, Model& model) {
                    selectedActions.push_back("PopWhenNonEmpty");
                    PROP_ASSERT(!object.empty());
                    object.pop_back();
                    --model.size;
                    PROP_ASSERT_EQ(object.size(), model.size);
                }));
        });

    const bool ok = prop.setSeed(kStatefulReplaySeed)
                        .setNumRuns(kStatefulShortRuns)
                        .setActionListSize(4)
                        .setPostCheck([](Object& object, Model& model) {
                            PROP_ASSERT(object.empty());
                            PROP_ASSERT_EQ(model.size, size_t{0});
                        })
                        .go();

    EXPECT_TRUE(ok);
    EXPECT_NE(find(selectedActions.begin(), selectedActions.end(), "PushWhenEmpty"), selectedActions.end());
    EXPECT_NE(find(selectedActions.begin(), selectedActions.end(), "PopWhenNonEmpty"), selectedActions.end());
}

TEST(StatefulModelContracts, successful_runs_balance_lifecycle_and_action_callbacks)
{
    struct CounterModel
    {
        int value = 0;
    };

    int startupCount = 0;
    int cleanupCount = 0;
    int postCheckCount = 0;
    int actionStartCount = 0;
    int actionEndCount = 0;

    auto increment = gen::just(Action<int, CounterModel>("Increment", [](int& object, CounterModel& model) {
        ++object;
        ++model.value;
        PROP_ASSERT_EQ(object, model.value);
    }));

    auto prop = statefulProperty<int, CounterModel>(
        gen::just(0),
        [](const int& value) -> CounterModel {
            return CounterModel{value};
        },
        increment);

    const bool ok = prop.setSeed(kStatefulReplaySeed)
                        .setNumRuns(kStatefulFocusedRuns)
                        .setActionListSize(kStatefulShortActions)
                        .setOnStartup([&startupCount]() { ++startupCount; })
                        .setOnCleanup([&cleanupCount]() { ++cleanupCount; })
                        .setOnActionStart([&actionStartCount](int&, CounterModel&) { ++actionStartCount; })
                        .setOnActionEnd([&actionEndCount](int&, CounterModel&) { ++actionEndCount; })
                        .setPostCheck([&postCheckCount](int& object, CounterModel& model) {
                            ++postCheckCount;
                            PROP_ASSERT_EQ(object, model.value);
                        })
                        .go();

    EXPECT_TRUE(ok);
    EXPECT_EQ(startupCount, kStatefulFocusedRuns);
    EXPECT_EQ(cleanupCount, startupCount);
    EXPECT_EQ(postCheckCount, startupCount);
    EXPECT_EQ(actionStartCount, startupCount * kStatefulShortActions);
    EXPECT_EQ(actionEndCount, actionStartCount);
}

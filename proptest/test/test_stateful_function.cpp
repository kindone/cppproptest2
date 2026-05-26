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
        Arbi<T>(), [](const T& obj) -> Model { return VectorModel2(obj.size()); }, actionGen);
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

TEST(stateful_function, state_dependent_simple_action_factory)
{
    using T = vector<int>;

    auto prop = statefulProperty<T>(gen::just(T{}), [](T& obj) -> SimpleActionGen<T> {
        if (obj.empty()) {
            return gen::just(SimpleAction<T>("Push", [](T& vec) { vec.push_back(1); }));
        }
        return gen::just(SimpleAction<T>("Pop", [](T& vec) { vec.pop_back(); }));
    });
    bool ok = prop.setSeed(0)
                  .setNumRuns(20)
                  .setActionListSize(2)
                  .setPostCheck([](T& vec) { PROP_ASSERT(vec.empty()); })
                  .go();

    EXPECT_TRUE(ok);
}

TEST(stateful_function, state_dependent_model_action_factory)
{
    using T = vector<int>;
    using Model = VectorModel2;

    auto prop = statefulProperty<T, Model>(
        gen::just(T{}), [](const T& obj) -> Model { return VectorModel2(obj.size()); },
        [](T&, Model& model) -> ActionGen<T, Model> {
            if (model.size == 0) {
                return gen::just(Action<T, Model>("Push", [](T& vec, Model& mdl) {
                    vec.push_back(1);
                    ++mdl.size;
                }));
            }
            return gen::just(Action<T, Model>("Pop", [](T& vec, Model& mdl) {
                vec.pop_back();
                --mdl.size;
            }));
        });
    bool ok = prop.setSeed(0)
                  .setNumRuns(20)
                  .setActionListSize(2)
                  .setPostCheck([](T& vec, Model& mdl) {
                      PROP_ASSERT(vec.empty());
                      PROP_ASSERT_EQ(mdl.size, 0);
                  })
                  .go();

    EXPECT_TRUE(ok);
}

struct StatefulShrinkStepModel
{
    int step = 0;
};

TEST(stateful_function, state_dependent_shrink_keeps_generated_prefix)
{
    optional<ReproductionStats> stats = nullopt;
    stringstream out;
    stringstream err;

    auto prop = statefulProperty<int, StatefulShrinkStepModel>(
        gen::just(0),
        [](const int&) -> StatefulShrinkStepModel { return StatefulShrinkStepModel{}; },
        [](int&, StatefulShrinkStepModel& model) -> ActionGen<int, StatefulShrinkStepModel> {
            if (model.step == 0) {
                return gen::just(Action<int, StatefulShrinkStepModel>(
                    "Setup", [](int& obj, StatefulShrinkStepModel& mdl) {
                        obj = 1;
                        ++mdl.step;
                    }));
            }
            return gen::just(Action<int, StatefulShrinkStepModel>(
                "FailAfterSetup", [](int& obj, StatefulShrinkStepModel& mdl) {
                    obj = 2;
                    ++mdl.step;
                }));
        });

    bool ok = prop.setSeed(0)
                  .setNumRuns(1)
                  .setActionListSize(2)
                  .setShrinkMaxRetries(1)
                  .setOutputStreams(out, err)
                  .setOnReproductionStats([&stats](ReproductionStats s) { stats = s; })
                  .setPostCheck([](int& obj, StatefulShrinkStepModel&) {
                      PROP_ASSERT(obj != 2);
                  })
                  .go();

    EXPECT_FALSE(ok);
    ASSERT_TRUE(stats.has_value());
    EXPECT_NE(stats->argsAsString.find("Setup"), string::npos);
    EXPECT_NE(stats->argsAsString.find("FailAfterSetup"), string::npos);
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

/**
 * Verifies Phase 3 (last-action parameter) shrinking for state-dependent stateful tests.
 *
 * Actions are generated with gen::interval so they carry real shrink trees.
 * The property fails when the accumulated value meets a threshold.  Phase 3
 * should walk the last action's shrink tree and find the minimal value that
 * still triggers the failure (the threshold itself).
 */
TEST(stateful_function, shrink_phase3_last_action_parameters)
{
    constexpr int THRESHOLD = 10;

    // Each action adds n to the int; fails inside the action when total >= THRESHOLD.
    auto incrGen = gen::interval(THRESHOLD, 100).map<SimpleAction<int>>([](const int& n) {
        return SimpleAction<int>(PROP_ACTION_NAME("Incr", n), [n](int& obj) {
            obj += n;
            PROP_ASSERT(obj < THRESHOLD);
        });
    });

    auto prop = statefulProperty<int>(gen::just(0), incrGen);
    std::ostringstream out;
    bool ok = prop.setSeed(1)
                  .setNumRuns(20)
                  .setActionListMinSize(1)
                  .setActionListMaxSize(3)
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok) << "Property should fail: every action adds >= THRESHOLD";
    // Phase 1 should reduce to a single-action sequence.
    // Phase 3 should shrink that action's value down to THRESHOLD.
    const auto& output = out.str();
    EXPECT_NE(output.find("Incr(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 3 should shrink last action value down to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
}

// ── Phase 2b (bookmark-based, non-last element shrinking) ────────────────────
//
// Feature:     Phase 2b in applyStatefulShrinkTree explores shrink candidates for
//              every non-last position by: (1) replaying the prefix to reconstruct
//              the live state, (2) regenerating the action from its stored bookmark
//              using that correct state, (3) yielding shrinks of the fresh tree.
//
// Spec:        ∀ failing state-dependent factory sequences where a non-last element
//              has a shrinkable parameter, Phase 2b walks that element's fresh
//              state-aware shrink tree and converges to the minimal parameter value.
//
// Complement:  Phase 3 covers the LAST element; Phase 2b covers all others.
//              The single-element guard (vec.size()<=1) prevents Phase 2b from
//              firing when there is nothing to the right.

/**
 * Phase 2b: 2-action state-dependent sequence, non-last element shrinks.
 *
 * Factory: obj==0 → Latch(n) with n ∈ [THRESHOLD, 2·THRESHOLD], sets obj=n
 *          obj>0  → Check: PROP_ASSERT(obj < THRESHOLD)
 *
 * setActionListSize(2) forces exactly [Latch(n), Check].  Phase 1 cannot reduce
 * (minSize=2).  Phase 2b fires at position 0 (Latch, non-last) and walks its
 * shrink tree; Phase 3 fires at position 1 (Check, last) — Check has no shrinks.
 *
 * Expected shrunken output: Latch(THRESHOLD), Check.
 */
TEST(stateful_function, shrink_phase2b_non_last_element_parameter)
{
    constexpr int THRESHOLD = 10;

    // Factory: obj==0 → Latch(n) with n ∈ [THRESHOLD, 2·THRESHOLD], sets obj=n.
    //          obj>0  → Observe: state-dependent no-op marker (no assert — failure
    //                   detected via postCheck so it does not throw during generation).
    auto prop = statefulProperty<int>(
        gen::just(0),
        [](int& obj) -> SimpleActionGen<int> {
            if (obj == 0) {
                return gen::interval(THRESHOLD, 2 * THRESHOLD)
                    .map<SimpleAction<int>>([](const int& n) {
                        return SimpleAction<int>(PROP_ACTION_NAME("Latch", n),
                            [n](int& v) { v = n; });
                    });
            }
            return gen::just(SimpleAction<int>("Observe", [](int&) {}));
        });

    std::ostringstream out;
    bool ok = prop.setSeed(1)
                  .setNumRuns(5)
                  .setActionListSize(2)
                  .setPostCheck([](int& v) { PROP_ASSERT(v < THRESHOLD); })
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok)
        << "Always fails: Latch sets v=n>=THRESHOLD, postCheck asserts v<THRESHOLD";
    const auto& output = out.str();
    EXPECT_NE(output.find("Latch(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 2b should shrink Latch's n to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
    EXPECT_NE(output.find("Observe"), string::npos)
        << "Shrunken sequence must still contain Observe\nactual output:\n" << output;
}

struct Phase2bStateMachineModel { int phase = 0; };

/**
 * Phase 2b: 3-action state machine — Phase 2b fires at two non-last positions.
 *
 * Factory (model-based):
 *   phase=0 → Init(n) with n ∈ [THRESHOLD, 3·THRESHOLD], sets obj=n, phase→1
 *   phase=1 → Mutate: transitions phase→2 (obj unchanged)
 *   phase=2 → Verify: no-op marker (failure detected via postCheck)
 *
 * setActionListSize(3) forces [Init(n), Mutate, Verify].  The 3-action chain is
 * necessary: removing any one action stops the failure (no Verify without Init+Mutate).
 * Phase 1 cannot reduce below 3.
 *
 * Phase 2b fires at position 0 (Init, non-last): replays empty prefix → state
 * phase=0, correctly regenerates Init and walks its shrink tree.
 * Phase 2b fires at position 1 (Mutate, non-last): replays [Init(n)] → phase=1,
 * regenerates Mutate — Mutate carries no shrink tree, so no candidates.
 * Phase 3 fires at position 2 (Verify, last): Verify has no shrinks.
 *
 * This verifies state-awareness: if Phase 2b used the wrong state at position 0
 * (e.g. phase=2), it would regenerate Verify instead of Init and find no shrinks,
 * failing to minimise n.
 *
 * Note: PROP_ASSERT is NOT placed inside action bodies — doing so would throw
 * during generation-time state advancement in genActionShrinkables, escaping the
 * property framework.  Failure is detected via setPostCheck instead.
 *
 * Expected shrunken output: Init(THRESHOLD), Mutate, Verify.
 */
TEST(stateful_function, shrink_phase2b_three_action_state_machine)
{
    constexpr int THRESHOLD = 10;

    auto prop = statefulProperty<int, Phase2bStateMachineModel>(
        gen::just(0),
        [](const int&) -> Phase2bStateMachineModel { return Phase2bStateMachineModel{}; },
        [](int&, Phase2bStateMachineModel& m) -> ActionGen<int, Phase2bStateMachineModel> {
            if (m.phase == 0) {
                return gen::interval(THRESHOLD, 3 * THRESHOLD)
                    .map<Action<int, Phase2bStateMachineModel>>([](const int& n) {
                        return Action<int, Phase2bStateMachineModel>(
                            PROP_ACTION_NAME("Init", n),
                            [n](int& v, Phase2bStateMachineModel& mdl) {
                                v = n;
                                mdl.phase = 1;
                            });
                    });
            }
            if (m.phase == 1) {
                return gen::just(Action<int, Phase2bStateMachineModel>(
                    "Mutate",
                    [](int&, Phase2bStateMachineModel& mdl) { mdl.phase = 2; }));
            }
            // phase == 2: no-op marker; failure is detected via setPostCheck below
            return gen::just(Action<int, Phase2bStateMachineModel>(
                "Verify",
                [](int&, Phase2bStateMachineModel&) {}));
        });

    std::ostringstream out;
    bool ok = prop.setSeed(1)
                  .setNumRuns(5)
                  .setActionListSize(3)
                  .setPostCheck([](int& v, Phase2bStateMachineModel&) { PROP_ASSERT(v < THRESHOLD); })
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok)
        << "Always fails: Init sets v=n>=THRESHOLD, postCheck asserts v<THRESHOLD";
    const auto& output = out.str();
    // Phase 2b at position 0 uses state phase=0, correctly regenerates Init,
    // and walks its shrink tree down to n = THRESHOLD.
    EXPECT_NE(output.find("Init(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 2b should shrink Init's n to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
    EXPECT_NE(output.find("Mutate"), string::npos)
        << "Shrunken sequence must still contain Mutate\nactual output:\n" << output;
    EXPECT_NE(output.find("Verify"), string::npos)
        << "Shrunken sequence must still contain Verify\nactual output:\n" << output;
}

/**
 * Phase 2b: single-element guard — vec.size()<=1 returns empty stream.
 *
 * Spec: When the action list contains exactly one element (the last), Phase 2b's
 * guard (vec.size() <= 1) fires and produces no candidates.  Shrinking delegates
 * entirely to Phase 3, which walks the sole element's own shrink tree.
 *
 * Observable: the shrunken output still reaches the minimal parameter value,
 * confirming Phase 3 compensates correctly and no crash occurs from Phase 2b.
 */
TEST(stateful_function, shrink_phase2b_single_element_guard)
{
    constexpr int THRESHOLD = 10;

    auto incrGen = gen::interval(THRESHOLD, 100).map<SimpleAction<int>>([](const int& n) {
        return SimpleAction<int>(PROP_ACTION_NAME("Add", n),
            [n](int& obj) { obj += n; PROP_ASSERT(obj < THRESHOLD); });
    });

    auto prop = statefulProperty<int>(gen::just(0), incrGen);
    std::ostringstream out;
    bool ok = prop.setSeed(1)
                  .setNumRuns(20)
                  .setActionListSize(1)
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok) << "Always fails: Add(n>=THRESHOLD) makes obj >= THRESHOLD";
    // Phase 2b guard: vec.size()<=1 → empty stream — no Phase 2b candidates.
    // Phase 3 takes over and shrinks the sole element's n down to THRESHOLD.
    EXPECT_NE(out.str().find("Add(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 3 should shrink Add's n to THRESHOLD=" << THRESHOLD
        << " (Phase 2b guard correct for single-element sequences)"
        << "\nactual output:\n" << out.str();
}

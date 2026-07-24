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
 * Verifies LastActionParams shrinking for state-dependent stateful tests.
 *
 * Actions are generated with gen::interval so they carry real shrink trees.
 * The property fails when the accumulated value meets a threshold.  LastActionParams
 * should walk the last action's shrink tree and find the minimal value that
 * still triggers the failure (the threshold itself).
 */
TEST(stateful_function, shrink_last_action_params)
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
    // SequencePruning should reduce to a single-action sequence.
    // LastActionParams should shrink that action's value down to THRESHOLD.
    const auto& output = out.str();
    EXPECT_NE(output.find("Incr(" + to_string(THRESHOLD) + ")"), string::npos)
        << "LastActionParams should shrink last action value down to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
}

// ── PrefixParams (bookmark-based, non-last element shrinking) ────────────────
//
// Feature:     PrefixParams in applyStatefulShrinkTree explores shrink candidates for
//              every non-last position by: (1) replaying the prefix to reconstruct
//              the live state, (2) regenerating the action from its stored bookmark
//              using that correct state, (3) yielding shrinks of the fresh tree.
//
// Spec:        ∀ failing state-dependent factory sequences where a non-last element
//              has a shrinkable parameter, PrefixParams walks that element's fresh
//              state-aware shrink tree and converges to the minimal parameter value.
//
// Complement:  LastActionParams covers the LAST element; PrefixParams covers all others.
//              The single-element guard (vec.size()<=1) prevents PrefixParams from
//              firing when there is nothing to the right.

/**
 * PrefixParams: 2-action state-dependent sequence, non-last element shrinks.
 *
 * Factory: obj==0 → Latch(n) with n ∈ [THRESHOLD, 2·THRESHOLD], sets obj=n
 *          obj>0  → Check: PROP_ASSERT(obj < THRESHOLD)
 *
 * setActionListSize(2) forces exactly [Latch(n), Check].  SequencePruning cannot reduce
 * (minSize=2).  PrefixParams fires at position 0 (Latch, non-last) and walks its
 * shrink tree; LastActionParams fires at position 1 (Check, last) — Check has no shrinks.
 *
 * Expected shrunken output: Latch(THRESHOLD), Check.
 */
TEST(stateful_function, shrink_prefix_params_non_last)
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
        << "PrefixParams should shrink Latch's n to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
    EXPECT_NE(output.find("Observe"), string::npos)
        << "Shrunken sequence must still contain Observe\nactual output:\n" << output;
}

struct ThreeStateModel { int phase = 0; };

/**
 * PrefixParams: 3-action state machine — PrefixParams fires at two non-last positions.
 *
 * Factory (model-based):
 *   phase=0 → Init(n) with n ∈ [THRESHOLD, 3·THRESHOLD], sets obj=n, phase→1
 *   phase=1 → Mutate: transitions phase→2 (obj unchanged)
 *   phase=2 → Verify: no-op marker (failure detected via postCheck)
 *
 * setActionListSize(3) forces [Init(n), Mutate, Verify].  The 3-action chain is
 * necessary: removing any one action stops the failure (no Verify without Init+Mutate).
 * SequencePruning cannot reduce below 3.
 *
 * PrefixParams fires at position 0 (Init, non-last): replays empty prefix → model
 * phase=0, correctly regenerates Init and walks its shrink tree.
 * PrefixParams fires at position 1 (Mutate, non-last): replays [Init(n)] → phase=1,
 * regenerates Mutate — Mutate carries no shrink tree, so no candidates.
 * LastActionParams fires at position 2 (Verify, last): Verify has no shrinks.
 *
 * This verifies state-awareness: if PrefixParams used the wrong model state at
 * position 0 (e.g. phase=2), it would regenerate Verify instead of Init and find
 * no shrinks, failing to minimise n.
 *
 * Note: PROP_ASSERT is NOT placed inside action bodies — doing so would throw
 * during generation-time state advancement in genActionShrinkables, escaping the
 * property framework.  Failure is detected via setPostCheck instead.
 *
 * Expected shrunken output: Init(THRESHOLD), Mutate, Verify.
 */
TEST(stateful_function, shrink_prefix_params_state_machine)
{
    constexpr int THRESHOLD = 10;

    auto prop = statefulProperty<int, ThreeStateModel>(
        gen::just(0),
        [](const int&) -> ThreeStateModel { return ThreeStateModel{}; },
        [](int&, ThreeStateModel& m) -> ActionGen<int, ThreeStateModel> {
            if (m.phase == 0) {
                return gen::interval(THRESHOLD, 3 * THRESHOLD)
                    .map<Action<int, ThreeStateModel>>([](const int& n) {
                        return Action<int, ThreeStateModel>(
                            PROP_ACTION_NAME("Init", n),
                            [n](int& v, ThreeStateModel& mdl) {
                                v = n;
                                mdl.phase = 1;
                            });
                    });
            }
            if (m.phase == 1) {
                return gen::just(Action<int, ThreeStateModel>(
                    "Mutate",
                    [](int&, ThreeStateModel& mdl) { mdl.phase = 2; }));
            }
            // phase == 2: no-op marker; failure is detected via setPostCheck below
            return gen::just(Action<int, ThreeStateModel>(
                "Verify",
                [](int&, ThreeStateModel&) {}));
        });

    std::ostringstream out;
    bool ok = prop.setSeed(1)
                  .setNumRuns(5)
                  .setActionListSize(3)
                  .setPostCheck([](int& v, ThreeStateModel&) { PROP_ASSERT(v < THRESHOLD); })
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok)
        << "Always fails: Init sets v=n>=THRESHOLD, postCheck asserts v<THRESHOLD";
    const auto& output = out.str();
    // PrefixParams at position 0 uses model state phase=0, correctly regenerates Init,
    // and walks its shrink tree down to n = THRESHOLD.
    EXPECT_NE(output.find("Init(" + to_string(THRESHOLD) + ")"), string::npos)
        << "PrefixParams should shrink Init's n to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
    EXPECT_NE(output.find("Mutate"), string::npos)
        << "Shrunken sequence must still contain Mutate\nactual output:\n" << output;
    EXPECT_NE(output.find("Verify"), string::npos)
        << "Shrunken sequence must still contain Verify\nactual output:\n" << output;
}

/**
 * PrefixParams: single-element guard — vec.size()<=1 returns empty stream.
 *
 * Spec: When the action list contains exactly one element (the last), PrefixParams'
 * guard (vec.size() <= 1) fires and produces no candidates.  Shrinking delegates
 * entirely to LastActionParams, which walks the sole element's own shrink tree.
 *
 * Observable: the shrunken output still reaches the minimal parameter value,
 * confirming LastActionParams compensates correctly and no crash occurs from PrefixParams.
 */
TEST(stateful_function, shrink_prefix_params_single_guard)
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
    // PrefixParams guard: vec.size()<=1 → empty stream — no PrefixParams candidates.
    // LastActionParams takes over and shrinks the sole element's n down to THRESHOLD.
    EXPECT_NE(out.str().find("Add(" + to_string(THRESHOLD) + ")"), string::npos)
        << "LastActionParams should shrink Add's n to THRESHOLD=" << THRESHOLD
        << " (PrefixParams guard correct for single-element sequences)"
        << "\nactual output:\n" << out.str();
}

namespace {

optional<int> firstLatchParam(const string& text)
{
    const auto pos = text.find("Latch(");
    if (pos == string::npos)
        return nullopt;
    const auto start = pos + 6;
    const auto end = text.find(')', start);
    if (end == string::npos)
        return nullopt;
    return stoi(text.substr(start, end - start));
}

string lineContaining(const string& output, const string& marker)
{
    const auto pos = output.find(marker);
    if (pos == string::npos)
        return "";
    const auto end = output.find('\n', pos);
    return output.substr(pos, end == string::npos ? string::npos : end - pos);
}

} // namespace

/**
 * PrefixParams effectiveness: demonstrates that PrefixParams shrinks the non-last
 * Latch parameter, not only the last action's parameters (LastActionParams).
 *
 * Factory (state-dependent):
 *   obj == 0 → Latch(n), n ∈ [10, 200]: sets obj = n
 *   obj  > 0 → Scale(m), m ∈ [1, 10]:  multiplies obj by m
 *
 * Failure (postCheck): obj > FAIL_THRESHOLD (= 500).
 * setActionListSize(3) forces a 3-action chain so SequencePruning cannot drop actions.
 *
 * Observable: the shrunken counterexample has a strictly smaller Latch(n) than the
 * original failing case, proving PrefixParams walked the non-last element's shrink tree.
 */
TEST(stateful_function, prefix_params_effectiveness)
{
    constexpr int FAIL_THRESHOLD = 500;

    auto prop = statefulProperty<int>(
        gen::just(0),
        [](int& obj) -> SimpleActionGen<int> {
            if (obj == 0) {
                return gen::interval(10, 200)
                    .map<SimpleAction<int>>([](const int& n) {
                        return SimpleAction<int>(PROP_ACTION_NAME("Latch", n),
                            [n](int& v) { v = n; });
                    });
            }
            return gen::interval(1, 10)
                .map<SimpleAction<int>>([](const int& m) {
                    return SimpleAction<int>(PROP_ACTION_NAME("Scale", m),
                        [m](int& v) { v *= m; });
                });
        });

    std::ostringstream out;
    bool ok = prop.setNumRuns(50)
                  .setActionListSize(3)
                  .setPostCheck([](int& v) { PROP_ASSERT(v <= FAIL_THRESHOLD); })
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok) << "Should find a failing case within 50 runs (product > " << FAIL_THRESHOLD << ")";
    const auto& output = out.str();

    const string initialLine = lineContaining(output, "with args:");
    const string simplestLine = lineContaining(output, "simplest args found by shrinking");
    ASSERT_FALSE(initialLine.empty()) << "Missing initial failing args\noutput:\n" << output;
    ASSERT_FALSE(simplestLine.empty()) << "Missing shrunken args\noutput:\n" << output;

    const auto initialLatch = firstLatchParam(initialLine);
    const auto simplestLatch = firstLatchParam(simplestLine);
    ASSERT_TRUE(initialLatch.has_value()) << "Initial case should contain Latch(n)\n" << initialLine;
    ASSERT_TRUE(simplestLatch.has_value()) << "Shrunken case should contain Latch(n)\n" << simplestLine;

    EXPECT_LT(*simplestLatch, *initialLatch)
        << "PrefixParams should shrink non-last Latch from " << *initialLatch << " to " << *simplestLatch
        << "\ninitial:\n" << initialLine << "\nsimplest:\n" << simplestLine;
    EXPECT_NE(simplestLine.find("Scale("), string::npos)
        << "Shrunken sequence should still contain Scale actions\n" << simplestLine;
}

// ── Non-copyable ObjectType: shared_ptr workaround and its shrinker limitation ──
//
// Problem: statefulProperty requires ObjectType to be copy-constructible at three
// sites (invoke() pre-simulation, runCandidate(), applyStatefulShrinkTree Phase 2b).
// For identity-typed objects (mutex-owning, resource-owning) that are semantically
// non-copyable, the only current workaround is shared_ptr<T> as the ObjectType.
//
// The workaround compiles but silently breaks shrinker isolation: a shared_ptr
// "copy" is a refcount bump aliasing the same underlying object.  Shrink candidates
// are validated against already-mutated state instead of a fresh initial state,
// so the shrinker reports spurious minimal counterexamples.
//
// Fix tracked as: HDBPROPTEST-11 (factory-based initial-state regeneration).
// Idea: store Function<ObjectType()> closing over (initialGen + Random snapshot)
// instead of ObjectType value.  Every replay calls the factory for a truly fresh
// instance.  Requires compile-time constraint shift: copy-constructible → move-
// constructible-or-factory-returnable.

namespace {

// A type that explicitly deletes its copy constructor — stands in for a
// mutex-owning or resource-owning "identity" object.
struct NonCopyableCounter {
    explicit NonCopyableCounter(int v = 0) : value(v) {}
    NonCopyableCounter(const NonCopyableCounter&) = delete;
    NonCopyableCounter& operator=(const NonCopyableCounter&) = delete;
    NonCopyableCounter(NonCopyableCounter&&) = default;
    int value;
};

// Scenario constants used by both tests below.
// n ∈ [1, 3], THRESHOLD = 5.
// Only valid 2-action failing CE from a clean counter: [Add(3), Add(3)] (sum=6>5).
// Any 2-action sequence with max(n_i) ≤ 2 gives sum ≤ 2+3=5 ≤ THRESHOLD — a PASS.
constexpr int NON_COPYABLE_N_MAX = 3;
constexpr int NON_COPYABLE_THRESHOLD = 5;

using CounterPtr = shared_ptr<NonCopyableCounter>;

auto makeNonCopyableAddGen()
{
    return gen::interval(1, NON_COPYABLE_N_MAX).map<SimpleAction<CounterPtr>>([](const int& n) {
        return SimpleAction<CounterPtr>(PROP_ACTION_NAME("Add", n),
            [n](CounterPtr& ptr) { ptr->value += n; });
    });
}

} // anonymous namespace

/**
 * Non-copyable workaround: shared_ptr<T> compiles and finds failures at runtime.
 *
 * This test verifies that using shared_ptr<NonCopyableCounter> as ObjectType is
 * a valid workaround for the copy-constructibility requirement — the property runs
 * to completion and detects actual failures.  Shrinker output quality is a
 * separate concern (see the DISABLED test below).
 */
TEST(stateful_function, non_copyable_shared_ptr_workaround_finds_failure)
{
    // Workaround: wrap the non-copyable type in shared_ptr so that the three
    // copy sites (invoke pre-sim, runCandidate, shrink_pipeline Phase 2b) only
    // copy the pointer (refcount bump), not the underlying NonCopyableCounter.
    auto addGen = makeNonCopyableAddGen();
    auto prop = statefulProperty<CounterPtr>(
        gen::lazy<CounterPtr>([] { return std::make_shared<NonCopyableCounter>(0); }),
        addGen);

    std::ostringstream out;
    bool ok = prop.setSeed(0)
                  .setNumRuns(200)
                  .setActionListMinSize(2)
                  .setActionListMaxSize(4)
                  .setPostCheck([](CounterPtr& ptr) {
                      PROP_ASSERT(ptr->value <= NON_COPYABLE_THRESHOLD);
                  })
                  .setOutputStreams(out, out)
                  .go();

    // The workaround compiles and the framework correctly detects the failure.
    EXPECT_FALSE(ok) << "Should find a failing action sequence (sum of Add(n) > THRESHOLD)";
    EXPECT_NE(out.str().find("simplest args found by shrinking"), string::npos)
        << "Shrinker should run and emit output";
}

/**
 * HDBPROPTEST-11 fixed: factory-based regeneration produces correct minimal CE.
 *
 * The shrinker stores a Function<ObjectType()> factory in args[0] (capturing
 * initialGen + a Random snapshot) instead of the ObjectType value.  Every
 * candidate replay calls the factory to get a brand-new initial instance,
 * eliminating the shared_ptr aliasing that caused state to accumulate.
 *
 * True minimal CE: [Add(3), Add(3)] — the only 2-action sequence with
 * n_i ∈ [1,3] whose sum (6) exceeds THRESHOLD (5) from a clean counter.
 * Add(1) and Add(2) cannot appear because 1+3=4 and 2+3=5 both ≤ THRESHOLD.
 */
TEST(stateful_function, non_copyable_correct_minimal_ce_after_hdbproptest11)
{
    auto addGen2 = makeNonCopyableAddGen();
    auto prop = statefulProperty<CounterPtr>(
        gen::lazy<CounterPtr>([] { return std::make_shared<NonCopyableCounter>(0); }),
        addGen2);

    std::ostringstream out;
    bool ok = prop.setSeed(0)
                  .setNumRuns(200)
                  .setActionListMinSize(2)
                  .setActionListMaxSize(4)
                  .setShrinkMaxRetries(1)
                  .setPostCheck([](CounterPtr& ptr) {
                      PROP_ASSERT(ptr->value <= NON_COPYABLE_THRESHOLD);
                  })
                  .setOutputStreams(out, out)
                  .go();

    EXPECT_FALSE(ok);
    const auto& output = out.str();

    const auto simplestPos = output.find("simplest args found by shrinking");
    ASSERT_NE(simplestPos, string::npos) << "Shrinker must have fired\n" << output;
    const auto simplestEnd = output.find('\n', simplestPos);
    const string simplestLine = output.substr(simplestPos, simplestEnd - simplestPos);

    // Factory-based regeneration: shrinker resets to value=0 on every replay.
    // Only Add(3)+Add(3)=6>5 is a valid 2-action failing CE; smaller values cannot fail.
    EXPECT_NE(simplestLine.find("Add(3)"), string::npos)
        << "True minimal CE must contain Add(3)\nsimplest: " << simplestLine;
    EXPECT_EQ(simplestLine.find("Add(1)"), string::npos)
        << "Add(1) must NOT appear: 1+3=4<=THRESHOLD=5 passes from a clean counter\nsimplest: " << simplestLine;
    EXPECT_EQ(simplestLine.find("Add(2)"), string::npos)
        << "Add(2) must NOT appear: 2+3=5<=THRESHOLD=5 passes from a clean counter\nsimplest: " << simplestLine;
}

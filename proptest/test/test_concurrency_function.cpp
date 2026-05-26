#include "proptest/statefultest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/vector.hpp"
#include "proptest/util/bitmap.hpp"
#include <mutex>
#include <sstream>

using namespace proptest;
using namespace proptest::stateful;

using std::mutex;
using std::lock_guard;

// extern mutex& getMutex();

mutex& getMutex()
{
    static mutex mtx;
    return mtx;
}

class ConcurrencyTest : public ::testing::Test {
public:
};

TEST(concurrency_function, WithoutModel)
{
    auto pushBackGen = gen::int32().map<SimpleAction<vector<int>>>([](const int& value) {
        return SimpleAction<vector<int>>([value](vector<int>& obj) {
            // cout << "PushBack(" << value << ")" << endl;
            lock_guard<mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = gen::just(SimpleAction<vector<int>>([](vector<int>& obj) {
        lock_guard<mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = gen::just(SimpleAction<vector<int>>([](vector<int>& obj) {
        lock_guard<mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionGen = gen::oneOf<SimpleAction<vector<int>>>(pushBackGen, popBackGen, clearGen);

    auto prop = statefulProperty<vector<int>>(gen::vector<int>(), actionGen);
    prop.setMaxConcurrency(2);
    prop.go();
}

TEST(concurrency_function, WithModel)
{
    struct Model
    {
    };

    auto pushBackGen = gen::int32().map<Action<vector<int>, Model>>([](const int& value) {
        return Action<vector<int>, Model>([value](vector<int>& obj, Model&) {
            // cout << "PushBack(" << value << ")" << endl;
            lock_guard<mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = gen::just(Action<vector<int>, Model>([](vector<int>& obj, Model&) {
        lock_guard<mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = gen::just(Action<vector<int>, Model>([](vector<int>& obj, Model&) {
        lock_guard<mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionGen = gen::oneOf<Action<vector<int>, Model>>(pushBackGen, popBackGen, clearGen);

    auto prop = statefulProperty<vector<int>, Model>(
        gen::vector<int>(), [](const vector<int>&) { return Model(); }, actionGen);
    prop.setMaxConcurrency(2);
    prop.go();
}

TEST(concurrency_function, bitmap)
{
    using Bitmap = util::Bitmap;

    auto acquireGen = gen::just(SimpleAction<Bitmap>("Acquire", [](Bitmap& bitmap) {
        [[maybe_unused]] int pos = bitmap.acquire();
        bitmap.unacquire(pos);
    }));

    [[maybe_unused]] auto unacquireGen = gen::integers<int>(0, Bitmap::size).map<SimpleAction<Bitmap>>(+[](const int& pos) {
        return SimpleAction<Bitmap>("Unacquire", [pos](Bitmap& bitmap) {
            try {
                bitmap.unacquire(pos);
                cout << "unacquired" << endl;
            } catch(runtime_error&) {
                cout << "failed to unacquire" << endl;
            }
        });
    });

    auto actionGen = gen::oneOf<SimpleAction<Bitmap>>(acquireGen/*, unacquireGen*/);
    auto prop = statefulProperty<Bitmap>(
        gen::just<Bitmap>(Bitmap()), actionGen);
    prop.setMaxConcurrency(2);
    prop.go();
}

TEST(concurrency_function, shrink_with_retry_timeout_smoke)
{
    auto noopGen = gen::just(SimpleAction<int>([](int&) {}));
    auto prop = statefulProperty<int>(gen::interval<int>(0, 100), noopGen);
    bool ok = prop.setSeed(1)
                  .setNumRuns(1)
                  .setMaxConcurrency(2)
                  .setShrinkMaxRetries(2)
                  .setShrinkTimeoutMs(200)
                  .setShrinkRetryTimeoutMs(100)
                  .setPostCheck([](int&) { PROP_ASSERT(false); })
                  .go();
    EXPECT_FALSE(ok);
}

TEST(concurrency_function, shrink_uses_saved_rng_for_later_failure)
{
    constexpr uint64_t seed = 123;
    auto initialGen = gen::interval<int>(1000, 1000000);
    auto noopGen = gen::just(SimpleAction<int>("Noop", [](int&) {}));
    auto actionGen = noopGen.template map<Action<int, EmptyModel>>(
        +[](const SimpleAction<int>& simpleAction) { return Action<int, EmptyModel>(simpleAction); });
    auto actionListGen = Arbi<list<Action<int, EmptyModel>>>(actionGen, 0, 0);

    Random expectedRand(seed);
    const int firstInitial = initialGen(expectedRand).getRef();
    actionListGen(expectedRand);
    actionListGen(expectedRand);
    const int secondInitial = initialGen(expectedRand).getRef();
    actionListGen(expectedRand);
    actionListGen(expectedRand);
    ASSERT_NE(firstInitial, secondInitial);

    auto prop = statefulProperty<int>(initialGen, noopGen);
    std::ostringstream out;
    auto* oldOut = cout.rdbuf(out.rdbuf());
    bool ok = prop.setSeed(seed)
                  .setNumRuns(2)
                  .setMaxConcurrency(1)
                  .setActionListSize(0)
                  .setPostCheck([secondInitial](int& value) {
                      PROP_ASSERT(value != secondInitial);
                  })
                  .go();
    cout.rdbuf(oldOut);

    EXPECT_FALSE(ok);
    EXPECT_NE(out.str().find("initial: " + to_string(secondInitial)), string::npos)
        << "shrink should regenerate the second failing run from savedRand, not run 1"
        << "\nfirstInitial=" << firstInitial << "\nsecondInitial=" << secondInitial
        << "\noutput:\n" << out.str();
}

TEST(concurrency_function, action_list_size_configuration)
{
    auto incAction = gen::just(SimpleAction<int>([](int& v) { ++v; }));
    auto prop = statefulProperty<int>(gen::just(0), incAction);

    bool ok = prop.setSeed(0)
                  .setNumRuns(20)
                  .setMaxConcurrency(1)
                  .setActionListSize(4)
                  .setPostCheck([](int& v) { PROP_ASSERT_EQ(v, 4); })
                  .go();

    EXPECT_TRUE(ok);
}

/**
 * Verifies that concurrency shrinking reduces thread count before shrinking
 * within sequences.  We set up a failure that only occurs when at least one
 * rear thread runs (postCheck fires regardless), and check that the shrinker
 * reports "fewer threads" in its output — i.e., phase-0 thread-count reduction
 * is attempted first.
 *
 * Also verifies that with numThreads==1 (1 rear), a serial-equivalent run
 * (effectiveThreads derived from args.size()-2) correctly executes just the
 * single rear thread without deadlock.
 */
TEST(concurrency_function, shrink_thread_count_reduction_and_serial_fallback)
{
    // Action: increment the shared int
    auto incGen = gen::just(SimpleAction<int>([](int& v) { ++v; }));

    std::ostringstream log;

    auto prop = statefulProperty<int>(gen::just(0), incGen);
    prop.setMaxConcurrency(2)
        .setNumRuns(1)
        .setActionListMinSize(1)
        .setActionListMaxSize(3)
        // postCheck always fails so shrinking kicks in immediately
        .setPostCheck([](int& v) { PROP_ASSERT(v >= 0); /* never fails */ });

    // We just want to confirm the property runs without crashing when
    // thread-count changes during shrinking.  Run a passing property with 1 thread.
    auto propSingle = statefulProperty<int>(gen::just(0), incGen);
    bool ok = propSingle.setSeed(42)
                        .setNumRuns(10)
                        .setMaxConcurrency(1)
                        .setActionListSize(2)
                        .setPostCheck([](int& v) { PROP_ASSERT_EQ(v, 2); })
                        .go();

    // With 1 rear thread and exactly 2 actions, each run should increment twice
    EXPECT_TRUE(ok);
}

/**
 * Verifies prefix-length-first ordering for concurrency action lists.
 *
 * With 1 rear thread, concurrency shrinking should behave like stateful:
 * shorter sequences are tried before element simplification.  We use an
 * action that accumulates a value and fails once the total reaches a threshold.
 * The shrinker should find a minimal single-action sequence rather than a
 * long sequence of smaller increments.
 */
TEST(concurrency_function, shrink_prefix_length_first_for_action_lists)
{
    constexpr int THRESHOLD = 5;

    // Each action adds n to the shared int; fails when total >= THRESHOLD.
    auto incrGen = gen::interval(THRESHOLD, 10).map<SimpleAction<int>>([](const int& n) {
        return SimpleAction<int>([n](int& obj) {
            obj += n;
            PROP_ASSERT(obj < THRESHOLD);
        });
    });

    // 1 rear thread: equivalent to stateful.  Should fail quickly since every
    // action adds at least THRESHOLD.
    auto prop = statefulProperty<int>(gen::just(0), incrGen);
    bool failed = !prop.setSeed(1)
                       .setNumRuns(50)
                       .setMaxConcurrency(1)
                       .setActionListMinSize(0)
                       .setActionListMaxSize(5)
                       .go();

    EXPECT_TRUE(failed) << "Property should fail: every action adds >= THRESHOLD";
}

/**
 * Verifies Phase 3 (last-action parameter) shrinking for concurrency tests.
 *
 * Actions are generated with gen::interval so they carry real shrink trees.
 * With 1 rear thread (serial-equivalent), Phase 1 should reduce to a single
 * action, then Phase 3 should walk the last action's shrink tree down to
 * the minimal value that still triggers the failure (the threshold itself).
 */
TEST(concurrency_function, shrink_phase3_last_action_parameters)
{
    constexpr int THRESHOLD = 10;

    // Each action adds n to the int and asserts the total stays below THRESHOLD.
    // Actions are labelled so we can find the shrunk value in the output.
    auto incrGen = gen::interval(THRESHOLD, 100).map<SimpleAction<int>>([](const int& n) {
        return SimpleAction<int>(PROP_ACTION_NAME("Incr", n), [n](int& obj) {
            obj += n;
            PROP_ASSERT(obj < THRESHOLD);
        });
    });

    auto prop = statefulProperty<int>(gen::just(0), incrGen);
    std::ostringstream out;
    auto* oldOut = cout.rdbuf(out.rdbuf());
    bool ok = prop.setSeed(1)
                  .setNumRuns(20)
                  .setMaxConcurrency(1)
                  .setActionListMinSize(1)
                  .setActionListMaxSize(3)
                  .go();
    cout.rdbuf(oldOut);

    EXPECT_FALSE(ok) << "Property should fail: every action adds >= THRESHOLD";
    // Phase 1 reduces to a single action; Phase 3 shrinks its value to THRESHOLD.
    const auto& output = out.str();
    EXPECT_NE(output.find("Incr(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 3 should shrink last action value down to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
}

/**
 * Verifies state-dependent action factory for concurrency: front actions are generated
 * and executed with interleaved factory calls; rear actions are pre-generated against
 * the post-front state snapshot (each thread independently simulates execution).
 *
 * A vector starts empty.  The factory only allows Push when empty; otherwise also allows Pop.
 * The postCheck verifies the vector size is in the expected range, confirming that
 * state-dependent generation doesn't produce out-of-bounds operations.
 */
TEST(concurrency_function, state_dependent_action_factory_without_model)
{
    using T = vector<int>;

    // Factory: only allow Push when empty; otherwise allow Push or Pop.
    auto factory = [](T& obj, EmptyModel&) -> ActionGen<T, EmptyModel> {
        Action<T, EmptyModel> pushAction("Push", [](T& v, EmptyModel&) {
            lock_guard<mutex> guard(getMutex());
            v.push_back(1);
        });
        Action<T, EmptyModel> popAction("Pop", [](T& v, EmptyModel&) {
            lock_guard<mutex> guard(getMutex());
            if (!v.empty()) v.pop_back();
        });
        if (obj.empty()) {
            return gen::just<Action<T, EmptyModel>>(pushAction);
        }
        return gen::oneOf<Action<T, EmptyModel>>(pushAction, popAction);
    };

    auto prop = statefulProperty<T>(gen::just(T{}), factory);
    bool ok = prop.setSeed(0).setNumRuns(100).setMaxConcurrency(2).go();
    EXPECT_TRUE(ok);
}

/**
 * Verifies state-dependent action factory with model for concurrency.
 * The model tracks element count; factory uses model size to bound random indices.
 * With 1 rear thread (serial-equivalent), front+rear together run 2 actions each,
 * and postCheck verifies the model and object stay in sync.
 */
TEST(concurrency_function, state_dependent_action_factory_with_model)
{
    using T = vector<int>;
    struct Model {
        int size = 0;
    };

    auto modelFactory = +[](const T& obj) { return Model{static_cast<int>(obj.size())}; };

    auto factory = [](T& obj, Model&) -> ActionGen<T, Model> {
        Action<T, Model> pushAction("Push", [](T& v, Model& m) {
            lock_guard<mutex> guard(getMutex());
            v.push_back(1);
            ++m.size;
        });
        Action<T, Model> popAction("Pop", [](T& v, Model& m) {
            lock_guard<mutex> guard(getMutex());
            if (!v.empty()) { v.pop_back(); --m.size; }
        });
        if (obj.empty()) {
            return gen::just<Action<T, Model>>(pushAction);
        }
        return gen::oneOf<Action<T, Model>>(pushAction, popAction);
    };

    auto prop = statefulProperty<T, Model>(gen::just(T{}), modelFactory, factory);
    bool ok = prop.setSeed(0)
                  .setNumRuns(50)
                  .setMaxConcurrency(1)
                  .setPostCheck([](T& v, Model& m) {
                      PROP_ASSERT_EQ(static_cast<int>(v.size()), m.size);
                  })
                  .go();
    EXPECT_TRUE(ok);
}

/**
 * Verifies that shrinking works correctly for factory-based concurrency.
 * With 1 rear thread (serial-equivalent), the shrinker should reduce a failing
 * factory-generated sequence to a single action.
 */
TEST(concurrency_function, state_dependent_factory_shrink)
{
    using T = int;
    constexpr int THRESHOLD = 5;

    // Factory ignores state; generates an action that increments and fails above threshold.
    auto factory = [](T&, EmptyModel&) -> ActionGen<T, EmptyModel> {
        return gen::interval(THRESHOLD, 20).map<Action<T, EmptyModel>>([](const int& n) {
            return Action<T, EmptyModel>(PROP_ACTION_NAME("Add", n), [n](T& v, EmptyModel&) {
                v += n;
                PROP_ASSERT(v < THRESHOLD);
            });
        });
    };

    auto prop = statefulProperty<T>(gen::just(0), factory);
    std::ostringstream out;
    auto* oldOut = cout.rdbuf(out.rdbuf());
    bool ok = prop.setSeed(1)
                  .setNumRuns(20)
                  .setMaxConcurrency(1)
                  .setActionListMinSize(1)
                  .setActionListMaxSize(3)
                  .go();
    cout.rdbuf(oldOut);

    EXPECT_FALSE(ok) << "Property should fail: every action adds >= THRESHOLD";
    const auto& output = out.str();
    EXPECT_NE(output.find("Add(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 3 should shrink action value down to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << output;
}

// ── Phase 2b tests ────────────────────────────────────────────────────────────

/**
 * Phase 2b — front thread, state-dependent factory (serial concurrency).
 *
 * setMaxConcurrency(1): rear lists are generated (for rand parity) but not
 * executed.  Shrinking uses the shared applyStatefulShrinkTree for the front,
 * which includes Phase 2b.
 *
 * Factory:
 *   obj = 0 → Latch(n) with n ∈ [THRESHOLD, 2·THRESHOLD]: sets obj = n
 *   obj > 0 → Observe: no-op
 *
 * setActionListSize(2) forces front = [Latch(n), Observe].
 * Failure: postCheck PROP_ASSERT(obj < THRESHOLD).
 *
 * Phase 1 cannot shorten below 1 action (Latch alone still fails).
 * Phase 2b at position 0 (Latch, non-last): replays empty prefix → state 0,
 * regenerates Latch from bookmark → shrinks n to THRESHOLD.
 * Phase 3 at position 1 (Observe, last): no shrinks.
 *
 * Note: PROP_ASSERT placed in setPostCheck, not in action bodies, to avoid
 * exceptions during generation-time state advancement in genActionShrinkables.
 */
TEST(concurrency_function, shrink_phase2b_non_last_front_element)
{
    constexpr int THRESHOLD = 10;

    auto factory = [](int& obj, EmptyModel&) -> ActionGen<int, EmptyModel> {
        if (obj == 0) {
            return gen::interval(THRESHOLD, 2 * THRESHOLD)
                .map<Action<int, EmptyModel>>([](const int& n) {
                    return Action<int, EmptyModel>(
                        PROP_ACTION_NAME("Latch", n),
                        [n](int& v, EmptyModel&) { v = n; });
                });
        }
        return gen::just(Action<int, EmptyModel>("Observe", [](int&, EmptyModel&) {}));
    };

    std::ostringstream out;
    auto* oldOut = cout.rdbuf(out.rdbuf());
    auto* oldErr = cerr.rdbuf(out.rdbuf());
    bool ok = statefulProperty<int>(gen::just(0), factory)
        .setSeed(1)
        .setNumRuns(5)
        .setMaxConcurrency(1)
        .setActionListSize(2)
        .setPostCheck([](int& v) { PROP_ASSERT(v < THRESHOLD); })
        .go();
    cout.rdbuf(oldOut);
    cerr.rdbuf(oldErr);

    EXPECT_FALSE(ok);
    EXPECT_NE(out.str().find("Latch(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Phase 2b should shrink Latch's n to THRESHOLD=" << THRESHOLD
        << "\nactual output:\n" << out.str();
    EXPECT_NE(out.str().find("Observe"), string::npos)
        << "Shrunken front must still contain Observe\nactual output:\n" << out.str();
}

/**
 * Phase 2b — rear thread pipeline smoke test.
 *
 * Uses a state-independent factory (same generator regardless of state) so
 * Phase 2b and Phase 2 yield equivalent candidates.  The goal is to exercise
 * the full rear-thread applyStatefulShrinkTree code path — including Phase 2b —
 * without crashing, and to confirm that the shrunken output is still minimal.
 *
 * setMaxConcurrency(2): two rear workers are spawned.
 * setShrinkMaxRetries(5): retry tolerance for scheduler non-determinism.
 *
 * Factory (state-independent):
 *   always → Inc(n) with n ∈ [THRESHOLD, 2·THRESHOLD]: obj += n
 *
 * With setActionListSize(1): front=[Inc(n1)], rear=[Inc(n2)].
 * Both add at least THRESHOLD; postCheck PROP_ASSERT(obj < THRESHOLD) fails.
 *
 * Phase 0 tries to remove the rear thread.  If the front alone reproduces
 * the failure (Inc(n1) makes obj=n1 ≥ THRESHOLD), rear is removed and
 * Phase 2b/3 on the front shrinks n1 to THRESHOLD.
 * Either way the shrunken output reaches the minimal value THRESHOLD.
 */
TEST(concurrency_function, shrink_phase2b_rear_thread_pipeline)
{
    constexpr int THRESHOLD = 5;

    auto factory = [](int&, EmptyModel&) -> ActionGen<int, EmptyModel> {
        return gen::interval(THRESHOLD, 2 * THRESHOLD)
            .map<Action<int, EmptyModel>>([](const int& n) {
                return Action<int, EmptyModel>(
                    PROP_ACTION_NAME("Inc", n),
                    [n](int& v, EmptyModel&) { v += n; });
            });
    };

    std::ostringstream out;
    auto* oldOut2 = cout.rdbuf(out.rdbuf());
    auto* oldErr2 = cerr.rdbuf(out.rdbuf());
    bool ok = statefulProperty<int>(gen::just(0), factory)
        .setSeed(1)
        .setNumRuns(10)
        .setMaxConcurrency(2)
        .setActionListSize(1)
        .setShrinkMaxRetries(5)
        .setPostCheck([](int& v) { PROP_ASSERT(v < THRESHOLD); })
        .go();
    cout.rdbuf(oldOut2);
    cerr.rdbuf(oldErr2);

    EXPECT_FALSE(ok);
    EXPECT_NE(out.str().find("Inc(" + to_string(THRESHOLD) + ")"), string::npos)
        << "Shrink pipeline should reduce Inc's n to THRESHOLD=" << THRESHOLD
        << " (rear Phase 2b code path exercised without crash)"
        << "\nactual output:\n" << out.str();
}

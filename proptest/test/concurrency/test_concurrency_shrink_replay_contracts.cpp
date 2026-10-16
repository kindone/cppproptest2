/*
 * Contract:
 * - Failing concurrent stateful properties replay the exact saved random state
 *   for the failing run before shrinking.
 * - Shrinking can reduce generated action parameters while preserving the
 *   failure boundary that made the trace interesting.
 * - State-dependent action factories are replayed from the prefix state that
 *   existed before the action being shrunk.
 * - The rear-worker shrink pipeline can run through thread/action shrink
 *   phases and still report a minimal failing action value.
 *
 * Scope:
 * This file covers failure replay and shrink behavior for stateful properties
 * configured through the concurrency API. It does not cover successful
 * concurrent execution, callback balancing, scheduler fairness, or exact full
 * report formatting.
 *
 * Helpers:
 * - CapturedStreams redirects cout/cerr into one stringstream while a property
 *   runs so tests can inspect public failure text without writing global output.
 * - Boundary-valued action generators make the intended minimal failing action
 *   easy to identify in the shrink report.
 */

#include "proptest/generator/vector.hpp"
#include "proptest/statefultest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

using namespace proptest;
using namespace proptest::stateful;
using namespace proptest::testprofile;

namespace {

constexpr uint64_t kLaterFailureReplaySeed = 123;

class CapturedStreams {
public:
    CapturedStreams()
        : oldOut_(cout.rdbuf(output_.rdbuf())),
          oldErr_(cerr.rdbuf(output_.rdbuf()))
    {
    }

    ~CapturedStreams()
    {
        cout.rdbuf(oldOut_);
        cerr.rdbuf(oldErr_);
    }

    string text() const
    {
        return output_.str();
    }

private:
    stringstream output_;
    streambuf* oldOut_;
    streambuf* oldErr_;
};

Generator<SimpleAction<int>> boundarySimpleActionGen(const char* name, int threshold, int upper)
{
    return gen::interval(threshold, upper).map<SimpleAction<int>>(
        [name, threshold](int& value) {
            return SimpleAction<int>(PROP_ACTION_NAME(name, value), [value, threshold](int& object) {
                object += value;
                PROP_ASSERT(object < threshold);
            });
        });
}

ActionGen<int, EmptyModel> boundaryModelActionGen(const char* name, int threshold, int upper)
{
    return gen::interval(threshold, upper).map<Action<int, EmptyModel>>(
        [name, threshold](int& value) {
            return Action<int, EmptyModel>(
                PROP_ACTION_NAME(name, value),
                [value, threshold](int& object, EmptyModel&) {
                    object += value;
                    PROP_ASSERT(object < threshold);
                });
        });
}

}  // namespace

TEST(ConcurrencyShrinkReplayContracts, shrink_replays_later_failing_run_from_saved_random_state)
{
    auto initialGen = gen::interval<int>(1000, 1000000);
    auto noopGen = gen::just(SimpleAction<int>("Noop", [](int&) {}));
    auto actionGen = noopGen.template map<Action<int, EmptyModel>>(
        +[](const SimpleAction<int>& simpleAction) {
            return Action<int, EmptyModel>(simpleAction);
        });
    auto actionListGen = Arbi<list<Action<int, EmptyModel>>>(actionGen, 0, 0);

    Random expectedRand(kLaterFailureReplaySeed);
    [[maybe_unused]] const int firstInitial = initialGen(expectedRand).getRef();
    actionListGen(expectedRand);
    actionListGen(expectedRand);
    const int secondInitial = initialGen(expectedRand).getRef();
    actionListGen(expectedRand);
    actionListGen(expectedRand);

    CapturedStreams capture;
    const bool ok = statefulProperty<int>(initialGen, noopGen)
        .setSeed(kLaterFailureReplaySeed)
        .setNumRuns(kSavedRngReplayRuns)
        .setMaxConcurrency(1)
        .setActionListSize(0)
        .setPostCheck([secondInitial](int& value) {
            PROP_ASSERT(value != secondInitial);
        })
        .go();

    EXPECT_FALSE(ok);
    EXPECT_NE(capture.text().find("initial: " + to_string(secondInitial)), string::npos)
        << "shrink should replay the saved RNG state for the later failing run"
        << "\nexpected initial: " << secondInitial
        << "\nactual output:\n" << capture.text();
}

TEST(ConcurrencyShrinkReplayContracts, serial_concurrency_shrinks_last_action_parameter_to_boundary)
{
    constexpr int kThreshold = 10;

    auto actionGen = boundarySimpleActionGen("Incr", kThreshold, 100);
    CapturedStreams capture;
    const bool ok = statefulProperty<int>(gen::just(0), actionGen)
        .setSeed(kStatefulFailureSeed)
        .setNumRuns(kStatefulShortRuns)
        .setMaxConcurrency(1)
        .setActionListMinSize(1)
        .setActionListMaxSize(3)
        .go();

    EXPECT_FALSE(ok);
    EXPECT_NE(capture.text().find("Incr(" + to_string(kThreshold) + ")"), string::npos)
        << "last-action parameter shrinking should reach the failure boundary"
        << "\nactual output:\n" << capture.text();
}

TEST(ConcurrencyShrinkReplayContracts, state_dependent_prefix_shrink_replays_prefix_state)
{
    constexpr int kThreshold = 10;

    auto factory = [](int& object, EmptyModel&) -> ActionGen<int, EmptyModel> {
        if (object == 0) {
            return gen::interval(kThreshold, 2 * kThreshold)
                .map<Action<int, EmptyModel>>([](const int& value) {
                    return Action<int, EmptyModel>(
                        PROP_ACTION_NAME("Latch", value),
                        [value](int& target, EmptyModel&) {
                            target = value;
                        });
                });
        }

        return gen::just(Action<int, EmptyModel>("Observe", [](int&, EmptyModel&) {}));
    };

    CapturedStreams capture;
    const bool ok = statefulProperty<int>(gen::just(0), factory)
        .setSeed(kStatefulFailureSeed)
        .setNumRuns(kStatefulFocusedRuns)
        .setMaxConcurrency(1)
        .setActionListSize(2)
        .setPostCheck([](int& value) {
            PROP_ASSERT(value < kThreshold);
        })
        .go();

    EXPECT_FALSE(ok);
    EXPECT_NE(capture.text().find("Latch(" + to_string(kThreshold) + ")"), string::npos)
        << "prefix-parameter shrinking should replay from the pre-Latch state"
        << "\nactual output:\n" << capture.text();
    EXPECT_NE(capture.text().find("Observe"), string::npos)
        << "the replayed prefix scenario should still include the dependent follow-up action"
        << "\nactual output:\n" << capture.text();
}

TEST(ConcurrencyShrinkReplayContracts, rear_worker_shrink_pipeline_reports_minimal_boundary_action)
{
    constexpr int kThreshold = 5;

    auto factory = [](int&, EmptyModel&) -> ActionGen<int, EmptyModel> {
        return boundaryModelActionGen("Inc", kThreshold, 2 * kThreshold);
    };

    CapturedStreams capture;
    const bool ok = statefulProperty<int>(gen::just(0), factory)
        .setSeed(kStatefulFailureSeed)
        .setNumRuns(kSmokeRuns)
        .setMaxConcurrency(2)
        .setActionListSize(1)
        .setShrinkMaxRetries(kShrinkConcurrencyRetryLimit)
        .setPostCheck([](int& value) {
            PROP_ASSERT(value < kThreshold);
        })
        .go();

    EXPECT_FALSE(ok);
    EXPECT_NE(capture.text().find("Inc(" + to_string(kThreshold) + ")"), string::npos)
        << "rear-worker shrink pipeline should reach the minimal failing action value"
        << "\nactual output:\n" << capture.text();
}

TEST(ConcurrencyShrinkReplayContracts, expectation_failures_are_reported_and_shrunk)
{
    constexpr int kThreshold = 5;
    auto actionGen = gen::interval(kThreshold, 2 * kThreshold).map<SimpleAction<int>>(
        [](const int& value) {
            return SimpleAction<int>(PROP_ACTION_NAME("ExpectBelow", value), [value](int& object) {
                object += value;
                PROP_EXPECT(object < kThreshold) << "object: " << object;
            });
        });

    CapturedStreams capture;
    const bool ok = statefulProperty<int>(gen::just(0), actionGen)
        .setSeed(kStatefulFailureSeed)
        .setNumRuns(1)
        .setActionListSize(1)
        .go();

    EXPECT_FALSE(ok);
    EXPECT_NE(capture.text().find("ExpectBelow(" + to_string(kThreshold) + ")"), string::npos)
        << "expectation failure should shrink to the boundary"
        << "\nactual output:\n" << capture.text();
    EXPECT_NE(capture.text().find("object: 5"), string::npos)
        << "expectation details should be preserved"
        << "\nactual output:\n" << capture.text();
    EXPECT_EQ(capture.text().find("context is not set"), string::npos)
        << "stateful actions should have an active property context"
        << "\nactual output:\n" << capture.text();
}

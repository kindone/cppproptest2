/*
 * Contract:
 * - Property.example(), Property.matrix(), and Property.forAll() execute in the
 *   order they are chained.
 * - A deterministic example or matrix failure prevents later generated runs
 *   from executing.
 * - A generated failure prevents later deterministic examples from executing.
 * - When setConfig() and fluent setters both specify numRuns, the later call is
 *   the one used by the following forAll().
 *
 * Scope:
 * This file covers the chain boundary between deterministic examples, finite
 * matrices, generated runs, and run-count configuration. It does not cover
 * failure message formatting, assertion macro text, shrink retry, lifecycle
 * callbacks, or broad generator behavior.
 *
 * Helpers:
 * - The order test records values in a vector: example(0), matrix({1, 2}), then
 *   kSmokeRuns generated values from forAll(gen::just(3)).
 * - The configuration test counts generated runs after setConfig().setNumRuns()
 *   and after setNumRuns().setConfig() to prove latest-writer-wins.
 * - The short-circuit tests increment counters only in steps that must be
 *   skipped after an earlier failure.
 */

#include "proptest/Property.hpp"
#include "proptest/combinator/combinators.hpp"
#include "proptest/gen.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

using namespace proptest;
using namespace proptest::testprofile;

TEST(PropertyApiComposition, examples_matrices_and_forall_share_one_ordered_chain)
{
    vector<int> seen;
    auto prop = property([&seen](int value) {
        seen.push_back(value);
        return true;
    });

    const bool ok = prop.setConfig({.numRuns = kSmokeRuns}).example(0).matrix({1, 2}).forAll(gen::just(3));

    ASSERT_TRUE(ok);
    vector<int> expected{0, 1, 2};
    expected.insert(expected.end(), kSmokeRuns, 3);
    EXPECT_EQ(seen, expected);
}

TEST(PropertyApiComposition, latest_run_count_configuration_wins)
{
    int batchThenFluentRuns = 0;
    auto batchThenFluent = property([&batchThenFluentRuns](int) {
        batchThenFluentRuns++;
        return true;
    });

    ASSERT_TRUE(batchThenFluent.setConfig({.numRuns = kSmokeRuns}).setNumRuns(kTinyRuns).forAll(gen::just(1)));
    EXPECT_EQ(batchThenFluentRuns, kTinyRuns);

    int fluentThenBatchRuns = 0;
    auto fluentThenBatch = property([&fluentThenBatchRuns](int) {
        fluentThenBatchRuns++;
        return true;
    });

    ASSERT_TRUE(fluentThenBatch.setNumRuns(kTinyRuns).setConfig({.numRuns = kSmokeRuns}).forAll(gen::just(1)));
    EXPECT_EQ(fluentThenBatchRuns, kSmokeRuns);
}

TEST(PropertyApiComposition, deterministic_failure_short_circuits_later_chain_steps)
{
    int generatedRuns = 0;
    int matrixRuns = 0;

    auto prop = property([&](int value) {
        if (value == 0) {
            return false;
        }
        if (value == 7) {
            matrixRuns++;
        }
        if (value == 9) {
            generatedRuns++;
        }
        return true;
    });

    const bool ok = prop.example(0).matrix({7}).forAll(gen::just(9));

    EXPECT_FALSE(ok);
    EXPECT_EQ(matrixRuns, 0);
    EXPECT_EQ(generatedRuns, 0);
}

TEST(PropertyApiComposition, generated_failure_short_circuits_following_examples)
{
    int exampleRuns = 0;
    auto prop = property([&exampleRuns](int value) {
        if (value == 42) {
            exampleRuns++;
        }
        return value > 0;
    });

    const bool ok = prop.setNumRuns(kTinyRuns).forAll(gen::just(0)).example(42);

    EXPECT_FALSE(ok);
    EXPECT_EQ(exampleRuns, 0);
}

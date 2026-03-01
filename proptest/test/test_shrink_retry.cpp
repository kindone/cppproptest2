/**
 * Shrink retry tests for flaky/non-deterministic properties.
 *
 * Covers: confirmation runs, shrink with retries, setOnReproductionStats callback,
 * config propagation, zero-confirmation (deterministic) mode.
 *
 * Uses getLastReproductionStats() and setOnReproductionStats() for programmatic
 * access to reproduction stats. See docs/Shrinking.md and docs/PropertyAPI.md.
 */

#include "proptest/Property.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/gen.hpp"
#include "proptest/stateful/stateful_function.hpp"
#include "proptest/PropertyBase.hpp"
#include <regex>
#include <chrono>
#include <thread>
using namespace proptest;
using namespace proptest::stateful;


// --- 1. Confirmation runs ---
// Flaky ~50% (fail when count%2==0); shrinkMaxRetries=10. X between 1-9 (relax to 0-10 for rare extremes).
// Wrapped in outer forAll to run with multiple seeds, avoiding accidental pass from a lucky seed.
TEST(ShrinkRetry, confirmation_runs)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        int n = 0;
        auto prop = property([&n](int x) {
            (void)x;
            n++;
            if (n == 1 || n % 2 == 0) {  // fail first run, then 50% flaky
                PROP_ASSERT(false);
            }
        });
        auto result = prop.setSeed(seed).setNumRuns(1).setShrinkMaxRetries(10).forAll(gen::interval(0, 100));
        PROP_ASSERT(!result);  // property must fail
        auto stats = result.getLastReproductionStats();
        PROP_ASSERT(stats.has_value());
        PROP_ASSERT(stats->numReproduced == kShrinkAssessmentRuns/2);
        PROP_ASSERT(stats->totalRuns == kShrinkAssessmentRuns);
    }, gen::uint64().noShrink());
}

// --- 2. Shrink with retries ---
// Same flaky property; shrinking completes, shrunk example reported.
// Wrapped in outer forAll to run with multiple seeds.
TEST(ShrinkRetry, shrink_with_retries)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        int n = 0;
        int lastArg = -999;
        auto prop = property([&n, &lastArg](int x) {
            n++;
            if (n == 1 || n % 2 == 0) {  // fail first run, then 50% flaky
                lastArg = x;
                PROP_ASSERT(false);
            }
        });
        auto result = prop.setSeed(seed).setNumRuns(1).setShrinkMaxRetries(10).forAll(gen::interval(0, 100));
        PROP_ASSERT(!result);
        PROP_ASSERT(lastArg == 0);  // shrunk to 0
        auto stats2 = result.getLastReproductionStats();
        PROP_ASSERT(stats2.has_value());
        PROP_ASSERT(stats2->numReproduced == kShrinkAssessmentRuns / 2);
        PROP_ASSERT(stats2->totalRuns == kShrinkAssessmentRuns);
        // should match  { n } for n in 0..100 (regex match)
        PROP_ASSERT(std::regex_match(stats2->argsAsString, std::regex("\\{ \\d+ \\}")));
    }, gen::uint64().noShrink());
}

// --- 3. Report on each shrink ---
// Use setOnReproductionStats callback to count assessments.
// Wrapped in outer forAll to run with multiple seeds.
TEST(ShrinkRetry, report_on_each_shrink)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        int n = 0;
        size_t statsCount = 0;
        ReproductionStats stats;
        auto prop = property([&n](int) {
            n++;
            if (n % 2 == 0) {
                PROP_ASSERT(false);
            }
        });
        prop.setSeed(seed).setNumRuns(10).setShrinkMaxRetries(10)
            .setOnReproductionStats([&statsCount, &stats](const ReproductionStats& _stats) {
                statsCount++;
                stats = _stats;
            })
            .forAll(gen::interval(0, 100));
        PROP_EXPECT_EQ(statsCount, 1U);
        PROP_EXPECT_EQ(stats.numReproduced, kShrinkAssessmentRuns/2);
        PROP_EXPECT_EQ(stats.totalRuns, kShrinkAssessmentRuns);
        PROP_EXPECT_TRUE(std::regex_match(stats.argsAsString, std::regex("\\{ \\d+ \\}")));
    }, gen::uint64().noShrink());
}

// --- 4. Timeouts ---
// Test shrink timeout mechanism.
// Use setMaxDurationMs and setShrinkTimeoutMs to test timeout mechanism.
// Wrapped in outer forAll to run with multiple seeds.
TEST(ShrinkRetry, shrink_max_timeout)
{
    forAll([](uint64_t seed, size_t sleepMs, int failDenominator) {
        int n = 0;
        auto prop = property([&n, sleepMs, failDenominator](int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
            n++;
            if (n % failDenominator == 0) {
                PROP_ASSERT(false);
            }
        });
        auto result = prop.setSeed(seed).setNumRuns(1000).setMaxDurationMs(100).setShrinkMaxRetries(100).setShrinkTimeoutMs(100).forAll(gen::interval(0, 20));
        PROP_ASSERT(!result);
        auto stats = result.getLastReproductionStats();
        PROP_ASSERT(stats.has_value());  // need shrinkMaxRetries>0 for assessment
        PROP_EXPECT_EQ(stats->totalRuns, kShrinkAssessmentRuns);
        PROP_STAT(stats->elapsedSec);
        PROP_STAT(stats->numReproduced);
        PROP_STAT(stats->totalRuns);
    }, {.maxDurationMs = 5000, .shrinkTimeoutMs = 5000}, gen::uint64().noShrink(), gen::interval<size_t>(0, 100), gen::interval(1, 100)).example(1, 63, 16);
}

// ========== Stateful ShrinkRetry tests (mirror stateless approaches) ==========

// --- Stateful 1. Confirmation runs ---
// Flaky ~50% stateful action; shrinkMaxRetries=10. Verify ReproductionStats.
TEST(ShrinkRetryStateful, confirmation_runs)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        using T = vector<int>;
        int n = 0;
        auto failWhenNonEmpty = SimpleAction<T>([&n](T& obj) {
            n++;
            if (!obj.empty() && (n == 1 || n % 2 == 0))
                PROP_ASSERT(false);
        });
        auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int v) {
            return SimpleAction<T>([v](T& obj) { obj.push_back(v); });
        });
        auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, failWhenNonEmpty);
        auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
        bool result = prop.setSeed(seed).setNumRuns(20).setShrinkMaxRetries(10).go();
        PROP_ASSERT(!result);
        auto stats = prop.getLastReproductionStats();
        PROP_ASSERT(stats.has_value());
        PROP_ASSERT(stats->numReproduced >= 1 && stats->numReproduced <= kShrinkAssessmentRuns);
        PROP_ASSERT(stats->totalRuns == kShrinkAssessmentRuns);
    }, gen::uint64().noShrink());
}

// --- Stateful 2. Shrink with retries ---
// Same flaky stateful property; shrinking completes, shrunk action sequence reported.
TEST(ShrinkRetryStateful, shrink_with_retries)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        using T = vector<int>;
        int n = 0;
        auto failWhenNonEmpty = SimpleAction<T>([&n](T& obj) {
            n++;
            if (!obj.empty() && (n == 1 || n % 2 == 0))
                PROP_ASSERT(false);
        });
        auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int v) {
            return SimpleAction<T>([v](T& obj) { obj.push_back(v); });
        });
        auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, failWhenNonEmpty);
        auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
        bool result = prop.setSeed(seed).setNumRuns(20).setShrinkMaxRetries(10).go();
        if (!result) {
            auto stats = prop.getLastReproductionStats();
            PROP_ASSERT(stats.has_value());
            PROP_ASSERT(stats->numReproduced >= 1 && stats->numReproduced <= kShrinkAssessmentRuns);
            PROP_ASSERT(stats->totalRuns == kShrinkAssessmentRuns);
            PROP_ASSERT(!stats->argsAsString.empty());
        }
    }, gen::uint64().noShrink());
}

// --- Stateful 3. Report on each shrink ---
// Use setOnReproductionStats callback to count assessments.
TEST(ShrinkRetryStateful, report_on_each_shrink)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        using T = vector<int>;
        int n = 0;
        size_t statsCount = 0;
        ReproductionStats stats;
        auto failWhenNonEmpty = SimpleAction<T>([&n](T& obj) {
            n++;
            if (!obj.empty() && n % 2 == 0)
                PROP_ASSERT(false);
        });
        auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int v) {
            return SimpleAction<T>([v](T& obj) { obj.push_back(v); });
        });
        auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, failWhenNonEmpty);
        auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
        bool result = prop.setSeed(seed).setNumRuns(20).setShrinkMaxRetries(10)
            .setOnReproductionStats([&statsCount, &stats](const ReproductionStats& _stats) {
                statsCount++;
                stats = _stats;
            })
            .go();
        if (result)
            return;  // passed, no failure to validate
        PROP_EXPECT_EQ(statsCount, 1U);
        PROP_EXPECT_GE(stats.numReproduced, 1);
        PROP_EXPECT_LE(stats.numReproduced, kShrinkAssessmentRuns);
        PROP_EXPECT_EQ(stats.totalRuns, kShrinkAssessmentRuns);
    }, gen::uint64().noShrink());
}

// --- Stateful 4. Config propagation ---
// shrinkMaxRetries=1; confirm deterministic-like shrink (one run per candidate).
TEST(ShrinkRetryStateful, config_propagation)
{
    EXPECT_FOR_ALL([](uint64_t seed) {
        using T = vector<int>;
        int n = 0;
        auto failWhenNonEmpty = SimpleAction<T>([&n](T& obj) {
            n++;
            if (!obj.empty() && n % 2 == 0)
                PROP_ASSERT(obj.empty());  // fail when non-empty
        });
        auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int v) {
            return SimpleAction<T>([v](T& obj) { obj.push_back(v); });
        });
        auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, failWhenNonEmpty);
        auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
        prop.setSeed(seed).setNumRuns(20).setShrinkMaxRetries(1).go();
    }, gen::uint64().noShrink());
}

// --- Stateful 5. Timeouts
// Test shrink timeout mechanism.
// Use setMaxDurationMs and setShrinkTimeoutMs to test timeout mechanism.
// Wrapped in outer forAll to run with multiple seeds.
TEST(ShrinkRetryStateful, shrink_max_timeout)
{
    forAll([](uint64_t seed, size_t sleepMs, int failDenominator) {
        using T = vector<int>;
        int n = 0;
        auto failWhenNonEmpty = SimpleAction<T>([&n, sleepMs, failDenominator](T& obj) {
            n++;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
            if (!obj.empty() && n % failDenominator == 0)
                PROP_ASSERT(false);
        });
        auto pushBackGen = gen::int32().map<SimpleAction<T>>([](int v) {
            return SimpleAction<T>([v](T& obj) { obj.push_back(v); });
        });
        auto actionGen = gen::oneOf<SimpleAction<T>>(pushBackGen, failWhenNonEmpty);
        auto prop = statefulProperty<T>(Arbi<T>(), actionGen);
        bool result = prop.setSeed(seed).setNumRuns(1000).setMaxDurationMs(100).setShrinkMaxRetries(100).setShrinkTimeoutMs(100).go();
        PROP_ASSERT(!result);
        auto stats = prop.getLastReproductionStats();
        PROP_ASSERT(stats.has_value());
        PROP_ASSERT(stats->numReproduced >= 1 && stats->numReproduced <= kShrinkAssessmentRuns);
        PROP_ASSERT(stats->totalRuns == kShrinkAssessmentRuns);
        PROP_STAT(stats->elapsedSec);
        PROP_STAT(stats->numReproduced);
        PROP_STAT(stats->totalRuns);
    }, {.maxDurationMs = 5000, .shrinkTimeoutMs = 5000}, gen::uint64().noShrink(), gen::interval<size_t>(0, 100), gen::interval(1, 100));
}
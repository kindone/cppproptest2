/*
 * Contract:
 * - forAll() uses the seed, run count, lifecycle callbacks, and streams supplied
 *   through ForAllConfig.
 * - Property::setConfig() and individual stream setters affect the next
 *   execution without changing the generated domain.
 * - Examples that fail through PROP_ASSERT and failed statistical assertions
 *   are reported to the configured error stream, while statistical summaries
 *   are reported to the configured output stream.
 *
 * Scope:
 * This file covers property runner configuration and reporting surfaces. It
 * does not cover chain ordering, shrink retry, exact shrink-tree structure, or
 * generator domain correctness.
 *
 * Helpers:
 * - Replay tests collect generated integers from two runs with the same seed and
 *   compare the traces.
 * - Lifecycle counters prove startup and cleanup run once for each successful
 *   generated case.
 * - String streams replace cout/cerr so assertions inspect the public report
 *   text without writing to the process-global streams.
 */

#include "proptest/Property.hpp"
#include "proptest/gen.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

using namespace proptest;
using namespace proptest::testprofile;

TEST(PropertyRunnerReporting, config_seed_replays_trace_and_lifecycle_wraps_each_generated_case)
{
    vector<int> firstTrace;
    vector<int> secondTrace;
    int startupCount = 0;
    int cleanupCount = 0;
    stringstream firstOutput;
    stringstream secondOutput;

    auto first = property([&firstTrace](int value) {
        firstTrace.push_back(value);
        return true;
    });
    ASSERT_TRUE(first.setConfig({
        .seed = kPropertyConfigReplaySeed,
        .numRuns = kConfigAllRuns,
        .onStartup = [&startupCount]() { startupCount++; },
        .onCleanup = [&cleanupCount]() { cleanupCount++; },
        .outputStream = &firstOutput
    }).forAll(gen::interval(0, 100)));

    auto second = property([&secondTrace](int value) {
        secondTrace.push_back(value);
        return true;
    });
    ASSERT_TRUE(second.setConfig({
        .seed = kPropertyConfigReplaySeed,
        .numRuns = kConfigAllRuns,
        .outputStream = &secondOutput
    }).forAll(gen::interval(0, 100)));

    EXPECT_EQ(firstTrace, secondTrace);
    EXPECT_EQ(startupCount, kConfigAllRuns);
    EXPECT_EQ(cleanupCount, kConfigAllRuns);
    EXPECT_NE(firstOutput.str().find("OK, passed " + to_string(kConfigAllRuns) + " tests"), string::npos);
}

TEST(PropertyRunnerReporting, assertion_failed_example_uses_configured_error_stream)
{
    stringstream err;
    auto prop = property([](int value) {
        PROP_ASSERT(value < 0);
        return true;
    });

    EXPECT_FALSE(prop.setErrorStream(err).example(7));

    const string report = err.str();
    EXPECT_NE(report.find("example failed"), string::npos);
    EXPECT_NE(report.find("with args: { 7 }"), string::npos);
}

TEST(PropertyRunnerReporting, stat_assertion_failure_uses_configured_streams)
{
    stringstream out;
    stringstream err;

    const bool ok = forAll([](int value) {
        PROP_STAT_ASSERT_GE(value > 0, 0.5);
        return true;
    }, {
        .numRuns = kSmokeRuns,
        .outputStream = &out,
        .errorStream = &err
    }, gen::interval(-10, -1));

    EXPECT_FALSE(ok);
    EXPECT_NE(err.str().find("Stat assertion failed"), string::npos);
    EXPECT_NE(err.str().find("PROP_STAT_ASSERT_GE(value > 0"), string::npos);
    EXPECT_NE(out.str().find("value > 0"), string::npos);
}

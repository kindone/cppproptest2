#pragma once

#include <cstddef>
#include <cstdint>

namespace proptest::testprofile {

// Cross-suite replay fixtures. Keep component-local seeds in the test file
// that explains the scenario they replay.
constexpr int64_t kCompatibilitySeed = 42;
constexpr int64_t kStatefulReplaySeed = 0;
constexpr int64_t kStatefulFailureSeed = 1;
constexpr int64_t kPropertyConfigReplaySeed = 999;

constexpr size_t kMinimalSamples = 2;
constexpr size_t kFocusedSamples = 5;
constexpr size_t kSmokeSamples = 10;
constexpr size_t kShortSamples = 20;
constexpr size_t kGeneratorContractSamples = 50;
constexpr size_t kFloatingDomainSamples = 100;
constexpr size_t kShrinkTraversalLimit = 1000;

constexpr int kTinyRuns = 1;
constexpr int kSavedRngReplayRuns = 2;
constexpr int kChainedForAllRuns = 3;
constexpr int kConfigAllRuns = 5;
constexpr int kStatefulFocusedRuns = 5;
constexpr int kSmokeRuns = 10;
constexpr int kStatefulShortRuns = 20;
constexpr int kPartialConfigRuns = 25;
constexpr int kMediumRuns = 50;
constexpr int kContractRuns = 100;
constexpr int kStatefulContractRuns = 100;
constexpr int kStatefulFrontierRuns = 200;
constexpr int kFailureSearchRuns = 50;
constexpr int kShrinkContractRuns = 5;
constexpr int kStatisticalRuns = 1000;
constexpr int kDurationGuardRuns = 1000000;
constexpr int kExtremeDurationGuardRuns = 10000000;
constexpr int kGeneratorPerformanceRuns = 100000;
constexpr int kAnyPerformanceRuns = 1000000;
constexpr int kStructuralPerformanceIterations = 10000;
constexpr int kTraversalPerformanceDepth = 100;
constexpr int kChainedOperationDepth = 10;
constexpr int kShrinkerPerformanceIterations = 1000;
constexpr int kShrinkIntegralBenchmarkValue = 100000;
constexpr size_t kLargeObjectElements = 1000;

constexpr int kStatefulShortActions = 5;
constexpr int kStatefulContractActions = 20;
constexpr int kStatefulDeepActions = 50;
constexpr int kIllustrativeSamples = 3;

constexpr int kShrinkRetryTimeoutMs = 100;
constexpr int kShrinkSearchRuns = 200;
constexpr int kShrinkSmokeRetryLimit = 2;
constexpr int kShrinkConcurrencyRetryLimit = 5;
constexpr int kShrinkRetryLimit = 10;
constexpr int kShrinkShortRetryLimit = 1;
constexpr int kShrinkStatefulRetryLimit = 20;
constexpr int kShrinkBroadRetryLimit = 100;
constexpr int kShrinkShortTimeoutMs = 10;
constexpr int kShrinkLongTimeoutMs = 200;
constexpr int kShrinkAssessmentTimeoutMs = 500;
constexpr int kConfigAllTimeoutMs = 1000;
constexpr int kDurationGuardTimeoutMs = 2000;

}  // namespace proptest::testprofile

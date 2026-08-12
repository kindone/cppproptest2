/*
 * Contract:
 * - Selection combinators generate only values from their declared alternatives.
 * - Weighted alternatives keep membership guarantees while making dominant
 *   branches observable under bounded sampling.
 * - Raw-value alternatives and generator alternatives compose through the same
 *   public API surface.
 *
 * Scope:
 * This file covers root generation for oneOf, elementOf, raw alternatives,
 * generator alternatives, and weighted alternatives. It does not assert exact
 * distribution percentages or exact shrink-tree text.
 *
 * Helpers:
 * - Generated Seed values provide replayable generator exploration while local
 *   loops sample one generated selection stream.
 * - Deterministic example seeds pin readable witnesses beside the generated
 *   exploration.
 */

#include "proptest/Random.hpp"
#include "proptest/Property.hpp"
#include "proptest/combinator/combinators.hpp"
#include "proptest/gen.hpp"
#include "proptest/generator/seed.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

#include <map>
#include <set>

using namespace proptest;
using namespace proptest::testprofile;

namespace {

constexpr int kDominantSelection = 1339;
constexpr int kRareSelection = 42;
constexpr int kRangeLow = 100;
constexpr int kRangeHigh = 200;

bool isOneOfMixedDomainValue(int value)
{
    return value == kDominantSelection || value == kRareSelection || (kRangeLow <= value && value <= kRangeHigh);
}

}  // namespace

TEST(SelectionCombinatorContracts, oneof_roots_stay_inside_raw_and_generated_alternatives)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed.value);
        auto generator = gen::oneOf<int>(
            kDominantSelection,
            gen::interval(kRangeLow, kRangeHigh),
            kRareSelection);

        for (int i = 0; i < static_cast<int>(kGeneratorContractSamples); i++) {
            PROP_ASSERT(isOneOfMixedDomainValue(generator(rand).getRef()));
        }
        return true;
    }, gen::seed()).example(Seed(kCompatibilitySeed))
                   .forAll());
}

TEST(SelectionCombinatorContracts, elementof_roots_stay_inside_declared_finite_set)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed.value);
        const std::set<int> allowed = {2, 5, 10};
        auto generator = gen::elementOf<int>(2, 5, 10);

        for (int i = 0; i < static_cast<int>(kGeneratorContractSamples); i++) {
            PROP_ASSERT(allowed.count(generator(rand).getRef()) == 1);
        }
        return true;
    }, gen::seed()).example(Seed(kCompatibilitySeed))
                   .forAll());
}

TEST(SelectionCombinatorContracts, weighted_oneof_keeps_membership_and_exercises_dominant_branch)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed.value);
        auto generator = gen::oneOf<int>(
            gen::weightedGen<int>(kDominantSelection, 0.8),
            gen::weightedGen<int>(kRareSelection, 0.1),
            gen::weightedGen(gen::interval(kRangeLow, kRangeHigh), 0.1));

        std::map<int, int> exactCounts;
        int rangeCount = 0;
        for (int i = 0; i < kStatisticalRuns; i++) {
            const int value = generator(rand).getRef();
            PROP_ASSERT(isOneOfMixedDomainValue(value));
            if (value == kDominantSelection || value == kRareSelection)
                exactCounts[value]++;
            else
                rangeCount++;
        }

        PROP_ASSERT(exactCounts[kDominantSelection] > exactCounts[kRareSelection]);
        PROP_ASSERT(exactCounts[kDominantSelection] > rangeCount);
        return true;
    }, gen::seed()).example(Seed(kCompatibilitySeed))
                   .forAll());
}

TEST(SelectionCombinatorContracts, weighted_elementof_keeps_membership_and_exercises_weight_order)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed.value);
        auto generator = gen::elementOf<int>(
            gen::weighted<int>(2, 0.8),
            gen::weighted<int>(5, 0.15),
            10);

        std::map<int, int> counts;
        for (int i = 0; i < kStatisticalRuns; i++) {
            const int value = generator(rand).getRef();
            PROP_ASSERT(value == 2 || value == 5 || value == 10);
            counts[value]++;
        }

        PROP_ASSERT(counts[2] > counts[5]);
        PROP_ASSERT(counts[5] > counts[10]);
        return true;
    }, gen::seed()).example(Seed(kCompatibilitySeed))
                   .forAll());
}

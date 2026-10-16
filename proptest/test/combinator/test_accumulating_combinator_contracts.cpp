/*
 * Contract:
 * - aggregate() builds a vector whose first element comes from the base
 *   generator and whose later elements come from generators selected by the
 *   previously generated value.
 * - accumulate() returns the final accumulated value after a bounded number of
 *   dependent generation steps.
 * - Generated roots and bounded shrink candidates must preserve the dependent
 *   transition contract chosen by the combinator.
 *
 * Scope:
 * This file covers aggregate/accumulate generation and shrink-domain
 * preservation for simple dependent integer/list traces. It does not cover
 * weighted branch distribution, exact serialized shrink-tree text, recursive
 * combinators, or compile-only template compatibility.
 *
 * Helpers:
 * - adjacentStepsStayWithin() checks the aggregate trace rule: each value is
 *   generated from the previous value and may increase by at most two.
 * - listAppendsStayWithin() checks the accumulate trace rule: each appended
 *   value is equal to the previous value or one greater.
 * - Generated Seed values are replay inputs for the aggregate/accumulate
 *   generator under test; explicit Random construction is kept inside property
 *   bodies so the test still runs through property/example/forAll.
 * - collectShrinkViolations() walks a bounded shrink tree and records values
 *   that violate the component contract.
 */

#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"
#include "proptest/test/testutil.hpp"

#include <iterator>

using namespace proptest;
using namespace proptest::testprofile;

namespace {

constexpr size_t kAggregateMinSize = 2;
constexpr size_t kAggregateMaxSize = 5;
constexpr size_t kAccumulateMinSteps = 2;
constexpr size_t kAccumulateMaxSteps = 4;

bool adjacentStepsStayWithin(const vector<int>& values)
{
    if (values.size() < kAggregateMinSize || values.size() > kAggregateMaxSize) {
        return false;
    }
    for (size_t index = 1; index < values.size(); ++index) {
        const int previous = values[index - 1];
        const int current = values[index];
        if (current < previous || current > previous + 2) {
            return false;
        }
    }
    return true;
}

bool listAppendsStayWithin(const list<int>& values)
{
    const size_t expectedMinLength = kAccumulateMinSteps + 1;
    const size_t expectedMaxLength = kAccumulateMaxSteps + 1;
    if (values.size() < expectedMinLength || values.size() > expectedMaxLength) {
        return false;
    }

    auto previous = values.begin();
    for (auto current = util::next(previous); current != values.end(); ++current, ++previous) {
        if (*current < *previous || *current > *previous + 1) {
            return false;
        }
    }
    return true;
}

Generator<vector<int>> dependentAggregateGen()
{
    return gen::aggregate(
        gen::interval<int>(0, 3),
        [](const int& previous) {
            return gen::interval(previous, previous + 2);
        },
        kAggregateMinSize,
        kAggregateMaxSize);
}

Generator<list<int>> dependentAccumulateGen()
{
    auto base = gen::interval<int>(0, 1).map<list<int>>([](const int& value) {
        return list<int>{value};
    });

    return gen::accumulate(
        base,
        [](const list<int>& previousValues) {
            const int previous = previousValues.back();
            return gen::interval(previous, previous + 1).map<list<int>>([previousValues](const int& value) {
                list<int> nextValues(previousValues);
                nextValues.push_back(value);
                return nextValues;
            });
        },
        kAccumulateMinSteps,
        kAccumulateMaxSteps);
}

template <typename T, typename Predicate>
vector<T> collectShrinkViolations(const Shrinkable<T>& root, Predicate predicate)
{
    vector<T> violations;
    vector<Shrinkable<T>> pending{root};

    for (size_t index = 0; index < pending.size() && index < kShrinkTraversalLimit; ++index) {
        const Shrinkable<T> current = pending[index];
        if (!predicate(current.getRef())) {
            violations.push_back(current.getRef());
        }

        auto shrinks = current.getShrinks();
        for (auto itr = shrinks.template iterator<typename Shrinkable<T>::StreamElementType>();
             itr.hasNext() && pending.size() < kShrinkTraversalLimit;) {
            pending.push_back(itr.next());
        }
    }

    return violations;
}

}  // namespace

TEST(AccumulatingCombinatorContracts, aggregate_has_deterministic_dependent_witness)
{
    EXPECT_TRUE(property([](Seed seed) {
        auto generator = gen::aggregate(
            gen::just<int>(1),
            [](const int& previous) {
                return gen::just<int>(previous + 1);
            },
            3,
            3);
        Random rand(seed);

        PROP_ASSERT_EQ(generator(rand).getRef(), vector<int>({1, 2, 3}));
        return true;
    }, gen::seed()).setNumRuns(kContractRuns).forAll());
}

TEST(AccumulatingCombinatorContracts, accumulate_has_deterministic_dependent_witness)
{
    EXPECT_TRUE(property([](Seed seed) {
        auto generator = gen::accumulate(
            gen::interval<int>(1, 1).map<list<int>>([](const int& value) {
                return list<int>{value};
            }),
            [](const list<int>& previousValues) {
                const int next = previousValues.back() + 1;
                return gen::interval<int>(next, next).map<list<int>>([previousValues](const int& value) {
                    list<int> nextValues(previousValues);
                    nextValues.push_back(value);
                    return nextValues;
                });
            },
            2,
            2);
        Random rand(seed);

        PROP_ASSERT(generator(rand).getRef() == list<int>({1, 2, 3}));
        return true;
    }, gen::seed()).setNumRuns(kContractRuns).forAll());
}

TEST(AccumulatingCombinatorContracts, aggregate_roots_and_shrinks_preserve_dependent_step_contract)
{
    EXPECT_TRUE(property([](Seed seed) {
        auto generator = dependentAggregateGen();
        Random rand(seed);
        const auto root = generator(rand);
        PROP_ASSERT(adjacentStepsStayWithin(root.getRef()));

        const auto violations = collectShrinkViolations(root, adjacentStepsStayWithin);
        PROP_ASSERT(violations.empty());
        return true;
    }, gen::seed()).example(Seed(kCompatibilitySeed))
      .setNumRuns(kShrinkContractRuns)
      .forAll());
}

TEST(AccumulatingCombinatorContracts, accumulate_roots_and_shrinks_preserve_append_contract)
{
    EXPECT_TRUE(property([](Seed seed) {
        auto generator = dependentAccumulateGen();
        Random rand(seed);
        const auto root = generator(rand);
        PROP_ASSERT(listAppendsStayWithin(root.getRef()));

        const auto violations = collectShrinkViolations(root, listAppendsStayWithin);
        PROP_ASSERT(violations.empty());
        return true;
    }, gen::seed()).example(Seed(kCompatibilitySeed))
      .setNumRuns(kShrinkContractRuns)
      .forAll());
}

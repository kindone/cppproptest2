/*
 * Contract:
 * - transform()/map() must expose only values produced by applying the mapper
 *   to values from the source generator, including bounded shrink candidates.
 * - filter()/suchThat() must expose only roots and bounded shrink candidates
 *   accepted by the predicate.
 * - derive()/pairWith() and chain()/tupleWith() must respect the dependency
 *   chosen from the earlier generated value.
 *
 * Scope:
 * This file covers root generation and shrink-domain preservation for
 * transformation, filtering, and dependent tuple/pair combinators. It does not
 * cover selection weighting, aggregate/accumulate traces, exact serialized
 * shrink-tree text, or compile-only template compatibility.
 *
 * Helpers:
 * - collectShrinkViolations() walks a bounded breadth-first view of a shrink
 *   tree and returns every visited value that fails the supplied invariant.
 * - Generated Seed values are replay inputs for the generators under test;
 *   explicit Random construction is kept inside property bodies so replay
 *   checks still run through property/example/matrix/forAll.
 * - The deterministic matrix pins two readable replay seeds beside the
 *   generated seed coverage.
 */

#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

using namespace proptest;
using namespace proptest::testprofile;

namespace {

constexpr int64_t kTransformationReplaySeed = 20260812;
constexpr int64_t kDependencyReplaySeed = 1339;

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

bool isMappedEvenDomain(int value)
{
    return value >= -40 && value <= 40 && value % 2 == 0;
}

bool isPositiveMultipleOfThree(int value)
{
    return value > 0 && value % 3 == 0;
}

bool isValidDependentPair(const pair<int, int>& value)
{
    return value.first >= 0 && value.first <= 5 && value.second >= value.first && value.second <= value.first + 2;
}

bool isValidDependentChain(const Chain<int, int>& value)
{
    const int first = get<0>(value);
    const int second = get<1>(value);
    return first >= 0 && first <= 5 && second >= first && second <= first + 2;
}

bool isValidDependentTriple(const Chain<int, int, int>& value)
{
    const int first = get<0>(value);
    const int second = get<1>(value);
    const int third = get<2>(value);
    return isValidDependentChain(Chain<int, int>(first, second)) && third >= second && third <= second + 2;
}

vector<pair<int, int>> collectPairWithTrace(Seed seed, size_t sampleCount)
{
    auto paired = gen::interval<int>(0, 5).pairWith<int>([](const int& value) {
        return gen::interval<int>(value, value + 2);
    });
    Random rand(seed);
    vector<pair<int, int>> trace;
    for (size_t i = 0; i < sampleCount; ++i) {
        trace.push_back(paired(rand).getRef());
    }
    return trace;
}

vector<pair<int, int>> collectChainTrace(Seed seed, size_t sampleCount)
{
    auto chained = gen::chain(gen::interval<int>(0, 5), [](const int& value) {
        return gen::interval<int>(value, value + 2);
    });
    Random rand(seed);
    vector<pair<int, int>> trace;
    for (size_t i = 0; i < sampleCount; ++i) {
        const auto value = chained(rand).getRef();
        trace.push_back(util::make_pair(get<0>(value), get<1>(value)));
    }
    return trace;
}

vector<pair<int, int>> collectTupleWithTrace(Seed seed, size_t sampleCount)
{
    auto tupled = gen::interval<int>(0, 5).tupleWith<int>([](const int& value) {
        return gen::interval<int>(value, value + 2);
    });
    Random rand(seed);
    vector<pair<int, int>> trace;
    for (size_t i = 0; i < sampleCount; ++i) {
        const auto value = tupled(rand).getRef();
        trace.push_back(util::make_pair(get<0>(value), get<1>(value)));
    }
    return trace;
}

}  // namespace

TEST(TransformationCombinatorContracts, transform_and_map_roots_and_shrinks_stay_in_mapped_domain)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed);

        auto freeFunctionGenerator = gen::transform<int, int>(
            gen::interval<int>(-20, 20),
            [](const int& value) {
                return value * 2;
            });
        const auto freeFunctionRoot = freeFunctionGenerator(rand);
        PROP_ASSERT(isMappedEvenDomain(freeFunctionRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(freeFunctionRoot, isMappedEvenDomain).empty());

        auto methodGenerator = gen::interval<int>(-20, 20).map<int>([](const int& value) {
            return value * 2;
        });
        const auto methodRoot = methodGenerator(rand);
        PROP_ASSERT(isMappedEvenDomain(methodRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(methodRoot, isMappedEvenDomain).empty());
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kTransformationReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

TEST(TransformationCombinatorContracts, filter_and_suchthat_roots_and_shrinks_satisfy_predicate)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed);

        auto filtered = gen::filter<int>(gen::interval<int>(1, 30), isPositiveMultipleOfThree);
        const auto filteredRoot = filtered(rand);
        PROP_ASSERT(isPositiveMultipleOfThree(filteredRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(filteredRoot, isPositiveMultipleOfThree).empty());

        auto constrained = gen::suchThat<int>(gen::interval<int>(1, 30), isPositiveMultipleOfThree);
        const auto constrainedRoot = constrained(rand);
        PROP_ASSERT(isPositiveMultipleOfThree(constrainedRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(constrainedRoot, isPositiveMultipleOfThree).empty());
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kTransformationReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

TEST(TransformationCombinatorContracts, derive_and_pairwith_respect_inner_generator_dependency)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed);

        auto derived = gen::derive<int, int>(gen::just<int>(3), [](const int& value) {
            return gen::interval<int>(value, value + 2);
        });
        const auto derivedRoot = derived(rand);
        PROP_ASSERT(derivedRoot.getRef() >= 3 && derivedRoot.getRef() <= 5);
        PROP_ASSERT(collectShrinkViolations(derivedRoot, [](int value) {
            return value >= 3 && value <= 5;
        }).empty());

        auto paired = gen::interval<int>(0, 5).pairWith<int>([](const int& value) {
            return gen::interval<int>(value, value + 2);
        });
        const auto pairedRoot = paired(rand);
        PROP_ASSERT(isValidDependentPair(pairedRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(pairedRoot, isValidDependentPair).empty());
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kDependencyReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

TEST(TransformationCombinatorContracts, chain_and_tuplewith_respect_prior_generated_values)
{
    EXPECT_TRUE(property([](Seed seed) {
        Random rand(seed);

        auto chained = gen::chain(gen::interval<int>(0, 5), [](const int& value) {
            return gen::interval<int>(value, value + 2);
        });
        const auto chainedRoot = chained(rand);
        PROP_ASSERT(isValidDependentChain(chainedRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(chainedRoot, isValidDependentChain).empty());

        auto tupled = gen::interval<int>(0, 5).tupleWith<int>([](const int& value) {
            return gen::interval<int>(value, value + 2);
        });
        const auto tupledRoot = tupled(rand);
        PROP_ASSERT(isValidDependentChain(tupledRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(tupledRoot, isValidDependentChain).empty());

        auto triple = gen::chain(chained, [](const Chain<int, int>& values) {
            return gen::interval<int>(get<1>(values), get<1>(values) + 2);
        });
        const auto tripleRoot = triple(rand);
        PROP_ASSERT(isValidDependentTriple(tripleRoot.getRef()));
        PROP_ASSERT(collectShrinkViolations(tripleRoot, isValidDependentTriple).empty());
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kDependencyReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

TEST(TransformationCombinatorContracts, dependent_pair_and_chain_combinators_replay_same_trace_for_same_seed)
{
    EXPECT_TRUE(property([](Seed seed) {
        const auto firstPairTrace = collectPairWithTrace(seed, kShortSamples);
        const auto secondPairTrace = collectPairWithTrace(seed, kShortSamples);
        PROP_ASSERT(firstPairTrace == secondPairTrace);
        for (const auto& value : firstPairTrace) {
            PROP_ASSERT(isValidDependentPair(value));
        }

        const auto firstChainTrace = collectChainTrace(seed, kShortSamples);
        const auto secondChainTrace = collectChainTrace(seed, kShortSamples);
        PROP_ASSERT(firstChainTrace == secondChainTrace);
        for (const auto& value : firstChainTrace) {
            PROP_ASSERT(isValidDependentPair(value));
        }

        const auto firstTupleTrace = collectTupleWithTrace(seed, kShortSamples);
        const auto secondTupleTrace = collectTupleWithTrace(seed, kShortSamples);
        PROP_ASSERT(firstTupleTrace == secondTupleTrace);
        for (const auto& value : firstTupleTrace) {
            PROP_ASSERT(isValidDependentPair(value));
        }
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kDependencyReplaySeed)})
                   .setNumRuns(kContractRuns)
                   .forAll());
}

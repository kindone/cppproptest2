/*
 * Contract:
 * - reference() must defer generator lookup until generation time, so recursive
 *   generators can mention a generator object before that object is assigned.
 * - A recursive generator built from construct(), vector(), oneOf(), and
 *   reference() must keep generated roots and bounded shrink candidates inside
 *   the declared recursive structure.
 * - lazy() must evaluate its callable when the generator runs, not when the
 *   generator object is constructed.
 *
 * Scope:
 * This file covers lazy/reference behavior and a bounded view of recursive
 * generation and shrinking. It does not assert exact generated tree shape,
 * recursive distribution, or serialized shrink-tree text.
 *
 * Helpers:
 * - RecursiveBox has only a vector of children, generated with size 0..2.
 * - recursiveBoxGen() uses gen::reference(boxGen) before boxGen is assigned;
 *   successful generation proves the reference is resolved lazily.
 * - collectShrinkViolations() walks a bounded breadth-first view of a shrink
 *   tree and returns every visited value that fails the supplied invariant.
 */

#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"

using namespace proptest;
using namespace proptest::testprofile;

namespace {

constexpr int64_t kRecursiveReplaySeed = 20260813;
constexpr size_t kMaxRecursiveChildren = 2;

struct RecursiveBox
{
    RecursiveBox() {}
    RecursiveBox(vector<RecursiveBox>& generatedChildren) : children(generatedChildren) {}

    vector<RecursiveBox> children;
};

bool hasDeclaredBranchShape(const RecursiveBox& box)
{
    if (box.children.size() > kMaxRecursiveChildren) {
        return false;
    }

    for (const auto& child : box.children) {
        if (!hasDeclaredBranchShape(child)) {
            return false;
        }
    }
    return true;
}

Generator<RecursiveBox> recursiveBoxGen()
{
    GenFunction<RecursiveBox> boxGen;
    auto leafGen = gen::construct<RecursiveBox>();
    auto branchGen = gen::construct<RecursiveBox, vector<RecursiveBox>&>(
        gen::vector<RecursiveBox>(gen::reference(boxGen)).setSize(0, kMaxRecursiveChildren));

    boxGen = gen::oneOf<RecursiveBox>(
        gen::weightedGen(leafGen, 0.85),
        gen::weightedGen(branchGen, 0.15));
    return generator(boxGen);
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

TEST(RecursiveCombinatorContracts, reference_delegates_to_generator_target_at_generation_time)
{
    EXPECT_TRUE(property([](Seed seed) {
        GenFunction<int> target = gen::just<int>(1);
        auto referenced = gen::reference(target);
        target = gen::just<int>(2);

        Random rand(seed);
        PROP_ASSERT_EQ(referenced(rand).getRef(), 2);
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kRecursiveReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

TEST(RecursiveCombinatorContracts, recursive_reference_roots_and_shrinks_keep_declared_branch_shape)
{
    EXPECT_TRUE(property([](Seed seed) {
        auto generator = recursiveBoxGen();
        Random rand(seed);
        const auto root = generator(rand);

        PROP_ASSERT(hasDeclaredBranchShape(root.getRef()));
        PROP_ASSERT(collectShrinkViolations(root, hasDeclaredBranchShape).empty());
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kRecursiveReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

TEST(RecursiveCombinatorContracts, lazy_invokes_callable_when_generator_runs)
{
    EXPECT_TRUE(property([](Seed seed) {
        int calls = 0;
        auto generator = gen::lazy<int>([&calls]() {
            return ++calls;
        });

        Random rand(seed);
        const auto first = generator(rand);
        const auto second = generator(rand);

        PROP_ASSERT_EQ(first.getRef(), 1);
        PROP_ASSERT_EQ(second.getRef(), 2);
        PROP_ASSERT(first.getShrinks().isEmpty());
        PROP_ASSERT(second.getShrinks().isEmpty());
        return true;
    }, gen::seed()).matrix({Seed(kCompatibilitySeed), Seed(kRecursiveReplaySeed)})
                   .setNumRuns(kShrinkContractRuns)
                   .forAll());
}

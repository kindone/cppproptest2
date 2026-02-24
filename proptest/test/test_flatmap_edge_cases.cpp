#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/string.hpp"

using namespace proptest;

/**
 * Test cases to validate/invalidate potential issues with flatMap implementations.
 *
 * These tests check if the theoretical issues identified actually manifest in practice.
 */

TEST(FlatMapEdgeCases, nested_flatmap_determinism)
{
    /**
     * Test: Does nested flatMap maintain determinism?
     *
     * Issue: Inner and outer flatMap might share RNG state incorrectly.
     * Expected: Should be deterministic with same seed.
     */
    Random rand1(42);
    Random rand2(42);

    // Nested flatMap: int -> list -> list of strings
    auto gen = gen::interval(1, 3).flatMap<vector<int>>([](const int& n) {
        return gen::vector<int>(gen::interval(1, 5)).setSize(n, n);
    }).flatMap<vector<string>>([](const vector<int>& lst) {
        size_t size = lst.size();
        return gen::vector<string>(gen::string()).setSize(size, size);
    });

    auto shrinkable1 = gen(rand1);
    auto shrinkable2 = gen(rand2);

    // Root values should match
    EXPECT_EQ(shrinkable1.getRef(), shrinkable2.getRef())
        << "Nested flatMap should be deterministic";
}

TEST(FlatMapEdgeCases, nested_flatmap_independence)
{
    /**
     * Test: Are nested flatMap generations independent?
     *
     * Issue: Inner flatMap might use RNG state that depends on outer flatMap.
     * Expected: Each level should generate independently.
     */
    Random rand(42);

    vector<int> outer_values;
    vector<int> inner_values;

    auto track_outer = [&outer_values](const int& n) {
        outer_values.push_back(n);
        return gen::vector<int>(gen::interval(1, 10)).setSize(n, n);
    };

    auto track_inner = [&inner_values](const vector<int>& lst) {
        inner_values.push_back(lst.size());
        int sum = 0;
        for (auto val : lst) sum += val;
        return gen::just(sum);
    };

    auto gen = gen::interval(1, 5).flatMap<vector<int>>(track_outer).flatMap<int>(track_inner);

    // Generate multiple times
    for (int i = 0; i < 10; i++) {
        gen(rand);
    }

    // Check: outer_values should be independent of inner_values
    EXPECT_GT(outer_values.size(), 0U) << "Should generate outer values";
    EXPECT_GT(inner_values.size(), 0U) << "Should generate inner values";

    // The key test: if we generate with same seed, should get same sequences
    Random rand2(42);
    vector<int> outer_values2;
    vector<int> inner_values2;

    auto track_outer2 = [&outer_values2](const int& n) {
        outer_values2.push_back(n);
        return gen::vector<int>(gen::interval(1, 10)).setSize(n, n);
    };

    auto track_inner2 = [&inner_values2](const vector<int>& lst) {
        inner_values2.push_back(lst.size());
        int sum = 0;
        for (auto val : lst) sum += val;
        return gen::just(sum);
    };

    auto gen2 = gen::interval(1, 5).flatMap<vector<int>>(track_outer2).flatMap<int>(track_inner2);

    for (int i = 0; i < 10; i++) {
        gen2(rand2);
    }

    EXPECT_EQ(outer_values, outer_values2)
        << "Outer values should be deterministic";
    EXPECT_EQ(inner_values, inner_values2)
        << "Inner values should be deterministic";
}

TEST(FlatMapEdgeCases, multiple_regenerations)
{
    /**
     * Test: Does multiple tree regenerations break determinism?
     *
     * Issue: If tree is regenerated multiple times, savedRand advances.
     * Expected: Should still be deterministic (if regenerated correctly).
     */
    Random rand(42);
    Random savedRand(42);

    auto gen = gen::interval(1, 5).flatMap<vector<int>>([](const int& n) {
        return gen::vector<int>(gen::interval(1, 10)).setSize(n, n);
    });

    // Regenerate tree multiple times
    vector<Shrinkable<vector<int>>> trees;
    for (int i = 0; i < 5; i++) {
        savedRand = rand;  // Reset to same state
        auto tree = gen(savedRand);
        trees.push_back(tree);
    }

    // All trees should have same root value (deterministic)
    for (size_t i = 1; i < trees.size(); i++) {
        EXPECT_EQ(trees[0].getRef(), trees[i].getRef())
            << "Multiple regenerations should be deterministic";
    }
}

TEST(FlatMapEdgeCases, deeply_nested_flatmap)
{
    /**
     * Test: Does deeply nested flatMap (3+ levels) work correctly?
     *
     * Issue: State management might break with deep nesting.
     * Expected: Should work correctly at any depth.
     */
    Random rand1(42);
    Random rand2(42);

    // 3 levels of nesting
    auto gen = gen::interval(1, 3).flatMap<vector<int>>([](const int& n) {
        return gen::vector<int>(gen::interval(1, 5)).setSize(n, n);
    }).flatMap<vector<string>>([](const vector<int>& lst) {
        size_t size = lst.size();
        return gen::vector<string>(gen::string()).setSize(size, size);
    }).flatMap<int>([](const vector<string>& str_lst) {
        return gen::just(static_cast<int>(str_lst.size()));
    });

    auto shrinkable1 = gen(rand1);
    auto shrinkable2 = gen(rand2);

    // Should be deterministic
    EXPECT_EQ(shrinkable1.getRef(), shrinkable2.getRef())
        << "Deeply nested flatMap should be deterministic";

    // Should produce valid values
    EXPECT_GE(shrinkable1.getRef(), 1);
}

TEST(FlatMapEdgeCases, flatmap_with_filter_chain)
{
    /**
     * Test: Does flatMap work correctly when chained with filter?
     *
     * Issue: Filter might interfere with flatMap's RNG state management.
     * Expected: Should work correctly.
     */
    Random rand(42);

    auto gen = gen::interval(1, 10).flatMap<vector<int>>([](const int& n) {
        return gen::vector<int>(gen::interval(1, 10)).setSize(n, n);
    }).filter([](const vector<int>& lst) {
        int sum = 0;
        for (auto val : lst) sum += val;
        return sum > 10;
    });

    auto shrinkable = gen(rand);

    // Root should satisfy filter
    int root_sum = 0;
    for (auto val : shrinkable.getRef()) {
        root_sum += val;
    }
    EXPECT_GT(root_sum, 10) << "Root should satisfy filter condition";
}


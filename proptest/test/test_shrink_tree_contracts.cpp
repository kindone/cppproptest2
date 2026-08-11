/*
 * Contract:
 * - Integral shrink trees preserve the root value and only move candidates
 *   toward zero without crossing to the opposite sign.
 * - Shrinkable::filter() removes rejected values at every traversed depth.
 * - Shrinkable::map() transforms every shrink-tree value while preserving the
 *   traversal shape of the original tree.
 * - List-like membership shrinking respects minimum size and does not expose
 *   duplicate vectors in the traversed tree.
 * - concatStatic() adds a fallback candidate at the current shrink level, while
 *   andThenStatic() appends fallback candidates below existing leaves.
 *
 * Scope:
 * This file covers structural shrink-tree invariants that are stable across
 * formatting changes. It does not assert exact serialized tree text, floating
 * point shrink strategy, tuple/pair formatting, or shrink retry behavior.
 *
 * Helpers:
 * - collectPreorderValues() walks a bounded preorder traversal and records the
 *   values exposed by a shrink tree.
 * - collectDirectChildren() records only the first shrink level under a root.
 * - makeIntegerVectorShrinkable() builds a vector shrinkable from integral
 *   element shrinkables so list-like shrink contracts can be checked directly.
 */

#include "proptest/proptest.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/shrinkers.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/test_profiles.hpp"
#include "proptest/test/testutil.hpp"

#include <algorithm>
#include <cstdlib>
#include <set>

using namespace proptest;
using namespace proptest::testprofile;

namespace {

template <typename T>
vector<T> collectPreorderValues(const Shrinkable<T>& root)
{
    vector<T> values;
    vector<Shrinkable<T>> pending{root};

    for (size_t index = 0; index < pending.size() && values.size() < kShrinkTraversalLimit; ++index) {
        const Shrinkable<T> current = pending[index];
        values.push_back(current.getRef());

        auto shrinks = current.getShrinks();
        for (auto itr = shrinks.template iterator<typename Shrinkable<T>::StreamElementType>();
             itr.hasNext() && pending.size() < kShrinkTraversalLimit;) {
            pending.push_back(itr.next());
        }
    }

    return values;
}

template <typename T>
vector<T> collectDirectChildren(const Shrinkable<T>& root)
{
    vector<T> values;
    auto shrinks = root.getShrinks();
    for (auto itr = shrinks.template iterator<typename Shrinkable<T>::StreamElementType>(); itr.hasNext();) {
        Shrinkable<T> child = itr.next();
        values.push_back(child.getRef());
    }
    return values;
}

Shrinkable<vector<int>> makeIntegerVectorShrinkable(const vector<int>& values, size_t minSize)
{
    Shrinkable<vector<ShrinkableBase>> base = make_shrinkable<vector<ShrinkableBase>>();
    vector<ShrinkableBase>& elements = base.getMutableRef();
    for (int value : values) {
        elements.push_back(ShrinkableBase(shrinkIntegral<int>(value)));
    }
    return shrinkListLike<vector, int>(base, minSize, false, true);
}

}  // namespace

TEST(ShrinkTreeContracts, integral_shrinks_preserve_root_and_move_toward_zero)
{
    matrix([](int root) {
        const auto values = collectPreorderValues(shrinkIntegral<int>(root));

        ASSERT_FALSE(values.empty());
        EXPECT_EQ(values.front(), root);

        for (int value : values) {
            EXPECT_LE(std::abs(value), std::abs(root));
            if (root > 0) {
                EXPECT_GE(value, 0);
            } else if (root < 0) {
                EXPECT_LE(value, 0);
            } else {
                EXPECT_EQ(value, 0);
            }
        }
    }, {-20, -8, -1, 0, 1, 8, 20});
}

TEST(ShrinkTreeContracts, filter_removes_rejected_nodes_at_every_traversed_depth)
{
    const auto shrinkable = util::binarySearchShrinkable(20).filter([](const int64_t& value) {
        return value > 10;
    });

    const auto values = collectPreorderValues(shrinkable);

    ASSERT_FALSE(values.empty());
    EXPECT_EQ(values.front(), 20);
    for (int64_t value : values) {
        EXPECT_GT(value, 10);
    }
}

TEST(ShrinkTreeContracts, map_transforms_values_without_changing_traversal_shape)
{
    const auto originalValues = collectPreorderValues(shrinkIntegral<int>(8));
    const auto mappedValues = collectPreorderValues(shrinkIntegral<int>(8).map<int>([](const int& value) {
        return value + 10;
    }));

    ASSERT_EQ(originalValues.size(), mappedValues.size());
    for (size_t index = 0; index < originalValues.size(); ++index) {
        EXPECT_EQ(mappedValues[index], originalValues[index] + 10);
    }
}

TEST(ShrinkTreeContracts, list_membership_shrinks_respect_minimum_size_and_remain_unique)
{
    const vector<int> root{1, 2, 3, 4, 5};
    const size_t minSize = 2;
    const auto values = collectPreorderValues(makeIntegerVectorShrinkable(root, minSize));
    set<vector<int>> uniqueValues(values.begin(), values.end());

    ASSERT_FALSE(values.empty());
    EXPECT_EQ(values.front(), root);
    EXPECT_EQ(uniqueValues.size(), values.size());
    for (const auto& value : values) {
        EXPECT_GE(value.size(), minSize);
        EXPECT_LE(value.size(), root.size());
    }
}

TEST(ShrinkTreeContracts, concat_and_andthen_place_fallback_candidates_at_distinct_depths)
{
    using Shr = Shrinkable<int>;
    using Elem = Shr::StreamElementType;
    using Stream = Shr::StreamType;

    const auto fallback = Stream::one<Elem>(Shrinkable<int>(3));
    const auto concatValues = collectDirectChildren(shrinkIntegral<int>(2).concatStatic(fallback));
    const auto andThenValues = collectDirectChildren(shrinkIntegral<int>(2).andThenStatic(fallback));
    const auto andThenFullValues = collectPreorderValues(shrinkIntegral<int>(2).andThenStatic(fallback));

    EXPECT_EQ(concatValues, vector<int>({0, 1, 3}));
    EXPECT_EQ(andThenValues, vector<int>({0, 1}));
    EXPECT_NE(std::find(andThenFullValues.begin(), andThenFullValues.end(), 3), andThenFullValues.end());
}

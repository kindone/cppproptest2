#include "proptest/util/tupleorvector.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/exception.hpp"
#include "gtest/gtest.h"

using namespace proptest;

TEST(TupleOrVector, tuple_homogeneous)
{
    tuple<int, int, int> t(1, 2, 3);
    TupleHolder<int, int, int> holder(t);
    EXPECT_EQ(holder.get<0>(), 1);

    unique_ptr<TupleOrVectorHolder> anyHolder = util::make_unique<TupleHolder<int, int, int>>(t);

    EXPECT_EQ(anyHolder->toAnyVector()[0].getRef<int>(), 1);
    EXPECT_EQ(anyHolder->toAnyVector()[1].getRef<int>(), 2);
    EXPECT_EQ(anyHolder->toAnyVector()[2].getRef<int>(), 3);

    auto t2 = anyHolder->toTuple<int, int, int>();
    EXPECT_EQ(t2, t);

    EXPECT_THROW(anyHolder->toAnyVector()[3].getRef<double>(), invalid_cast_error);
}

TEST(TupleOrVector, tuple_heterogeneous)
{
    tuple<int, double, string> t(1, 2.0, "3");
    TupleHolder<int, double, string> holder(t);
    EXPECT_EQ(holder.get<0>(), 1);

    unique_ptr<TupleOrVectorHolder> anyHolder = util::make_unique<TupleHolder<int, double, string>>(t);

    EXPECT_EQ(anyHolder->toAnyVector()[0].getRef<int>(), 1);
    EXPECT_EQ(anyHolder->toAnyVector()[1].getRef<double>(), 2.0);
    EXPECT_EQ(anyHolder->toAnyVector()[2].getRef<string>(), "3");

    auto t2 = anyHolder->toTuple<int, double, string>();
    EXPECT_EQ(t2, t);

    EXPECT_THROW(anyHolder->toAnyVector()[3].getRef<int>(), invalid_cast_error);
}

TEST(TupleOrVector, vector)
{
    tuple<int, int, int> t(1, 2, 3);
    vector<int> vec{1, 2, 3};
    VectorHolder<int> holder(vec);
    EXPECT_EQ(holder.value[0], 1);

    unique_ptr<TupleOrVectorHolder> anyHolder = util::make_unique<VectorHolder<int>>(vec);

    EXPECT_EQ(anyHolder->toAnyVector()[0].getRef<int>(), 1);
    EXPECT_EQ(anyHolder->toAnyVector()[1].getRef<int>(), 2);
    EXPECT_EQ(anyHolder->toAnyVector()[2].getRef<int>(), 3);
    auto t2 = anyHolder->toTuple<int, int, int>();
    EXPECT_EQ(t2, t);

    EXPECT_THROW(anyHolder->toAnyVector()[3].getRef<double>(), invalid_cast_error);
}
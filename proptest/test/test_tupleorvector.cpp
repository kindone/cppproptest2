#include "proptest/util/tupleorvector.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/gtest.hpp"

using namespace proptest;

TEST(vectorToTuple, basic)
{
    vector<Any> vec{1, 2, 3};
    tuple<int, int, int> t(1, 2, 3);
    EXPECT_EQ((util::vectorToTuple<int, int, int>(vec)), t);
}

TEST(vectorToTuple, invalid)
{
    vector<Any> vec{1, 2, 3};
    EXPECT_THROW((util::vectorToTuple<int, int, int, int>(vec)), invalid_argument);
}

TEST(tupleToVector, basic)
{
    tuple<int, int, int> t(1, 2, 3);
    vector<Any> vec{1, 2, 3};
    EXPECT_EQ((util::vectorToTuple<int, int, int>(util::tupleToAnyVector<int, int, int>(t))), t);
}

TEST(Map, basic)
{
    auto t = util::Map([](auto index) { return index.value; }, make_index_sequence<3>{});
    EXPECT_EQ(t, util::make_tuple(0, 1, 2));
}

TEST(Map, hetero)
{
    auto t = util::Map([](auto index) {
        if constexpr (index.value == 0)
            return 0;
        else if constexpr (index.value == 1)
            return 1.0;
        else
            return string("2");
    }, make_index_sequence<3>{});
    EXPECT_EQ(t, util::make_tuple(0, 1.0, "2"));
}

TEST(TupleOrVectorHolder, tuple_homogeneous)
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

    EXPECT_THROW(anyHolder->toAnyVector()[2].getRef<double>(), invalid_cast_error);
}

TEST(TupleOrVectorHolder, tuple_heterogeneous)
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

    EXPECT_THROW(anyHolder->toAnyVector()[2].getRef<int>(), invalid_cast_error);
}

TEST(TupleOrVectorHolder, vector)
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

    EXPECT_THROW(anyHolder->toAnyVector()[2].getRef<double>(), invalid_cast_error);
}

TEST(TupleOrVector, tuple_homogeneous)
{
    tuple<int, int, int> t(1, 2, 3);
    TupleOrVector any(t);
    EXPECT_EQ((any.toTuple<int, int, int>()), t);
    EXPECT_EQ(any.toAnyVector()[0].getRef<int>(), 1);
    EXPECT_EQ(any.toAnyVector()[1].getRef<int>(), 2);
    EXPECT_EQ(any.toAnyVector()[2].getRef<int>(), 3);
    EXPECT_THROW(any.toAnyVector()[2].getRef<double>(), invalid_cast_error);
}

TEST(TupleOrVector, tuple_heterogeneous)
{
    tuple<int, double, string> t(1, 2.0, "3");
    TupleOrVector any(t);
    EXPECT_EQ((any.toTuple<int, double, string>()), t);
    EXPECT_EQ(any.toAnyVector()[0].getRef<int>(), 1);
    EXPECT_EQ(any.toAnyVector()[1].getRef<double>(), 2.0);
    EXPECT_EQ(any.toAnyVector()[2].getRef<string>(), "3");
    EXPECT_THROW(any.toAnyVector()[2].getRef<int>(), invalid_cast_error);
}

TEST(TupleOrVector, vector)
{
    tuple<int, int, int> t(1, 2, 3);
    vector<int> vec{1, 2, 3};
    TupleOrVector any(vec);
    EXPECT_EQ((any.toTuple<int, int, int>()), t);
    EXPECT_EQ(any.toAnyVector()[0].getRef<int>(), 1);
    EXPECT_EQ(any.toAnyVector()[1].getRef<int>(), 2);
    EXPECT_EQ(any.toAnyVector()[2].getRef<int>(), 3);
    EXPECT_THROW(any.toAnyVector()[2].getRef<double>(), invalid_cast_error);
}
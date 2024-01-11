#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/io.hpp"
#include "gtest/gtest.h"

using namespace proptest;

TEST(Shrinkable, basic)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    EXPECT_EQ(shr.getAny().getRef<int>(), 100);
}

TEST(Shrinkable, with)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.with(TypedStream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    EXPECT_EQ(shr2.get(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), false);
    EXPECT_EQ(shr2.getShrinks().getHeadRef().get(), 200);
}

TEST(Shrinkable, filter)
{
    Shrinkable<int> shr(100);
    shr = shr.with(TypedStream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.filter([](const int& val) { return val == 100; });
    EXPECT_EQ(shr2.get(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), true);

    auto shr3 = shr.filter([](const int& val) { return val <= 200; });
    EXPECT_EQ(shr3.get(), 100);
    EXPECT_EQ(shr3.getShrinks().isEmpty(), false);
    EXPECT_EQ(shr3.getShrinks().getHeadRef().get(), 200);
}

TEST(Shrinkable, map)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.map<int>([](const int& val) { return val + 1; });
    EXPECT_EQ(101, shr2.get());
}

TEST(Shrinkable, flatMap)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.flatMap<int>([](const int& val) { return make_shrinkable<int>(val + 1); });
    EXPECT_EQ(101, shr2.get());
}

TEST(Shrinker, integral)
{
    auto shr = util::binarySearchShrinkable(8);
    auto shrinks = shr.getShrinks();
    for(auto itr = shrinks.iterator(); itr.hasNext();)
    {
        cout << itr.next().get() << endl;
    }
}
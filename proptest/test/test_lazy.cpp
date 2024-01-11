#include "proptest/util/lazy.hpp"
#include "gtest/gtest.h"

using namespace proptest;

TEST(Lazy, value)
{
    Lazy<int> x(5);
    EXPECT_EQ(*x, 5);
}

TEST(Lazy, function)
{
    Lazy<int> x([]() { return 5; });
    EXPECT_EQ(*x, 5);
}

TEST(Lazy, copy)
{
    Lazy<int> x([]() { return 5; });
    auto y = x;
    EXPECT_EQ(*y, 5);
    auto z = y;
    EXPECT_EQ(*z, 5);
}

TEST(Lazy, reassign)
{
    Lazy<int> x([]() { return 5; });
    Lazy<int> y([]() { return 7; });
    auto z = x;
    EXPECT_EQ(*z, 5);
    z = y;
    EXPECT_EQ(*z, 7);
}
#include "proptest/util/anyfunction.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(AnyFunction, lambda)
{
    Function1 func1 = [](int x) { return x + 1; };
    EXPECT_EQ(func1(1).getRef<int>(), 2);
}

TEST(AnyFunction, Function)
{
    Function<int(int)> func0 = [](int x) { return x + 1; };
    Function1 func1 = func0;
    EXPECT_EQ(func1(1).getRef<int>(), 2);
}

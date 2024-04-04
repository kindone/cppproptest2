#include "proptest/util/anyfunction.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Function1, lambda)
{
    Function1 func1 = [](int x) { return x + 1; };
    EXPECT_EQ(func1(1).getRef<int>(), 2);

    func1 = [](int& x) { return x + 1; };
    int a = 1;
    EXPECT_EQ(func1(a).getRef<int>(), 2);

    func1 = [](const int& x) { return x + 1; };
    EXPECT_EQ(func1(1).getRef<int>(), 2);

    func1 = [](int* x) { return *x + 1; };
    EXPECT_EQ(func1(&a).getRef<int>(), 2);
}

TEST(Function1, lambda2)
{
    Function1 func1 = [](int x) {
        Function1 func0 = [x](int y) { return x + y; };
        return func0;
    };
    EXPECT_EQ(func1(1).getRef<Function1>()(1).getRef<int>(), 2);
}

TEST(Function1, Function)
{
    Function<int(int)> func0 = [](int x) { return x + 1; };
    Function1 func1 = func0;
    EXPECT_EQ(func1(1).getRef<int>(), 2);
}

TEST(Function1Impl, lambda)
{
    Function1Impl<int, int> func1 = Function1([](int x) { return x + 1; });
    EXPECT_EQ(func1(1), 2);
}

#include "proptest/proptest.hpp"

#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, Shrinkable)
{
    auto shr1 = make_shrinkable<int>(1);
    auto shr2 = make_shrinkable<string>("hello");
    auto shr3 = make_shrinkable<vector<int>>(0);
    auto shr4 = make_shrinkable<Function<int(int)>>([](int a) { return a + 5; });

    auto shr5 = make_shrinkable<map<string, string>>();
}

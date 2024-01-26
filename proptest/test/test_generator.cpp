#include "proptest/generator/integral.hpp"
#include "proptest/Random.hpp"
#include "proptest/gtest.hpp"

using namespace proptest;

TEST(GenIntegral, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 10);
    auto val = gen(rand).get();
    EXPECT_TRUE(val >= std::numeric_limits<int>::min());
    EXPECT_TRUE(val <= std::numeric_limits<int>::max());
}
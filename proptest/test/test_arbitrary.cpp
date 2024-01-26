#include "proptest/Arbitrary.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/Random.hpp"
#include "proptest/gtest.hpp"

using namespace proptest;

TEST(Arbitrary, basic)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto val = arbi(rand).get();
    for(int i = 0; i < 10; i++)
        cout << arbi(rand).get() << endl;
    EXPECT_TRUE(val >= std::numeric_limits<int>::min());
    EXPECT_TRUE(val <= std::numeric_limits<int>::max());
}
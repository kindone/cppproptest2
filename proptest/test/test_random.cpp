#include "proptest/Random.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Random, basic)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Random rand2(seed);
    // EXPECT_EQ(rand, rand2);
    EXPECT_EQ(rand.getRandomUInt64(), rand2.getRandomUInt64());
    EXPECT_EQ(rand.getRandomUInt64(), rand2.getRandomUInt64());
    EXPECT_EQ(rand.getRandomUInt64(), rand2.getRandomUInt64());
}

TEST(Random, bool)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    bool trueFound = false, falseFound = false;
    for(size_t i = 0; i < 100; i++)
    {
        bool val = rand.getRandomBool();
        if(val)
            trueFound = true;
        else
            falseFound = true;

        if(trueFound && falseFound)
            break;
    }

    EXPECT_EQ(trueFound, true);
    EXPECT_EQ(falseFound, true);
}

// TODO: how many tries need to be done to get all values in a range?
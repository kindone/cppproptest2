#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, vector)
{
    Random rand(1);
    auto numGen = Arbi<int64_t>();
    auto gen1 = Arbi<vector<int64_t>>();
    auto gen2 = Arbi<vector<int64_t>>(numGen);
    auto gen3 = Arbi<vector<int64_t>>(generator(numGen));
    gen1(rand);
    gen2(rand);
    gen3(rand);
}

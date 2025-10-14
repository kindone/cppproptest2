#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, integral)
{
    Random rand(1);
    auto gen = gen::int32();
    gen(rand);

    gen::inRange<int>(0, 10);
    gen::interval<int>(0, 10);
    gen::integers<int>(0, 10);
}

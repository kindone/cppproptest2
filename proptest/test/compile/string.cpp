#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, string)
{
    Random rand(1);
    auto gen = Arbi<string>();
    gen(rand);
}

TEST(Compile, stringGen)
{
    Arbi<string>(interval('A', 'Z'));
}

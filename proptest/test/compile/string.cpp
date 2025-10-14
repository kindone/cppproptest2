#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, string)
{
    Random rand(1);
    auto gen = gen::string();
    gen(rand);
}

TEST(Compile, stringGen)
{
    gen::string(gen::interval('A', 'Z'));
}

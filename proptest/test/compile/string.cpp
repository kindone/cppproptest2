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

TEST(Compile, string_config)
{
    Random rand(1);
    auto gen1 = gen::string({.minSize = 5, .maxSize = 20});
    auto gen2 = gen::string({.elemGen = gen::interval<char>('0', '9'), .minSize = 1, .maxSize = 10});
    gen1(rand);
    gen2(rand);
}
